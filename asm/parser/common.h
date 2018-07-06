/*
 * common.h -- a set of common settings
 */

#ifndef COMMON_INCL
#define COMMON_INCL

/* Debug tools */
#define DEBUG   0    // change to 1 prints mif strings to stdout
#define pc(x)   { if (DEBUG) printf("[%c]\n",  x); }  // print character
#define pd(x)   { if (DEBUG) printf("[%d]\n",  x); }  // print decimal
#define px(x)   { if (DEBUG) printf("[%04x]\n",x); }  // print hex
#define ps(x)   { if (DEBUG) printf("[%s]\n",  x); }  // print string
#define ps2(x,y){ if (DEBUG) printf("[%s: %s]\n",x,y); } // two strings
#define oops(x) { perror(x); exit(1); }                  // perror and exit
#define oops2(x,y) { fprintf(stderr, "%s: [%s]\n",x,y); exit(1); } // two strings
#define STOP    { fflush(stdout); sleep(3); }
#define pstop(x){ if (DEBUG) printf("%s: [%s]\n",__func__, x == NULL ? "null" : x); fflush(stdout); sleep(3); }

/* Assembler constants definition */
#define STRLEN  256                                   // decode string length

#endif /* COMMON_INCL */