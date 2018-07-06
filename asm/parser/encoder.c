/*
 * encoder.c -- a set of encoding
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <regex.h>
#include "linkedlist.h"
#include "encoder.h"
#include "decoder.h"
#include "common.h"
#include "strfunc.h"

#define CODELEN 6                                     // num of char in opfunc code incl '\0'
#define REGNUM  16                                    // num of registers
#define NUMSTR  512                                   // TARGET/OFFSET/IMMEDIATE number string size
#define DEPTH   32768                                 // max number of instructions


/* Assembler command mapping with OP + FUNC */
enum opfunc {
    ADD   = 0b000000, SUB   = 0b000001, MUL   = 0b000010, SLT   = 0b000011,
    ADDU  = 0b000100, SUBU  = 0b000101, MULU  = 0b000110, SLTU  = 0b000111,
    AND   = 0b001000, OR    = 0b001001, XOR   = 0b001010, NOR   = 0b001011, 
    SLL   = 0b001100, SRL   = 0b001101, SRA   = 0b001110, ROTL  = 0b001111, 
    ADDI  = 0b010000, SUBI  = 0b010001, MULI  = 0b010010, SLTI  = 0b010011,
    ADDIU = 0b010100, SUBIU = 0b010101, MULIU = 0b010110, SLTIU = 0b010111,
    ANDI  = 0b011000, ORI   = 0b011001, XORI  = 0b011010, NORI  = 0b011011,
    SLLI  = 0b011100, SRLI  = 0b011101, SRAI  = 0b011110, ROTLI = 0b011111,
    J     = 0b101000, JAL   = 0b101001, JR    = 0b101010, JALR  = 0b101011,
    BEQ   = 0b101100, BNE   = 0b101101,
    LW    = 0b110000, LB    = 0b110001, SW    = 0b110010, SB    = 0b110011,
    MFHI  = 0b110100, MFLO  = 0b110101, MTHI  = 0b110110, MTLO  = 0b110111, 
};

/* Assembler registers */
enum reg {
    R0, AT, SP, FP, RA, RB, RC, RD, S0, S1, T0, T1, HI, LO, PC, FL,
};

/* autogen boundary list of list (IJO type has boundary) 
   NOTE - this boudary is with abs value */
static int autogen_boundary_list[14][4] = {
    {0xffff, 0xffff, 0xffff, 0xffff},  // R_TYPE has no autogen
    {0xffff, 0xffff, 0xffff, 0xffff},
    {0xffff, 0xffff, 0xffff, 0xffff},
    {0xffff, 0xffff, 0xffff, 0xffff},
    {0x001f, 0x001f, 0x001f, 0x001f},  // I_TYPE has 6 bits but signed,   so check 5 bits
    {0x003f, 0x003f, 0x003f, 0x003f},  // I_TYPE has 6 bits but unsigned, so check 6 bits
    {0x003f, 0x003f, 0x003f, 0x003f},  // I_TYPE has 6 bits but logical,  so check 6 bits
    {0x003f, 0x003f, 0x001f, 0x003f},  // SRAI is signed, so check 5 bits
    {0xffff, 0xffff, 0xffff, 0xffff},  // RSVD has no autogen
    {0xffff, 0xffff, 0xffff, 0xffff},
    {0x03ff, 0x03ff, 0xffff, 0xffff},  // J_TYPE has 10 bits (unsigned)
    {0x0007, 0x0007, 0xffff, 0xffff},
    {0x0007, 0x0007, 0x0007, 0x0007},  // O_TYPE has 4 bits (signed), so check 3 bits
    {0xffff, 0xffff, 0xffff, 0xffff},
};

/* autogen address increase list of list */
static int addr_increase_list[14][4] = {
    { 0,  0,  0,  0},
    { 0,  0,  0,  0},
    { 0,  0,  0,  0},
    { 0,  0,  0,  0},
    {24, 24, 24, 24},  // I_TYPE autogen makes 24 more instructions
    {24, 24, 24, 24},  // one instruction cause 2 address increase
    {24, 24, 24, 24},  // so 24 increase for IJ_type
    {24, 24, 24, 24},
    { 0,  0,  0,  0},
    { 0,  0,  0,  0},
    {24, 24,  0,  0},  // J_TYPE      has 24 increase
    {30, 30,  0,  0},  // BEQ|BNE     has 30 increase
    {26, 26, 26, 26},  // LW|LB|SW|SB has 26 increase
    { 0,  0,  0,  0},
};

