/* mkfscript -- front end to mkfilter/gencode/genplot
   for WWW interactive filter design
   AJF	 January 1996
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <math.h>
#include <stdarg.h>

#define unless(x)    if(!(x))
#define until(x)     while(!(x))

#define MAXSTR	    256

#define TEMP_DIR    "./output"
#define TOOLS_DIR   "./helpers"
#define TEMP_URL    "/output"
#define MY_URL	    "http://www-users.cs.york.ac.uk/~fisher/mkfilter"

#define TWOPI	    (2.0 * M_PI)

/* 2nd arg. to getfval */
#define UNCHECKED   0x00
#define MB_PRES	    0x01    /* must be present (non-empty) */
#define MB_GT0	    0x02    /* must be .gt. 0 */
#define MB_GE0	    0x04    /* must be .ge. 0 */
#define MB_LT0	    0x08    /* must be .lt. 0 */
#define MB_LE1	    0x10    /* must be .le. 1 */

typedef unsigned int uint;

union word
  { word(int nx)   { n = nx; }
    word(char *sx) { s = sx; }
    int n; char *s;
  };


static double samplerate, graph_a1, graph_a2;
static int nirsteps;
static char expid[16], mypid[16];

static void newhandler(), logaccess(), checkreferrer(), printheader(), summarize();
static void mkfilter(), mkmagphasegraph(), mktimegraphs(), dlcoeffs()/*, redirect(char*) */;
static void obeycmd(char*, bool);
static FILE *do_popen(char*);
static void do_pclose(FILE*);
static void makefiltercmd(char*), makeres(char*, int&), makepif(char*, int&), maketrad(char*, int&);
static double getalpha(const char*, uint);
static double getfval(const char*, uint);
static int getival(const char*, uint);
static void appendptype(char*, int&, const char*);
static void appends(char*, int&, const char*, const char* = NULL, const char* = NULL, const char* = NULL);
static void appendi(char*, int&, const char*, int = 0, int = 0);
static void appendf(char*, int&, const char*, double = 0.0, double = 0.0);
static void printtrailer();
static void hfatal(const char*, ...);

inline bool seq(const char *s1, const char *s2)    { return strcmp(s1,s2) == 0;		  }
inline bool starts(const char *s1, const char *s2) { return strncmp(s1, s2, strlen(s2)) == 0; }

// libcgi quick replacement:

#include <vector>
#include <cgicc/Cgicc.h>

struct entry
{
private:
	std::string name;
	std::string value;

public:
	const char* nam;
	const char* val;

	entry(const std::string& n, const std::string& v) : name(n), value(v), nam(name.c_str()), val(value.c_str()) {}
	entry(entry const& other) : name(other.nam), value(other.val), nam(name.c_str()), val(value.c_str()) {}
};

std::vector<entry> entries;
static size_t numentries;

static void getentries()
{
	try {
		cgicc::Cgicc cgi;
		const std::vector<cgicc::FormEntry>& fes = cgi.getElements();

		for (std::vector<cgicc::FormEntry>::const_iterator fe = fes.begin(); fe != fes.end(); fe++)
		{
			entry e(fe->getName(), fe->getValue());
			entries.push_back(e);
		}

		numentries = entries.size();

	} catch (std::exception const& e) {
		printheader();
		hfatal("Exception: %s!", e.what());
	} catch (...) {
		printheader();
		hfatal("Exception!");
	}
}

static bool isset(const char* key)
{
	for (std::vector<entry>::const_iterator e = entries.begin(); e != entries.end(); e++)
	{
		if (!strcmp(key, e->nam))
			return true;
	}

	return false;
}

static const char* getval(const char* key)
{
	for (std::vector<entry>::const_iterator e = entries.begin(); e != entries.end(); e++)
	{
		if (!strcmp(key, e->nam))
			return e->val;
	}

	return "";
}

static void discard_output()
{
	/* TODO */
}

// libcgi end.


