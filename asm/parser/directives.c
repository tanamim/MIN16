/*
 * directives.c -- assembler directive handline functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include "linkedlist.h"
#include "directives.h"
#include "strfunc.h"
#include "common.h"

extern void write_mif(int, int, char*, FILE*);        // defined in parser.c
extern struct list* get_label_list();                 // defined in parser.c
extern void update_address(int);                      // defined in parser.c    

/* directive handlers */
int handle_space(char*, int, FILE*);
int handle_word(char*, int, FILE*);
int handle_half(char*, int, FILE*);
int handle_byte(char*, int, FILE*);
int handle_ascii(char*, int, FILE*);
int handle_asciiz(char*, int, FILE*);
int handle_org(char*, int, FILE*);
int handle_align(char*, int, FILE*);

/*  
  name:    directive
  purpose: create an assembler directive struct
  field:   str        - directive string used by regexp
           *f         - function pointer to handle directive
*/
struct directive {
    char*    str;
    int (*f)(char*, int, FILE*);
};


/* file scope table sructure to register directive type and function pointers */
static struct directive directive_table[] = {
    {".space",  handle_space},
    {".word",   handle_word},
    {".half",   handle_half},
    {".byte",   handle_byte},
    {".ascii",  handle_ascii},
    {".asciiz", handle_asciiz},
    {".org",    handle_org},
    {".align",  handle_align},
    {NULL, NULL}
};

/* helper to get arg part of directive string */
char* get_args(char* str, char* directive)
{
    return strip(strstr(str, directive) + strlen(directive));
}

/**
* Shared functions (directives.h)
*/

/* helper for check_directive to slice out directive */
char* get_directive(char* str)
{
    static char tmp[STRLEN];
    strcpy(tmp, str);
    char* comm = strchr(tmp, '#');
    if (comm) *comm = '\0';

    char* start;
    char* label = strchr(tmp, ':');
    if (label) start = label + 1;

    start = strchr(tmp, '.');
    if (!start) return NULL;
    char* end = strchr(start, ' ');
    if (!end) return NULL;
    *end = '\0';

    return start;
}

/* go through directive_table to try directive match and encode the string */
int check_directive(char* str, int address, FILE* fp)
{
    int i;
    char* directive;
    directive = get_directive(str);
    if (directive == NULL) return -1;

    for (i = 0; directive_table[i].str != NULL; i++) {
        if (strcmp(directive, directive_table[i].str) == 0) {
            int rv = directive_table[i].f(str, address, fp); 
            return rv;
        }
    }
    return -1;  // not a directive
}


/***
*  o To set a label to a specified value
*    label: nothing 
*    label:    .equ    <value>
*/
/* check if label exists, then check .equ exists. return label value */
int handle_label(char* str, int address)
{
    char tmp[STRLEN];
    strcpy(tmp, str);

    char* p = search_chr(tmp, '#');
    if (p != NULL) *p = '\0';  // slice out if comment exists
    p = tmp;

    p = search_chr(p, ':');    
    if (p == NULL) return -1;  // not a label
    *p = '\0';                 // else slice out the label

    p = strstr(p + 1, ".equ");
    
    // if label found and no .equ
    if (p == NULL) {
        return address;
    }

    // else remove "." for .equ directive search 
    *p = '\0';                 
    return strop(p + strlen(".equ") + 1);
}

/* check if label exists */
int is_label(char* str)
{
    char* p = str;
    p = search_chr(p, ':');
    
    if (p == NULL) return 0;  // not a label
    return 1;    
}

/**
* Directive handler functions
*/

/***
*  o To reserve space
*    .space    <number of bytes>   # skip over specified number of bytes
*/
int handle_space(char* str, int address, FILE* fp)
{ 
    return address + strop(get_args(str, ".space"));
}


/* helper to write toknized arguments and return address */
int write_and_update_address(char* str, int address, FILE* fp, int increment)
{
    char* tok = strtok(no_comment_str(str), " \t");
    for (;;)
    {
        tok = strtok(NULL, " \t");
        if (tok == NULL) break;
        if (fp) write_mif(address, strop(tok), concat2(str, tok), fp);
        address += increment;
    }
    return address;     
}

/***
*  o To reserve space and initialize it to particular values
*    .word <w1>,<w2>, ... ,<wn>
*    # .word also forces alignment to a word boundary
*    .half <h1>,<h2>, ... ,<hn>
*    # .half also forces alignment to a halfword boundary
*    .byte <b1>,<b2>, ... ,<bn>
*    .ascii    <string>        # string in double quotes
*    .asciiz   <string>        # null terminated string
*/
int handle_word(char* str, int address, FILE* fp)
{ 
    return write_and_update_address(str, address, fp, 2);
}

int handle_half(char* str, int address, FILE* fp)
{ 
    return write_and_update_address(str, address, fp, 1);
}

int handle_byte(char* str, int address, FILE* fp)
{ 
    return write_and_update_address(str, address, fp, 1);
}

int handle_ascii(char* str, int address, FILE* fp)
{ 
    char chararr[] = "Z";
    char* s = get_args(str, ".ascii");
    s++;  // skip '"'
    for (; *s != '\"'; s++) {
        *chararr = *s;
        if (fp) write_mif(address, (int) *chararr, concat2(trimmed(str, '\n'), chararr), fp);
        address += 2;
    }    
    return address; 
}

int handle_asciiz(char* str, int address, FILE* fp)
{
    char chararr[] = "Z";
    char* s = get_args(str, ".asciiz");
    s++;  // skip '"'
    for (; *s != '\"'; s++) {
        *chararr = *s;
        if (fp) write_mif(address, (int) *chararr, concat2(trimmed(str, '\n'), chararr), fp);
        address += 2;
    }    
    *chararr = '\0';
    if (fp) write_mif(address, (int) *chararr, concat2(trimmed(str, '\n'), chararr), fp);
    address += 2;

    return address; 
}

/***
*  o To set the location counter
*    .org  <location>
*/
int handle_org(char* str, int address, FILE* fp)
{ 
    char* new_address_str = get_args(str, ".org");
    return strop(new_address_str); 
}

/***
*  o To force alignment
*   .align    <n>         # align next item on a 2^n byte boundary
*/
int handle_align(char* str, int address, FILE* fp)
{ 
    int n = strop(get_args(str, ".align"));
    int aln = pow(2,n);
    int quo = address / aln;
    int adj = address % aln == 0 ? 0 : 1;
    return aln * (quo + adj);
}



