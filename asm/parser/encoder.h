/*
 * encoder.h -- encoding functions
 */

#ifndef ENCODER_INCL
#define ENCODER_INCL

/* instruction encoding functions */
int encode_blank(char*, regmatch_t*);
int encode_R1(char*, regmatch_t*);
int encode_R2(char*, regmatch_t*);
int encode_I(char*, regmatch_t*);
int encode_J(char*, regmatch_t*);
int encode_O(char*, regmatch_t*);

#endif /* ENCODER_INCL */