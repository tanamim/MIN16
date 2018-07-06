/*
 * decoder.h -- decoding functions
 */

#ifndef DECODER_INCL
#define DECODER_INCL

char* decode(int num);
int   opstr_to_opfunc(char*);
char* opfunc_to_opstr(int num);

#endif /* DECODER_INCL */