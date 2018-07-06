/*
 * regexp.c -- a set of regular expression related functions
 *
 * core functionality is pattern matcing and encode/decode to produce instructions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <math.h>
#include "linkedlist.h"
#include "regexp.h"
#include "strfunc.h"
#include "decoder.h"
#include "encoder.h"
#include "common.h"

/* Assembler constants definition */
#define MATCH   5                                     // num of regmatch sections
#define PATLEN  256                                   // max pattern string length

/* regexp pattern snippets */
#define R1_PAT  "JR|MFHI|MFLO|MTHI|MTLO"              // R1 is R_TYPE only needs 1 register
#define R2_PAT  "ADD|SUB|MUL|SLT|ADDU|SUBU|MULU|SLTU|AND|OR|XOR|NOR|SLL|SRL|SRA|ROTL|JALR"
#define I_PAT   "ADDI|SUBI|MULI|SLTI|ADDIU|SUBIU|MULIU|SLTIU|ANDI|ORI|XORI|NORI|SLLI|SRLI|SRAI|ROTLI"
#define J_PAT   "J|JAL"                     
#define O_PAT   "BEQ|BNE|LW|LB|SW|SB"
#define REGLIST "r0|at|sp|fp|ra|rb|rc|rd|s0|s1|t0|t1" // general purpose registerd (0-11)
#define REGSUB  "r0|at|sp|fp|ra|rb|rc|rd"             // sub list of registerd (0-7)
#define IMM     ".+"                                  // arithmetic string, 0x00  - 0x3f
#define TARGET  ".+"                                  // arithmetic string, 0x000  - 0x3ff
#define OFFS    ".+"                                  // arithmetic string, 0x0  - 0xf
#define ENDPAT  ",{0}(( *$)|( *#+.*$))"               // this pattern should come at the end

/* regex pattern string generator */
regex_t* get_preg_blank();
regex_t* get_preg_R1();
regex_t* get_preg_R2();
regex_t* get_preg_I();
regex_t* get_preg_J();
regex_t* get_preg_O();

/*  
  name:    pattern
  purpose: create an assembler pattern struct
  field:   str        - pattern string used by regexp
           *f1        - function pointer to generate pattern
           *f2        - function pointer to encode
  note:    all the regex_t in the pattern_table should be freed by regfree_all.
*/
struct pattern {
    char*    str;
    regex_t* (*f1)();
    int      (*f2)(char*, regmatch_t*);
};

/* file scope table sructure to register addressing type and function pointers */
static struct pattern pattern_table[] = {
    {"BLANK", get_preg_blank, encode_blank},
    {"R1_PAT", get_preg_R1, encode_R1},
    {"R2_PAT", get_preg_R2, encode_R2},
    {"I_PAT" , get_preg_I,  encode_I},
    {"J_PAT" , get_preg_J,  encode_J},
    {"O_PAT" , get_preg_O,  encode_O},
    {NULL, NULL, NULL}
};


/**
* Shared functions (regexp.h)
*/

/* go through pattern_table to try pattern match and encode the string */
int check_pattern(char* str)
{
    if (strlen(str) == 0) return 0; // just blank    
    regmatch_t pmatch[MATCH];       // array of matched object
    regex_t* preg;
    int i;
    for (i = 0; pattern_table[i].str != NULL; i++) {
        preg = pattern_table[i].f1();
        if (regexec(preg, str, MATCH, pmatch, 0) == 0) {
            return pattern_table[i].f2(str, pmatch);                    
        }
    }
    return -1;  // wrong format
}

/* free all regex_t in the static pattern_table */
void regfree_all()
{
    int i;
    for (i = 0; pattern_table[i].str != NULL; i++) {
        regex_t* preg = pattern_table[i].f1();
        regfree(preg);
    }
}


/**
* Regex pattern string generator
*/

/* helper to compile regexp with error message */
void regex_compile(regex_t* preg, char* pattern)
{
    if (regcomp(preg, pattern, REG_EXTENDED|REG_NEWLINE|REG_ICASE) != 0) 
        oops("check_pattern: regcomp failed.")
}

/* return matched regmatch_t pointer for BLANK */
regex_t* get_preg_blank()
{
    static regex_t re;
    if (re.re_nsub == 0) {
        char* pattern = "(^[ *#+])";
        regex_compile(&re, pattern);
    }
    return &re;    
}


/* return matched regmatch_t pointer for R1_PAT */
regex_t* get_preg_R1()
{
    static regex_t re;
    if (re.re_nsub == 0) {
        char pattern[PATLEN];
        sprintf(pattern, "(%s) +\\$(%s)%s", R1_PAT, REGLIST, ENDPAT);
        regex_compile(&re, pattern);
    }
    return &re;
};

/* return matched regmatch_t pointer for R2_PAT */
regex_t* get_preg_R2()
{
    static regex_t re;
    if (re.re_nsub == 0) {
        char pattern[PATLEN];
        sprintf(pattern, "(%s) +\\$(%s), +\\$(%s)%s", R2_PAT, REGLIST, REGLIST, ENDPAT); 
        regex_compile(&re, pattern);
    }
    return &re;
};

/* return matched regmatch_t pointer for I_PAT */
regex_t* get_preg_I()
{
    static regex_t re;
    if (re.re_nsub == 0) {
        char pattern[PATLEN];
        sprintf(pattern, "(%s) +\\$(%s), (%s)%s", I_PAT, REGLIST, IMM, ENDPAT); 
        regex_compile(&re, pattern);
    }
    return &re;
};

/* return matched regmatch_t pointer for J_PAT */
regex_t* get_preg_J()
{
    static regex_t re;
    if (re.re_nsub == 0) {
        char pattern[PATLEN];
        sprintf(pattern, "(%s) +(%s)%s", J_PAT, TARGET, ENDPAT); 
        regex_compile(&re, pattern);
    }
    return &re;
};

/* return matched regmatch_t pointer for O_PAT */
regex_t* get_preg_O()
{
    static regex_t re;
    if (re.re_nsub == 0) {
        char pattern[PATLEN];
        sprintf(pattern, "(%s) +\\$(%s), +\\$(%s), +(%s)%s", O_PAT, REGSUB, REGSUB, OFFS, ENDPAT); 
        regex_compile(&re, pattern);
    }
    return &re;
};



/* DEBUG */
void regexp_test()
{
    // ps(opfunc_to_opstr(0b000011))
    // ps(opfunc_to_opstr(41))
    // ps(decode(0x2040))
    // ps(decode(0x704a))
    // ps(decode(0x6440))

    // ps(0b1111 < 0b11111 ? "a" : "b")
    // ps(int_to_bin(-1))
    // pd(strop("-10"))
}


