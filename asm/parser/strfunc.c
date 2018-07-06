/*
 * strfunc.c -- set of data manipulating helper functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include "strfunc.h"
#include "linkedlist.h"
#include "common.h"

/* Assembler constants definition */
#define NUMLEN     20  // "0b" plus 16 bit

extern struct list* get_label_list();                 // defined in parser.c
extern int          get_address();                    // defined in parser.c
extern FILE*        get_fp();                         // defined in parser.c
extern void         used_label_found();               // defined in parser.c


/**
* Data Conversion Helper Shared Functions 
*/

/* error message */
void err_msg(char* a, char* b)
{
    fprintf(stderr, "%s: [%s]\n", a, b);
}

/* return concatnated string */
char* concat(char* a, char* b)
{
    static char str[STRLEN];
    memset(&str, 0, sizeof(str));
    sprintf(str, "%s%s", a, b);
    return str;
}

char* concat2(char* a, char* b)
{
    static char str[STRLEN];
    memset(&str, 0, sizeof(str));
    sprintf(str, "%s: %s", a, b);
    return str;
}

/* remove comment from string */
char* no_comment_str(char* str)
{
    char* p = strchr(str, '#');
    if (p != NULL) *p = '\0';
    return str;    
}

/* remove leading white-space character */
char* strip(char* str)
{
    char* s = str;
    for(; isspace(*s); s++) {}
    return s;
}

/* helper to return non-whitespace last char */
char* get_last_char(char* str)
{
    int i = strlen(str) - 1;
    for(; i != 0; i--) {
        if (!isspace(str[i])) break;
    }
    return &str[i];
}

/* remove leading and trailing white spaces */
char* remove_space(char* str)
{
    char* s = strip(str);
    *(get_last_char(s) + 1) = '\0';  // remove space
    return s;
}

/* helper to check if the string is octal */
int is_octal(char* str)
{    
    char* p = strip(str);
    if (*p != '0') return 0;  // not start with '0'

    for (p++; *p != '\0' && *p != ' '; p++) {
        if (*p < '0' || *p > '7') return 0;
    }
    return 1;
}


/* convert a number string to int */
int str_to_int(char* str)
{
    char* result;
    if ((result = strstr(str, "0x")) != NULL)
        return (int) strtol(result + 2, NULL, 16);
    if ((result = strstr(str, "0b")) != NULL)
        return (int) strtol(result + 2, NULL, 2);
    if (is_octal(str))
        return (int) strtol(str, NULL, 8);
    return strtol(str, NULL, 10);
}

/* int to string */
char* int_to_str(int num)
{
    static char str[STRLEN];
    sprintf(str, "%d", num);
    return str;
}

/* int to hex string */
char* int_to_hexstr(int num)
{
    static char str[STRLEN];
    sprintf(str, "0x%x", num);
    return str;
}

/* helper to convert integer to binary string */
char* int_to_bin(int num)
{
    static char str[23];
    memset(str, 0, sizeof str);
    char* s = str;

    // initial character for binary representation
    *s++ = '0';
    *s++ = 'b';
    *s++ = ' ';

    // hexadecimal 2**16
    int mask = 0x10000;

    // mask each num
    while (mask >>= 1) {
        // insert space every 4 characters
        if ((str - s) % 5 + 2 == 0)
            *s++ = ' ';
        *s++ = !(mask & num) ? '0' : '1';
    }
    *s = '\0';
    return str;    
}

/* helper to return new string trimmed by the char */
char* trimmed(char* str, char c)
{
    static char trimstr[STRLEN];
    memset(trimstr, 0, sizeof trimstr);
    strcpy(trimstr, str);
    char* p;
    if ((p = strchr(trimstr, c)) != NULL)
        *p = '\0';
    return trimstr;
}

/* helper to get label */
char* getlabel(char* str)
{
    static char tmp[STRLEN];
    memset(tmp, 0, sizeof tmp);
    strcpy(tmp, strip(str));
    char* p = search_chr(tmp, ':');  
    if (p == NULL) oops2("getlabel: label dosen't exist", str);  // no label
    if(p) *p = '\0';    // slice out the label    
    return tmp;
}

