#define global
#define bool	    int
#define false	    0
#define true	    1
#define unless(x)   if(!(x))
#define until(x)    while(!(x))
#define seq(s1,s2)  (strcmp(s1,s2) == 0)

#define MAXSTRLEN 80

#include "gfxlib.h"

#include <stdio.h>

extern struct bitmap *ReadBitmap(), *Balloc();


global main(argc, argv) int argc; char *argv[];
  { bool ok, border; struct bitmap *bm1, *bm2;
    int bbox[4]; int wid, hgt;
    border = false;
    if (argc >= 2 && seq(argv[1],"-b"))
      { border = true;
	argc--; argv++;
      }
    unless (argc == 3) usage();
    bm1 = ReadBitmap(argv[2]);
    if (bm1 == NULL) giveup("can't read bitmap file %s", argv[2]);
    getbbox(argv[1], bbox);
    wid = bbox[2] - bbox[0];
    hgt = bbox[3] - bbox[1];
    bm2 = Balloc(wid, hgt, 1);
    fpr_rop(bm2, 0, 0, wid, hgt, F_STORE, bm1, bbox[0], (bm1 -> hgt) - bbox[3]);
    if (border) fpr_border(bm2, 0, 0, wid, hgt, F_SET, NULL, 0, 0);
    ok = WriteBitmap(bm2, argv[2], 'c'); /* write in xor'd binary format */
    unless (ok) giveup("can't write bitmap file %s", argv[2]);
    exit(0);
  }

static usage()
  { fprintf(stderr, "Usage: cropbm [-b] fn.ps fn.bm\n");
    exit(1);
  }

static getbbox(fn, bbox) char *fn; int *bbox;
  { char line[MAXSTRLEN]; int nc; bool ok;
    FILE *fi = fopen(fn, "r");
    if (fi == NULL) giveup("can't read PostScript file %s", fn);
    nc = rdline(fi, line);
    ok = false;
    while (nc >= 0 && !ok)
      { if (strncmp(line, "%%BoundingBox:", 14) == 0)
	  { int ni = sscanf(&line[14], " %ld %ld %ld %ld", &bbox[0], &bbox[1], &bbox[2], &bbox[3]);
	    if (ni == 4) ok = true;
	  }
	nc = rdline(fi, line);
      }
    unless (ok) giveup("no valid %%%%BoundingBox in file %s", fn);
    fclose(fi);
  }

static int rdline(fi, line) FILE *fi; char line[];
  { int nc = 0;
    int ch = getc(fi);
    until (ch == '\n' || ch == EOF || nc >= MAXSTRLEN)
      { line[nc++] = ch;
	ch = getc(fi);
      }
    if (ch == EOF) nc = -1;
    /* we don't care if the line's too long;
       we're only looking at the first few chars anyway */
    until (ch == EOF || ch == '\n') ch = getc(fi);
    return nc;
  }

static giveup(msg, p1) char *msg, *p1;
  { fprintf(stderr, "cropbm: "); fprintf(stderr, msg, p1); putc('\n', stderr);
  }

