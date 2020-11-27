#ifndef LCFILTER_H_
#define LCFILTER_H_

#include <string.h>

#define unless(x)   if(!(x))
#define until(x)    while(!(x))

#define MAXSTR	    256
#define MAXORDER    10

extern float *coeffs(const char*);

inline bool seq(const char *s1, const char *s2)    { return strcmp(s1,s2) == 0;		  }
inline bool starts(const char *s1, const char *s2) { return strncmp(s1, s2, strlen(s2)) == 0; }

#define tabindex(n) ((n)*((n)+1)/2)	/* index triangular array */

#endif /* LCFILTER_H_ */