int main(int, char **)
  { std::set_new_handler(newhandler);
    umask(022);				   /* make files written to tmpdir readable by server */
    bool pt = true; /* print trailer? */
    getentries();
	bool dl = isset("dlcoeffs");
	if (!dl)
	{
		printheader();
		summarize();
	}
    logaccess();
    checkreferrer();
    mkfilter();
    if (!isset("expid"))
      { /* new filter; make all graphs */
	mkmagphasegraph();
	mktimegraphs();
      }
    else
      { /* expand existing graph, or download code or coeffs */
	if (isset("flim1")) mkmagphasegraph();
	if (isset("nirsteps")) mktimegraphs();
	if (dl)
	  { dlcoeffs();
	    pt = false;
	  }
      }
    if (pt) printtrailer();
    exit(0);
  }

static void newhandler()
  { hfatal("No room!");
  }

static void cleanup();

static const char* uniqueid()
{
  static char* unique = NULL;
  static char  tmplate[] = TEMP_DIR "/tmpXXXXXX";

  if (!unique)
  {
    char* result = mkdtemp(tmplate);
    if (result == NULL)
    {
      hfatal("create temporary directory: %s", strerror(errno));
    }
    unique = strrchr(result, '/');
    unique++;
    
    atexit(cleanup); // remove empty directories
  }
  
  return unique;
}

static void cleanup()
{
  char dirname[] = TEMP_DIR "/tmpXXXXXX";
  const char *unique = uniqueid();
  
  snprintf(dirname + sizeof(dirname) - 10, 10, "%s", unique);
  (void)rmdir(dirname);
}

static void logweb(const char *prefix, const char* message)
{
  fprintf(stderr, "%s: %s\n", prefix, message);
}

static void logaccess()
  { const char* id = uniqueid();
    logweb("mkfilter", id);
  }

static void checkreferrer()
  { return;
	/*
	 * Yes, unfortunately this is a bootleg site...
	 *
	char *s = getenv("HTTP_REFERER");
    if (s == NULL || !starts(s, "http://www-users.cs.york.ac.uk/~fisher/"))
      { char url[MAXSTR+1];
	sprintf(url, "%s/bootleg.html", MY_URL);
	redirect(url);
	exit(0);
      }
	 */
  }

static void printheader()
  { printf("Content-type: text/html\n\n");
    printf("<HTML>\n\n");
    printf("<title> Filter Design Results </title>\n");
    printf("<h1> Filter Design Results </h1>\n");
    printf("Generated by: &nbsp; <a href=/>%s</a> <p>\n", MY_URL);
  }

static void summarize()
  { printf("<h2> Summary </h2>\n");
    printf("You specified the following parameters:\n");
    printf("<ul> <table>\n");
    for (size_t i = 0; i < numentries; i++)
      { entry *e = &entries[i];
	printf("   <tr> <td> %s <td> = <td> %s\n", e -> nam, e -> val);
      }
    printf("</table> </ul>\n");
  }

static void mkfilter()
  { samplerate = getfval("samplerate", MB_PRES | MB_GT0);
    sprintf(mypid, "%s", uniqueid());
    if (isset("expid"))
      { /* expand a previously-created graph */
	strcpy(expid, getval("expid")); /* identifies parent .mkf file */
	if (isset("flim1") && isset("flim2"))
	  { graph_a1 = getfval("flim1", UNCHECKED);     /* dflt is zero */
	    graph_a2 = getfval("flim2", UNCHECKED);     /* dflt is zero */
	    if (graph_a1 >= graph_a2) hfatal("Upper limit must be greater than lower limit.");
	  }
	if (isset("nirsteps")) nirsteps = getival("nirsteps", MB_GT0);
      }
    else
      { /* create a new filter */
	const char *ftype = getval("filtertype");
	strcpy(expid, mypid);
	if (seq(ftype, "Raised Cosine"))
	  { double alpha = getalpha("corner", MB_PRES);
	    int nir = getival("impulselen", MB_PRES | MB_GT0);
	    graph_a1 = -(3*alpha); graph_a2 = +(3*alpha); nirsteps = (int) (1.5 * nir);
	  }
	else if (seq(ftype, "Hilbert Transformer"))
	  { int nir = getival("impulselen", MB_PRES | MB_GT0);
	    graph_a1 = -0.5; graph_a2 = +0.5; nirsteps = (int) (1.5 * nir);
	  }
	else
	  { graph_a1 = 0.0; graph_a2 = 0.5; nirsteps = 100;
	  }
	printf("<h2> Results </h2>\n");
	char cmd[MAXSTR+1];
	makefiltercmd(cmd);
	obeycmd(cmd, true);
	printf("<h2> Ansi ``C'' Code </h2>\n");
	int len = strlen(cmd); sprintf(&cmd[len], "-l >%s/%s/params.mkf", TEMP_DIR, expid);
	obeycmd(cmd, true);
	sprintf(cmd, "cat %s/%s/params.mkf | %s/gencode", TEMP_DIR, expid, TOOLS_DIR);
	obeycmd(cmd, true);
	printf("<form method=POST>\n");
	printf("    Download code and/or coefficients:\n");
	printf("    <input type=hidden name=expid value=%s>\n", expid);
	printf("    <input type=hidden name=samplerate value=%g>\n", samplerate);
	printf("    <input type=submit name=dlcoeffs value=TERSE>\n");
	printf("    <input type=submit name=dlcoeffs value=VERBOSE>\n");
	printf("</form>\n");
      }
  }