/* helper to overwrite to make a replaced line */
void swap_str(char* line, char* str, char* newstr)
{
    if (STRLEN - strlen(line) - 1 < newstr - str) 
        oops("swap_str: too long new string")
    char* result = strstr(line, str);
    if (result == NULL) return;
    char rest[STRLEN];                     // the rest of the replaced str
    strcpy(rest, result + strlen(str));    // create copy of the rest
    sprintf(result, "%s%s", newstr, rest); // concat newstr and the rest
}

/* helper to search char exclude double quote */
char* search_chr(char* str, char c)
{
    char* p = str;
    int in_quote = 0;  // FSM
    for(; *p != '\0'; p++)
    {
        if (in_quote && *p == '\"') in_quote = 0;  // turn off
        if (!in_quote && *p == '\"') in_quote = 1; // turn on
        if (!in_quote && (*p == c)) return p;
    }
    return NULL;
}



/**
* String arighmetic and logical operations functions
*/

//   o Arithmetic expressions
//     o decimal numbers
//     o hexadecimal numbers
//     o octal numbers
//     o some C-like operators
//     o labels can be used as the location counter where they are defined
//     o ASCII character constants (in single quotes)
//     o $ by itself is the current value of the location counter


/* file scope variable */
static char* operators = "*/+-%%&^|";

/* function declarations */
int   strop_unsigned(char* str);
char* in_paren(char* str);
int   no_number_handler(char* str);
char* get_replaced_str(char* str);  // TODO - use dictionary for replace
char* get_fst_op(char* str);
int   opcmp(char c1, char c2);
int   get_op_priority(char c);
int   operate(int l, int r, char op);

/*
* String arithmetic operation function
*
* Evaluate a string of arithmetic expressions 
* and perform arithmetic and logical operations (+, -, *, /, %, &, ^, |)
* 
* This algorithm is original, and a kind of divide and conquer algorithm.
*
* The rule of dividing is to compare the first two operators, and split 
* by the lower priority operator. This is because higher priority operator
* has strong associativity to operands.
*
* When looking for an operator, parenthesis is ignored because it has
* higher priority. What we care is operators before parenthesis and after. 
*
* 0) 1 has no operator and is a base case, then returning integer value
* 1) 1+2 is split by '+' and divided to (1) + (2)
* 2) 1*2 is split by '*' and divided to (1) * (2)
* 3) 1+2*3 is split by '+' and divided to (1) + (2*3)
* 4) 1*2+3 is split by '+' and divided to (1*2) + 3
* 5) 1*(2+3)+4 is is split by '+' and divided to (1*(2+3)) + 4
* 6) 1+(2+3)+4 is is split by '+' and divided to (1+(2+3)) + 4
*
*/
int strop(char* original)
{
    char copy[STRLEN];
    strcpy(copy, no_comment_str(original));
    char* str = in_paren(copy);
    if (*str == '-') 
        return -1 * strop_unsigned(++str);
    return strop_unsigned(str);
}


int strop_unsigned(char* str)
{
    char* op1 = get_fst_op(str);    // no operator means base case
    if (op1 == NULL) {

        int n = str_to_int(str);
        if (errno == 0) return n;   // strtol is successful
        errno = 0;                  // reset global variable for next call
        return no_number_handler(str);  // string substitution
    }

    char* op2 = get_fst_op(op1+1);  // second operator
    if (op2 == NULL || opcmp(*op1, *op2) > 0) { // 1 + 2 * 3
        char op = *op1;  // copy operation char
        *op1 = '\0';     // split str at op1
        return operate(strop(str), strop(op1+1), op);    
    } else {                                    // 1 * 2 + 3
        char op = *op2;
        *op2 = '\0';       
        return operate(strop(str), strop(op2+1), op);
    }
}


/* helper to remove parenthesis revursively so `(str)` will be `str` */
/* not that original string will be modified */
char* in_paren(char* str)
{
    char* s = remove_space(str);
    char* e = get_last_char(str);
    if ((*s != '(') || (*e != ')')) {
        return s;
    }
    *e = '\0';
    return in_paren(s+1);  
}

/* handle non-number string */
int no_number_handler(char* str)
{
    if (strcmp(str, "'\\n'") == 0) 
        return 10;  // special case. new line is decimal 10.    
    str = in_paren(str);
    char* newstr = get_replaced_str(str);
    if (newstr == NULL)  
        oops2("strop: undefined number representation", str); 
    return str_to_int(newstr);
}

/* helper to remove single quote to return a char ptr. NULL on error */
char* in_quote(char* str)
{
    char* s = strip(str);
    char* e = get_last_char(str);    

    if ((*s != '\'') || (*e != '\'')) {
        return NULL;                
    }
    *e = '\0';

    return s+1;
}

