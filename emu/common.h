/*
 * common.h -- a set of common settings
 */

#ifndef COMMON_INCL
#define COMMON_INCL

/* Emulator constants definition */
#define SIMU    0            // 0: simple, 1: display register, 2: line-by-line exec
#define STRLEN  256          // decode string length

/* Debug tools */
#define DEBUG   1    // change to 1 prints mif strings to stdout
#define pc(x)   { if (DEBUG) printf("[%c]\n",  x); }  // print character
#define pd(x)   { if (DEBUG) printf("[%d]\n",  x); }  // print decimal
#define px(x)   { if (DEBUG) printf("[%04x]\n",x); }  // print hex
#define ps(x)   { if (DEBUG) printf("[%s]\n",  x); }  // print string
#define ps2(x,y){ if (DEBUG) printf("[%s: %s]\n",x,y); } // two strings
#define oops(x) { perror(x); exit(1); }                  // perror and exit
#define oops2(x,y) { fprintf(stderr, "%s: [%s]\n",x,y); exit(1); } // two strings

/* Font color */
#define RED0   "\x1B[7;31m"  // Reverse,      Red
#define RED1   "\x1B[0;31m"  // No attribute, Red
#define GRN0   "\x1B[7;32m"  // Reverse,      Green
#define GRN1   "\x1B[0;32m"  // No attribute, Green
#define YELO0  "\x1B[7;33m"  // Reverse,      Yellow
#define YELO1  "\x1B[0;33m"  // No attribute, Yellow
#define BLUE0  "\x1B[7;34m"  // Reverse,      Blue
#define BLUE1  "\x1B[4;34m"  // Underscore,   Blue
#define MGNT0  "\x1B[7;35m"  // Reverse,      Magenta
#define MGNT1  "\x1B[4;35m"  // Underscore,   Magenta
#define RESET  "\x1B[0m"

#endif /* COMMON_INCL */