static void mkmagphasegraph()
  { const char *ftype = getval("filtertype");
    char cmd[MAXSTR+1]; int p = 0;
    double logmin = getfval("logmin", MB_LT0);  /* 0.0 if not specified */
    printf("<h2> Magnitude (red) and phase (blue) vs. frequency </h2>\n");
    printf("<ul>\n");
    printf("<li> <i>x</i> axis: frequency, as a fraction of the sampling rate\n");
    printf("(i.e. 0.5 represents the Nyquist frequency, which is %g Hz)\n", 0.5*samplerate);
    printf("<li> <i>y</i> axis (red): magnitude (%s, normalized)\n", (logmin != 0.0) ? "logarithmic" : "linear");
    printf("<li> <i>y</i> axis (blue): phase\n");
    printf("</ul> <p>\n");
    appends(cmd, p, "cat %s/%s/params.mkf | %s/genplot ", TEMP_DIR, expid, TOOLS_DIR);
    appendf(cmd, p, "-a %17.10e %17.10e ", graph_a1, graph_a2);
    if (logmin != 0.0) appendf(cmd, p, "-log %17.10e ", logmin);
    if (seq(ftype, "Raised Cosine") || seq(ftype, "Hilbert Transformer")) appends(cmd, p, "-d ");
    appends(cmd, p, "%s/%s/F.gif", TEMP_DIR, mypid);
    obeycmd(cmd, true);
    printf("<img src=%s/%s/F.gif> <p>\n", TEMP_URL, mypid);
    printf("For an expanded view, enter frequency limits (as a fraction of the sampling rate) here: <br>\n");
    printf("<form method=POST>\n");
    printf("    <input type=hidden name=expid value=%s>\n", expid);
    printf("    <input type=hidden name=samplerate value=%g>\n", samplerate);
    printf("    <input type=hidden name=filtertype value=\"%s\">\n", ftype);
    if (logmin != 0.0) printf("    <input type=hidden name=logmin value=%17.10e>\n", logmin);
    printf("    Lower limit: <input size=8 name=flim1>\n");
    printf("    Upper limit: <input size=8 name=flim2>\n");
    printf("    <input type=submit value=zoom>\n");
    printf("</form>\n");
  }