/* helper: substitute string with number. return NULL on error */
char* get_replaced_str(char* str)
{
    struct list* lp = get_label_list();  // API from parser.c

    // special case, if any
    char* key = "hello";
    char* value = "0xff";
    if (strcmp(str, key) == 0) return value;  // special case

    int num;
    char* charptr;
    if ((strchr(str, '$')) != NULL) {
        num = get_address();        // API from parser.c        
    } 
    else if ((charptr = in_quote(str)) != NULL) {
            num = (int) *charptr;   // one character
    }
    else {
        char* tok = strtok(str, " \t#");
        num = getnum(lp, tok);                  // API from linkedlist.c

        if (num == -1) {
            if (get_fp() != NULL) return NULL;  // not found when encoding line
         
            // NOTE - How to handle label address when auto-gen happens where the label is used?
            //        Address is adjusted during encoding. One use of label cause auto-gen instructions.
            //        Until label address is determined, we cannot tell auto-gen happens or not.
            //        Also, a label address depends on all the other labels because they can
            //        trigger another auto-gen.
            //        Here, while not encoding, we set num as zero just to avoid auto-gen.
            //        After both label_list and used_list completes, addr_resolution in linkedlist.c
            //        caribrate additional address increase to update correct label address.

            num = 0;  // intentionally zero no to cause auto-gen
 
            used_label_found();   // API from parser.c
        }
    }
    static char numstr[NUMLEN];
    sprintf(numstr, "%d", num);
    return numstr;
}


/* return the first appearing operator but skip over parenthesis */
char* get_fst_op(char* str)
{

    str = remove_space(str);

    if (strcmp(str, "'-'") == 0) return NULL; // negative sign, not minus

    char* p;
    char* op;
    int paren = 0;
    for (p = str; *p != '\0'; p++) {
        for (op = operators; *op != '\0'; op++) {
            if (*p == '(') { paren++; break; }
            if (*p == ')') { paren--; break; }
            if (*p == *op && paren == 0) return p;
        }
    }
    return NULL;
}


/* helper for op char comparison if grater than zero, c1 < c2 */
int opcmp(char c1, char c2)
{    
    return get_op_priority(c2) - get_op_priority(c1);
}

/* convert operation char to priority represented in int */
int get_op_priority(char c)
{
    switch(c) {
        case '*': return 5;
        case '/': return 5;
        case '%': return 5;
        case '+': return 4;
        case '-': return 4;
        case '&': return 3;
        case '^': return 2;
        case '|': return 1;
        default: oops("get_op_priority: undefined operation priority.")
    }
}

/* define and apply operation character */
int operate(int l, int r, char op)
{
    switch(op) {
        case '*': return l * r;
        case '/': return l / r;
        case '%': return l % r;
        case '+': return l + r;
        case '-': return l - r;
        case '&': return l & r;
        case '^': return l ^ r;
        case '|': return l | r;
        default: oops("operate: unknown operation character.")
    }    
}

/* Testing - uncomment to test in main function in parser.c */
void strfunc_test()
{
    pd(strop("1"));
    pd(strop("1+2"));
    pd(strop(" (1) +2"));
    pd(strop("(1)+(2)"));
    pd(strop("(1)+2"));
    pd(strop("1+(2)"));
    pd(strop("3*4"));
    pd(strop("1+2*3"));
    pd(strop("1+(2+3)*4"));
    pd(strop("(2+3) *4"));
    pd(strop("(2+3)"));
    pd(strop("(1)"));
    pd(strop("((1))"));
    pd(strop("(((1)))"));
    pd(strop("1+2+3+4+2*(3+4)+5"));
    pd(strop("1*(2*(3+4)+5)+6"));
    pd(strop("(2*(3+4)+5)"));
    pd(strop("(0x2*(010+0b10)+0b101*2)")); // 2*(8+2)+5*2 = 30
    pd(strop(" ( 0x2 * ( 010 + 0b10 ) + 0b101 * 2 ) ")); // 2*(8+2)+5*2 = 30
    pd(strop("(0x2 + 0x4)"));
    pd(strop("0x10 + 0x4"));
    pd(strop(" ( hello )"));
    pd(strop("0b0111"));
    pd(strop(" 0b1 +  ( hello )"));
}