/* Register list. index matches with the register number. */
static char* registers[] = {
    "r0", "at", "sp", "fp", "ra", "rb", "rc", "rd", 
    "s0", "s1", "t0", "t1", "hi", "lo", "pc", "fl",
};

/* API to write to mif file */
extern FILE* get_fp();                           // defined in parser.c
extern int   get_address();                      // defined in parser.c
extern void  update_address(int);                // defined in parser.c
extern int   is_label_used();                    // defined in parser.c
extern struct list* get_label_list();            // defined in parser.c
extern struct list* get_used_list();             // defined in parser.c

extern void  write_mif(int, int, char*, FILE*);  // defined in parser.c
extern void  mif_asm_gen_message(int, int);      // defined in parser.c
extern void  mif_blank_message();                // defined in parser.c


/* mif related encoding helper */
int gen_IJ_to_R(int num, int instruction);
int gen_O_to_R(int opfunc, int rd, int rs, int num);
int gen_instr_R(int opfunc, int rd, int rs);
int gen_instr_I(int opfunc, int rd, int imm);
int gen_instr_J(int opfunc, int target);
int gen_instr_O(int opfunc, int rd, int rs, int offs);

/* Data conversion and Helper functions */
int   is_autogen(int num, int opfunc, int address);  // to be called from linkedlist.c
void  slice_cpy(char*, char*, int, int);
int   reg_to_int(char*);
int   word_to_int(char*);
int   opstr_to_opfunc(char*);


/**
* Instruction Encoding Functions 
*/

/* just do nothing and return 0 */
/* NOTE: value 0x0000 is ADD $r0, $r0 but it's the same as do nothing */
int encode_blank(char* str, regmatch_t* matched)
{
    return 0;
}

/* encode and return instruction for R_TYPE with only one register */
int encode_R1(char* str, regmatch_t* matched)
{
    char op[CODELEN], rd[CODELEN];
    slice_cpy(str, op, matched[1].rm_so, matched[1].rm_eo);
    slice_cpy(str, rd, matched[2].rm_so, matched[2].rm_eo);
    return opstr_to_opfunc(op)<<10 | reg_to_int(rd)<<6;
}

/* encode and return instruction for R_TYPE with two registers */
int encode_R2(char* str, regmatch_t* matched)
{
    char op[CODELEN], rd[CODELEN], rs[CODELEN];
    slice_cpy(str, op, matched[1].rm_so, matched[1].rm_eo);
    slice_cpy(str, rd, matched[2].rm_so, matched[2].rm_eo);
    slice_cpy(str, rs, matched[3].rm_so, matched[3].rm_eo);
    return opstr_to_opfunc(op)<<10 | reg_to_int(rd)<<6 | reg_to_int(rs)<<2;
}


/* encode and return instruction for I_PAT  */
int encode_I(char* str, regmatch_t* matched)
{
    char op[CODELEN], rd[CODELEN], imm[NUMSTR];
    slice_cpy(str, op,  matched[1].rm_so, matched[1].rm_eo);
    slice_cpy(str, rd,  matched[2].rm_so, matched[2].rm_eo);
    slice_cpy(str, imm, matched[3].rm_so, matched[3].rm_eo);

    int num = strop(imm);  // this marks is_used_label when found

    // build used_list on 1st path
    if ((get_fp() == NULL) && (is_label_used())) {
        build_used_list(get_used_list(), imm, op);  // ex. imm = "tag", op = "ANDI" 
    }

    if (!is_autogen(num, opstr_to_opfunc(op), get_address())) {
        int numlast = abs(num);
        if (num < 0)
            numlast = (numlast ^ 0x003f) + 1; // 6-bit version negation
        return opstr_to_opfunc(op)<<10 | reg_to_int(rd)<<6 | numlast;
    }
    // handling too big abs(imm) > 5 bits
    mif_asm_gen_message(5, num);
    int opfunc = opstr_to_opfunc(op) ^ 0b010000;  // ex. convert ADDI to ADD
    int instruction = gen_instr_R(opfunc, reg_to_int(rd), AT); // R2 type
    int rv = gen_IJ_to_R(num, instruction);
    mif_blank_message();
    return rv;
}