static void mktimegraphs()
  { char cmd[MAXSTR+1];
    printf("<h2> Impulse response </h2>\n");
    printf("<ul>\n");
    printf("<li> <i>x</i> axis: time, in samples\n");
    printf("(i.e. %g represents 1 second)\n", samplerate);
    printf("<li> <i>y</i> axis (red): filter response (linear, normalized)\n");
    printf("</ul> <p>\n");
    sprintf(cmd, "cat %s/%s/params.mkf | %s/genplot -i %d %s/%s/T.gif", TEMP_DIR, expid, TOOLS_DIR, nirsteps, TEMP_DIR, mypid);
    obeycmd(cmd, true);
    printf("<img src=%s/%s/T.gif> <p>\n", TEMP_URL, mypid);
    printf("<h2> Step response </h2>\n");
    printf("<ul>\n");
    printf("<li> <i>x</i> axis: time, in samples\n");
    printf("(i.e. %g represents 1 second)\n", samplerate);
    printf("<li> <i>y</i> axis (red): filter response (linear, normalized)\n");
    printf("</ul> <p>\n");
    sprintf(cmd, "cat %s/%s/params.mkf | %s/genplot -s %d %s/%s/S.gif", TEMP_DIR, expid, TOOLS_DIR, nirsteps, TEMP_DIR, mypid);
    obeycmd(cmd, true);
    printf("<img src=%s/%s/S.gif> <p>\n", TEMP_URL, mypid);
    printf("For a view on a different scale, enter upper time limit (integer number of samples) here: <br>\n");
    printf("<form method=POST>\n");
    printf("    <input type=hidden name=expid value=%s>\n", expid);
    printf("    <input type=hidden name=samplerate value=%g>\n", samplerate);
    printf("    Upper limit: <input size=8 name=nirsteps>\n");
    printf("    <input type=submit value=zoom>\n");
    printf("</form>\n");
  }

static void dlcoeffs()
  { const char *str = getval("dlcoeffs");
    const char *lang = (str[0] == 'V') ? "" : "-xyc";
    discard_output();
    printf("Content-type: text/saveme\n\n");
    char cmd[MAXSTR+1];
    sprintf(cmd, "cat %s/%s/params.mkf | %s/gencode %s", TEMP_DIR, expid, TOOLS_DIR, lang);
    obeycmd(cmd, false);
  }

//static void redirect(char *url)
//  { discard_output();
//    printf("Status: 301 Moved Permanently\n");
//    printf("Content-type: text/html\n");
//    printf("Location: %s\n\n", url);
//    printf("<HTML>\n");
//    printf("<title> Automatic Redirection </title>\n");
//    printf("You are redirected <a href=%s>here</a>. <br>\n", url);
//  }

static void obeycmd(char *cmd, bool hfmt)
  { FILE *fi = do_popen(cmd);
    bool pr = false; int ital = 0;
    int ch = getc(fi);
    while (ch >= 0)
      { if (hfmt && !pr) { printf("<pre>\n"); pr = true; }
	if (hfmt)
	  { if (ch == '<') printf("&lt;");
	    else if (ch == '&') printf("&amp;");
	    else if (ch == '`' && ital++ == 0) printf("<i>");
	    else if (ch == '\'' && ital > 0 && --ital == 0) printf("</i>");
	    else putchar(ch);
	  }
	else putchar(ch);
	ch = getc(fi);
      }
    if (ital > 0) printf("</i>");
    if (pr) printf("</pre>\n");
    do_pclose(fi);
  }

static FILE *do_popen(char *cmd)
  { char cmdx[MAXSTR+1]; sprintf(cmdx, "(%s) 2>&1", cmd);
    FILE *fi = popen(cmdx, "r");
    if (fi == NULL) hfatal("popen failed.  Seek help.");
    return fi;
  }

static void do_pclose(FILE *fi)
  { int code = pclose(fi);
    if (code != 0) hfatal("Command failed (return code %d)", code);
  }

static void makefiltercmd(char *cmd)	/* make "mkfilter" command */
  { int p = 0;
    const char *ftype = getval("filtertype");
    if (seq(ftype, "Raised Cosine") || seq(ftype, "Hilbert Transformer"))
      { int nir = getival("impulselen", MB_PRES | MB_GT0);
	if (ftype[0] == 'R')
	  { double alpha = getalpha("corner", MB_PRES);
	    double beta = getfval("beta", MB_PRES | MB_GE0 | MB_LE1);       /* range 0 .. 1 */
	    appends(cmd, p, "%s/mkshape ", TOOLS_DIR);
	    const char *typ = getval("racos");
	    if (seq(typ, "yes") || seq(typ, "sqrt"))
	      { appends(cmd, p, (typ[0] == 'y') ? "-c " : "-r ");
		appendf(cmd, p, "%17.10e %17.10e ", alpha, beta);
	      }
	    else if (seq(typ, "no")) appends(cmd, p, "-i ");        /* identity function */
	    else hfatal("Bad racos: `%s'", typ);
	    appendi(cmd, p, "%d ", nir);
	    if (seq(getval("comp"), "yes")) appends(cmd, p, "-x "); /* x / sin x compensation */
	  }
	else
	  { appends(cmd, p, "%s/mkshape -h ", TOOLS_DIR);
	    appendi(cmd, p, "%d ", nir);
	  }
	if (isset("window")) appends(cmd, p, "-w ");            /* apply windowing */
	int nb = getival("bits", MB_GT0);                       /* truncate coeffs to nb bits */
	if (nb > 0) appendi(cmd, p, "-b %d ", nb);
      }
    else
      { appends(cmd, p, "%s/mkfilter ", TOOLS_DIR);
	if (seq(ftype, "Resonator")) makeres(cmd, p);
	else if (seq(ftype, "Pi")) makepif(cmd, p);
	else maketrad(cmd, p);
      }
  }

