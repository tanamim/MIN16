/*
 * strfunc.h -- set of data manipulating helper functions
 */

#ifndef STRFUNC_INCL
#define STRFUNC_INCL

void  err_msg(char* a, char* b);
char* concat(char* a, char* b);
char* concat2(char* a, char* b);
char* no_comment_str(char* str);
char* strip(char* str);
char* get_last_char(char* str);
char* remove_space(char* str);
int   is_octal(char* str);
int   str_to_int(char* str);
char* int_to_str(int num);
char* int_to_hexstr(int num);
char* int_to_bin(int num);
char* trimmed(char* str, char c);
char* getlabel(char* str);
void  swap_str(char* line, char* str, char* newstr);
char* search_chr(char* str, char c);

int   strop(char* str);
void  strfunc_test();  // DEBUG

#endif /* STRFUNC_INCL */