/* encode and return instruction for J_PAT  */
int encode_J(char* str, regmatch_t* matched)
{
    char op[CODELEN], target[NUMSTR];
    slice_cpy(str, op, matched[1].rm_so, matched[1].rm_eo);
    slice_cpy(str, target, matched[2].rm_so, matched[2].rm_eo);

    uint16_t num = strop(target);  // this marks is_used_label when found

    // build used_list on 1st path
    if ((get_fp() == NULL) && (is_label_used())) {
        build_used_list(get_used_list(), target, op);  // ex. target = "tag", op = "ANDI" 
    }
    if (!is_autogen(num, opstr_to_opfunc(op), get_address())) {
        return (opstr_to_opfunc(op)<<10) | (num);
        // return (opstr_to_opfunc(op)<<10) | (num & 0x03ff);
    }
    // handling too big target and num is 10 bits farther away from the next address
    mif_asm_gen_message(10, num);
    int opfunc = -1;
    if (strcmp(op, "J") == 0) opfunc = JR;         // R1 type
    if (strcmp(op, "JAL") == 0) opfunc = JALR;     // R2 type
    int instruction = gen_instr_R(opfunc, AT, RA); // R1 or R2 type. AT holds big num.
    int rv = gen_IJ_to_R(num, instruction);
    mif_blank_message();
    return rv;
}



/* generate instructions to convert I,J_TYPE to R2_TYPE if num is too big */
int gen_IJ_to_R(int num, int instruction)
{
    FILE* fp = get_fp();
    int address = get_address();

    // break down num into 6-4-6 bits and store to register at
    int a = (num & 0xFC00) >> 10;
    int b = (num & 0x03C0) >> 6;
    int c = (num & 0x003F);

    int inst[DEPTH] = {0,};
    inst[0]  = gen_instr_R(AND,  T1, R0);
    inst[1]  = gen_instr_R(AND,  AT, R0);
    inst[2]  = gen_instr_I(ORI,  T1, a);
    inst[3]  = gen_instr_I(SLLI, T1, 10);
    inst[4]  = gen_instr_R(OR,   AT, T1);
    inst[5]  = gen_instr_R(AND,  T1, R0);
    inst[6]  = gen_instr_I(ORI,  T1, b);
    inst[7]  = gen_instr_I(SLLI, T1, 6);
    inst[8]  = gen_instr_R(OR,   AT, T1);
    inst[9]  = gen_instr_R(AND,  T1, R0);
    inst[10] = gen_instr_I(ORI,  T1, c);
    inst[11] = gen_instr_R(OR,   AT, T1);  // $at is num

    inst[12] = instruction;  // append the R_TYPE instruction     

    int i = 0;
    for (; inst[i] != 0; i++)
    {
        if (fp) write_mif(address, inst[i], concat2("asm", decode(inst[i])), fp);
        address += 2;
    }
    update_address(address);
    return 0;
}


/* encode and return instruction for O_PAT  */
int encode_O(char* str, regmatch_t* matched)
{
    char op[CODELEN], rd[CODELEN], rs[CODELEN], offs[NUMSTR];
    slice_cpy(str, op, matched[1].rm_so, matched[1].rm_eo);
    slice_cpy(str, rd, matched[2].rm_so, matched[2].rm_eo);
    slice_cpy(str, rs, matched[3].rm_so, matched[3].rm_eo);
    slice_cpy(str, offs, matched[4].rm_so, matched[4].rm_eo);

    int num = strop(offs);   // this marks is_used_label when found

    // build used_list on 1st path
    if ((get_fp() == NULL) && (is_label_used())) {
        build_used_list(get_used_list(), offs, op);  // ex. offs = "tag", op = "ANDI" 
    }

    if (!is_autogen(num, opstr_to_opfunc(op), get_address())) {
        int numlast = abs(num);
        if (num < 0)
            numlast = (numlast ^ 0x000f) + 1; // 4-bit version negation
        return opstr_to_opfunc(op)<<10 | reg_to_int(rd)<<7 | reg_to_int(rs)<<4 | numlast;
    }
    // handling too big abs(target) > 3 bits
    mif_asm_gen_message(3, num);
    int rv = gen_O_to_R(opstr_to_opfunc(op), reg_to_int(rd), reg_to_int(rs), num);
    mif_blank_message();
    return rv;
}