static void makeres(char *cmd, int &p)
  { double qf = getfval("qfactor", MB_PRES | MB_GT0);
    appendf(cmd, p, "-Re %17.10e ", qf);
    const char *ptype = getval("passtype");
    appendptype(cmd, p, ptype);	    /* -Lp etc. */
    double alpha = getalpha("centre", MB_PRES);
    appendf(cmd, p, "-a %17.10e -@", alpha);    // ???
  }

static void makepif(char *cmd, int &p)
  { double tau2 = getfval("tau2", MB_PRES | MB_GT0);
    appends(cmd, p, "-Pi -o 1 ");
    appendf(cmd, p, "-a %17.10e ", 1.0 / (TWOPI * tau2 * samplerate));
    if (isset("mzt"))
      { printf("<img src=//www.york.ac.uk/icons/warning.gif> ");
	printf("<b>Warning:</b> Using matched <i>z</i>-transform.\n");
	printf("<p>\n");
	appends(cmd, p, "-z ");
      }
  }

static void maketrad(char *cmd, int &p)
  { const char *ftype = getval("filtertype"); const char *ptype = getval("passtype");
    appends(cmd, p, "-%.2s ", ftype);  /* Bu/Be/Ch */
    double rip = getfval("ripple", MB_LT0);
    if (ftype[0] == 'C')
      { if (rip == 0.0)
	  { /* ripple param was not set */
	    hfatal("You've specified a Chebyshev design, so you have to give a ripple, e.g. \"-0.5\".");
	  }
	appendf(cmd, p, "%17.10e ", rip);
      }
    else
      { if (rip != 0.0)
	  { /* ripple param was set */
	    printf("Warning: ripple is meaningful only for Chebyshev designs; ");
	    printf("you've specified a %s design, so the ripple parameter will be ignored. <p>\n", ftype);
	  }
      }
    appendptype(cmd, p, ptype);	    /* -Lp etc. */
    int ord = getival("order", MB_PRES | MB_GT0);
    appendi(cmd, p, "-o %d ", ord);
    double alpha1 = getalpha("corner1", MB_PRES);
    double alpha2 = getalpha("corner2", UNCHECKED);     /* optional, returns -1.0 if not set */
    double alphaz = getalpha("adzero", UNCHECKED);      /* optional additional zero */
    if (ptype[0] == 'B' && alpha2 == 0.0)
      { hfatal("You've specified ``Bandpass'' or ``Bandstop'', "
	       "so you must specify a ``corner frequency 2'' value.");
      }
    if (ptype[0] == 'B' && alpha2 < alpha1)
      { hfatal("The upper corner frequency must be greater than the lower.");
      }
    if (ptype[0] != 'B' && alpha2 != 0.0)
      { printf("Warning: you've specified ``%s'', ", ptype);
	printf("so the ``corner frequency 2'' value you gave is redundant and will be ignored.\n");
	printf("<p>\n");
	alpha2 = 0.0;
      }
    if (isset("mzt"))
      { printf("<img src=//www.york.ac.uk/icons/warning.gif> ");
	printf("<b>Warning:</b> Using matched <i>z</i>-transform.\n");
	printf("<p>\n");
	appends(cmd, p, "-z ");
      }
    appendf(cmd, p, "-a %17.10e %17.10e ", alpha1, alpha2);
    if (alphaz != 0.0) appendf(cmd, p, "-Z %17.10e ", alphaz);
  }

