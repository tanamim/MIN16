/*
 * decoder.c -- a set of decoding
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "strfunc.h"


/* Assembler addressing modes */
enum addr_mode {
    R1_TYPE = 0, R2_TYPE = 1, I_TYPE = 2, J_TYPE = 3, O_TYPE = 4, RSVD = 5,
};

/* addr_mode list of list */
static int addr_mode_list[14][4] = {
    {R2_TYPE, R2_TYPE, R2_TYPE, R2_TYPE},
    {R2_TYPE, R2_TYPE, R2_TYPE, R2_TYPE},
    {R2_TYPE, R2_TYPE, R2_TYPE, R2_TYPE},
    {R2_TYPE, R2_TYPE, R2_TYPE, R2_TYPE},
    {I_TYPE , I_TYPE , I_TYPE , I_TYPE },
    {I_TYPE , I_TYPE , I_TYPE , I_TYPE },
    {I_TYPE , I_TYPE , I_TYPE , I_TYPE },
    {I_TYPE , I_TYPE , I_TYPE , I_TYPE },
    {RSVD   , RSVD   , RSVD   , RSVD   },
    {RSVD   , RSVD   , RSVD   , RSVD   },
    {J_TYPE , J_TYPE , R1_TYPE, R2_TYPE},
    {O_TYPE , O_TYPE , RSVD   , RSVD   },
    {O_TYPE , O_TYPE , O_TYPE , O_TYPE },
    {R1_TYPE, R1_TYPE, R1_TYPE, R1_TYPE},
};

/* command name list of list */
static char* command_list[14][4] = {
    {"ADD"  , "SUB"  , "MUL"  , "SLT"  },
    {"ADDU" , "SUBU" , "MULU" , "SLTU" },
    {"AND"  , "OR"   , "XOR"  , "NOR"  }, 
    {"SLL"  , "SRL"  , "SRA"  , "ROTL" },
    {"ADDI" , "SUBI" , "MULI" , "SLTI" },
    {"ADDIU", "SUBIU", "MULIU", "SLTIU"},
    {"ANDI" , "ORI"  , "XORI" , "NORI" },
    {"SLLI" , "SRLI" , "SRAI" , "ROTLI"},
    {"RSVD" , "RSVD" , "RSVD" , "RSVD" },
    {"RSVD" , "RSVD" , "RSVD" , "RSVD" },
    {"J"    , "JAL"  , "JR"   , "JALR" },
    {"BEQ"  , "BNE"  , "RSVD" , "RSVD" },
    {"LW"   , "LB"   , "SW"   , "SB"   },
    {"MFHI" , "MFLO" , "MTHI" , "MTLO" },
};


/* opcode + func list of list */
static int opfunc_list[14][4] = {
    {0b000000, 0b000001, 0b000010, 0b000011},
    {0b000100, 0b000101, 0b000110, 0b000111},
    {0b001000, 0b001001, 0b001010, 0b001011}, 
    {0b001100, 0b001101, 0b001110, 0b001111}, 
    {0b010000, 0b010001, 0b010010, 0b010011},
    {0b010100, 0b010101, 0b010110, 0b010111},
    {0b011000, 0b011001, 0b011010, 0b011011},
    {0b011100, 0b011101, 0b011110, 0b011111},
    {      -1,       -1,       -1,       -1},
    {      -1,       -1,       -1,       -1},
    {0b101000, 0b101001, 0b101010, 0b101011},
    {0b101100, 0b101101,       -1,       -1},
    {0b110000, 0b110001, 0b110010, 0b110011},
    {0b110100, 0b110101, 0b110110, 0b110111}, 
};


/* Register value representation list. used for decoding. */
static char* regvals[] = {
    "$r0", "$at", "$sp", "$fp", "$ra", "$rb", "$rc", "$rd", 
    "$s0", "$s1", "$t0", "$t1", "$hi", "$lo", "$pc", "$fl",
};