/* generate instructions to convert O_TYPE to R2_TYPE if num is too big */
int gen_O_to_R(int opfunc, int rd, int rs, int num)
{
    FILE* fp = get_fp();
    int address = get_address();

    // break down num into 6-4-6 bits and store to register at
    int a = (num & 0xFC00) >> 10;
    int b = (num & 0x03C0) >> 6;
    int c = (num & 0x003F);

    int inst[DEPTH] = {0,};
    inst[0]  = gen_instr_R(AND , T1, R0);
    inst[1]  = gen_instr_R(AND , AT, R0);
    inst[2]  = gen_instr_I(ORI , T1, a);
    inst[3]  = gen_instr_I(SLLI, T1, 10);
    inst[4]  = gen_instr_R(OR  , AT, T1);
    inst[5]  = gen_instr_R(AND , T1, R0);
    inst[6]  = gen_instr_I(ORI , T1, b);
    inst[7]  = gen_instr_I(SLLI, T1, 6);
    inst[8]  = gen_instr_R(OR  , AT, T1);
    inst[9]  = gen_instr_R(AND , T1, R0);
    inst[10] = gen_instr_I(ORI , T1, c);
    inst[11] = gen_instr_R(OR  , AT, T1);  // $at is num

    // test equality of rd and rs then do the BEQ or BNE
    if (opfunc == BEQ || opfunc == BNE) {
        inst[12] = gen_instr_R(SUB , rd, rs);
        inst[13] = gen_instr_J(J   , address + 15); // go to inst[15]
        inst[14] = gen_instr_R(JR  , AT, R0);           // R0 is used as don't care
        inst[15] = gen_instr_O(opfunc, rd, R0, 0b1111); // go to inst[14] if EQ or NE
    }
    else { // LW, LB, SW, SB
        inst[12] = gen_instr_R(ADD , AT, rs);           // $at = $rs + $at (num)
        inst[13] = gen_instr_O(opfunc, rd, AT, 0);
    }

    int i = 0;
    for (; inst[i] != 0; i++)
    {
        if (fp) write_mif(address, inst[i], concat2("asm", decode(inst[i])), fp);
        address += 2;
    }
    update_address(address);
    return 0;
}


/* generate instruction for Type_R */
int gen_instr_R(int opfunc, int rd, int rs)
{
    return opfunc<<10 | rd<<6 | rs<<2;
}

/* generate instruction for Type_I */
int gen_instr_I(int opfunc, int rd, int imm)
{
    return opfunc<<10 | rd<<6 | imm;
}

/* generate instruction for Type_J */
int gen_instr_J(int opfunc, int target)
{
    return opfunc<<10 | target;
}

/* generate instruction for Type_O */
int gen_instr_O(int opfunc, int rd, int rs, int offs)
{
    return opfunc<<10 | rd<<7 | rs<<4 | offs;
}




/**
* Data Conversion Helper Functions 
*/

/* helper to slice and copy string */
void slice_cpy(char* src, char* dst, int start, int end)
{
    int i, j;
    for (i = start, j = 0; i < end; i++, j++) {    
        if (isspace(src[i]))  // sometimes src includes trailing spaces, so remove them
            break;
        dst[j] = src[i];
    }
    dst[j] = '\0';
}

/* helper to convert register str to int */
int reg_to_int(char* str)
{
    int i;                                 
    for (i = 0; i < REGNUM; i++) {
        if (strcmp(registers[i], str) == 0)
            return i;
    }
    oops("reg_to_int: unexpected register")
}


/* helper to convert instruction to int */
int word_to_int(char* str)
{
    char* p = str;
    int rv = *p - '0';
    for (p++; *p != '\0'; p++) {
        rv = (rv << 4) + (*p - '0');
    }
    return rv;
}

/* helper to get autogen boundary */
int opfunc_to_boundary(int num)
{
    int op = (num & 0x3c) >> 2;
    int func = num & 0x3;
    return autogen_boundary_list[op][func];    
}

/* Helper to get autogen address increase */
int opfunc_to_increase(int num)
{
    int op = (num & 0x3c) >> 2;
    int func = num & 0x3;
    return addr_increase_list[op][func];    
}

/* Check if autogen needed */
/*
    num     - argument that might be too big and trigger autogen
    opfunc  - opcode and function code
    address - current address
 */
int is_autogen(int num, int opfunc, int address)
{
    int boundary = opfunc_to_boundary(opfunc);

    // if ((opfunc == J) || (opfunc == JAL)) {
    //     uint16_t addr_part = (address + 2) & 0xFC00;
    //     uint16_t num_part  = num & 0xFC00;
    //     if (addr_part != num_part)
    //         return 1;            
    //     else
    //         return 0;
    // }

    if (abs(num) > boundary)
        return 1;  // YES
    return 0;      // NO
}