static double getalpha(const char *key, uint chk)
  { const char *val = getval(key);
    if (val[0] == '\0')
      { if (chk & MB_PRES) hfatal("You must specify a value for ``%s frequency''.", key);
	return 0.0;	/* means "none specified" */
      }
    double cf = atof(val);
    double alpha = cf / samplerate;
    if (alpha <= 0.0) hfatal("The corner frequency/ies must be greater than zero.");
    if (alpha >= 0.5)
      { char temp[MAXSTR+1]; sprintf(temp, "%g", 0.5*samplerate);
	hfatal("The corner frequency/ies must be less than the Nyquist frequency, <br> "
	       "which is %s Hz (half the sample rate).", temp);
      }
    return alpha;
  }

static double getfval(const char *key, uint chk)
  { const char *val = isset(key) ? getval(key) : "";
    if (val[0] == '\0')
      { if (chk & MB_PRES) hfatal("You must specify a value for ``%s''.", key);
	return 0.0;	/* means "none specified" */
      }
    double dval = atof(val);
    if ((chk & MB_GT0) && !(dval > 0.0)) hfatal("``%s'' must be greater than zero.", key);
    if ((chk & MB_GE0) && !(dval >= 0.0)) hfatal("``%s'' must be greater than or equal to zero.", key);
    if ((chk & MB_LT0) && !(dval < 0.0)) hfatal("``%s'' must be less than zero.", key);
    if ((chk & MB_LE1) && !(dval <= 1.0)) hfatal("``%s'' must be less than or equal to one.", key);
    return dval;
  }

static int getival(const char *key, uint chk)
  { const char *val = isset(key) ? getval(key) : "";
    if (val[0] == '\0')
      { if (chk & MB_PRES) hfatal("You must specify a value for ``%s''.", key);
	return 0;	/* means "none specified" */
      }
    int ival = atoi(val);
    if ((chk & MB_GT0) && !(ival > 0)) hfatal("``%s'' must be greater than zero.", key);
    if ((chk & MB_GE0) && !(ival >= 0)) hfatal("``%s'' must be greater than or equal to zero.", key);
    if ((chk & MB_LT0) && !(ival < 0)) hfatal("``%s'' must be less than zero.", key);
    if ((chk & MB_LE1) && !(ival <= 1)) hfatal("``%s'' must be less than or equal to one.", key);
    return ival;
  }

static void appendptype(char *cmd, int &p, const char *pt)
  { if (seq(pt, "Lowpass")) appends(cmd, p, "-Lp");
    else if (seq(pt, "Highpass")) appends(cmd, p, "-Hp");
    else if (seq(pt, "Bandpass")) appends(cmd, p, "-Bp");
    else if (seq(pt, "Bandstop")) appends(cmd, p, "-Bs");
    else if (seq(pt, "Allpass"))  appends(cmd, p, "-Ap");
    /* else error will be picked up by mkfilter */
    cmd[p++] = ' ';
  }

static void appends(char *cmd, int &p, const char *fmt, const char *p1, const char *p2, const char *p3)
  { sprintf(&cmd[p], fmt, p1, p2, p3);
    until (cmd[p] == '\0') p++;
  }

static void appendi(char *cmd, int &p, const char *fmt, int p1, int p2)
  { sprintf(&cmd[p], fmt, p1, p2);
    until (cmd[p] == '\0') p++;
  }

static void appendf(char *cmd, int &p, const char *fmt, double p1, double p2)
  { sprintf(&cmd[p], fmt, p1, p2);
    until (cmd[p] == '\0') p++;
  }

static void printtrailer()
  { printf("<hr>\n");
    printf("<address>\n");
    printf("<a href=http://www-users.cs.york.ac.uk/~fisher>Tony Fisher</a>\n");
    printf("fisher@minster.york.ac.uk\n");
    printf("</address>\n");
  }

static void hfatal(const char *msg, ...)
  { va_list ap;
   	printf("<h2> Error! </h2>\n");
	va_start(ap, msg);
    vprintf(msg, ap);
	va_end(ap);
   	printf(" <p>\n");
    exit(0);
  }