/* instruction decoding functions */
char* decode(int num);
char* oparg1(int num);
char* oparg2(int num);
char* oparg3(int num);

/* data conversion functions */
char* int_to_opstr(int);
int   int_to_opfunc(int);
int   op_row(int);
int   func_col(int);


/**
* Instruction Decoding Functions 
*/

/* decode and return instruction */
char* decode(int num)
{
    static char decstr[STRLEN];
    static char* formats[] = {
        "(R1):  %s \t%s", 
        "(R2):  %s \t%s, %s", 
        " (I):  %s \t%s, %s", 
        " (J):  %s \t%s", 
        " (O):  %s \t%s, %s, %s",
        "(RSVD)"
    };
    int mode = addr_mode_list[op_row(num)][func_col(num)];
    sprintf(decstr, formats[mode], int_to_opstr(num), oparg1(num), oparg2(num), oparg3(num));
    return decstr;
}

char* oparg1(int num)
{
    int mode = addr_mode_list[op_row(num)][func_col(num)];
    switch(mode) {
        case R1_TYPE: return regvals[(num & 0b0000001111000000)>>6];
        case R2_TYPE: return regvals[(num & 0b0000001111000000)>>6];
        case I_TYPE:  return regvals[(num & 0b0000001111000000)>>6];
        case J_TYPE:  return int_to_hexstr(num & 0b0000001111111111);
        case O_TYPE:  return regvals[(num & 0b0000001110000000)>>7];
        case RSVD:    return "";
        default: oops2("oparg1", int_to_str(num))
    }
}

char* oparg2(int num)
{
    int mode = addr_mode_list[op_row(num)][func_col(num)];
    switch(mode) {
        case R1_TYPE: return regvals[(num & 0b0000000000111100)>>2];
        case R2_TYPE: return regvals[(num & 0b0000000000111100)>>2];
        case I_TYPE:  return int_to_hexstr(num & 0b0000000000111111);
        case J_TYPE:  return "";
        case O_TYPE:  return regvals[(num & 0b0000000001110000)>>4];
        case RSVD:    return "";
        default: oops2("oparg2", int_to_str(num))
    }
}

char* oparg3(int num)
{
    int mode = addr_mode_list[op_row(num)][func_col(num)];
    switch(mode) {
        case R1_TYPE: return "";
        case R2_TYPE: return "";
        case I_TYPE:  return "";
        case J_TYPE:  return "";
        case O_TYPE:  return int_to_hexstr(num & 0b0000000000001111);
        case RSVD:    return "";
        default: oops2("oparg3", int_to_str(num))
    }
}


/**
* Data Conversion Helper Functions 
*/

/* helper to slice opfunc and return opstr */
char* int_to_opstr(int num)
{
    return command_list[op_row(num)][func_col(num)];
}

/* helper to slice opfunc and return opfunc */
int int_to_opfunc(int num)
{
    return opfunc_list[op_row(num)][func_col(num)];
}

/* helper to get opcode */
int op_row(int num)
{
    return (num & 0b1111000000000000) >> 12;
}

/* helper to get func code */
int func_col(int num)
{
    return (num & 0b0000110000000000) >> 10;
}

/* helper to convert opfunc code str to int */
int opstr_to_opfunc(char* str)
{
    int rows = (sizeof(command_list)/sizeof(command_list[0]));
    int cols = (sizeof(command_list[0])/sizeof(command_list[0][0]));

    int r, c;
    for (r = 0; r < rows; r++)
        for(c = 0; c < cols; c++)
            if (strcmp(command_list[r][c], str) == 0)
                return opfunc_list[r][c];            
    oops2("opstr_to_opfunc: unexpected op code", str)    
}

/* Helper to convert opfunc int to opfunc str */
char* opfunc_to_opstr(int num)
{
    int op = (num & 0x3c) >> 2;
    int func = num & 0x3;
    return command_list[op][func];
}
