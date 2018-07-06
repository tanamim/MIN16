/*
 * directives.h -- assembler directive handline functions
 */

#ifndef DIRECTIVES_INCL
#define DIRECTIVES_INCL

int check_directive(char*, int, FILE*);
int handle_label(char*, int );
int is_label(char*);

#endif /* DIRECTIVES_INCL */