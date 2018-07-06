/*
 * executor.c -- a set of execute functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "decoder.h"
#include "strfunc.h"
#include "executor.h"

#define REGSIZE  16
#define FLAGSZ   5
#define ARGS(x) (get_args_from_instr(x))

#define ZF 0b00001  // Zero Flag
#define SF 0b00010  // Zero Flag
#define CF 0b00100  // Zero Flag
#define OF 0b01000  // Zero Flag
#define EQ 0b10000  // Zero Flag

/* Static variable */
static uint16_t regs[REGSIZE];   // Register Array
static uint16_t program_counter; // corresponds to memory address

/* Defined in emulator.c */
extern uint8_t  load_byte(int);
extern uint16_t load_word(int);
extern void     store_byte(int, uint16_t);
extern void     store_word(int, uint16_t);
extern uint8_t* get_mem();

/* Registers */
enum reg {
    R0, AT, SP, FP, RA, RB, RC, RD, S0, S1, T0, T1, HI, LO, PC, FL,
};

/* Register list. index matches with the register number. */
static char* regs_str[] = {
    "r0", "at", "sp", "fp", "ra", "rb", "rc", "rd", 
    "s0", "s1", "t0", "t1", "hi", "lo", "pc", "fl",
};

/* Status Flag list. */
static char* flags_str[] = {
    "ZF", "SF", "CF", "OF", "EQ",
};

/* Execute Function Declarations */
void ADD  () ; void SUB  () ; void MUL  () ; void SLT  () ;
void ADDU () ; void SUBU () ; void MULU () ; void SLTU () ;
void AND  () ; void OR   () ; void XOR  () ; void NOR  () ; 
void SLL  () ; void SRL  () ; void SRA  () ; void ROTL () ; 
void ADDI () ; void SUBI () ; void MULI () ; void SLTI () ;
void ADDIU() ; void SUBIU() ; void MULIU() ; void SLTIU() ;
void ANDI () ; void ORI  () ; void XORI () ; void NORI () ;
void SLLI () ; void SRLI () ; void SRAI () ; void ROTLI() ;
void J    () ; void JAL  () ; void JR   () ; void JALR () ;
void BEQ  () ; void BNE  () ; void RSVD () ;
void LW   () ; void LB   () ; void SW   () ; void SB   () ;
void MFHI () ; void MFLO () ; void MTHI () ; void MTLO () ; 

/* Execute Function Pointer Array */
static void (*func_list[14][4])() = {
    {ADD  , SUB  , MUL  , SLT  },
    {ADDU , SUBU , MULU , SLTU },
    {AND  , OR   , XOR  , NOR  }, 
    {SLL  , SRL  , SRA  , ROTL },
    {ADDI , SUBI , MULI , SLTI },
    {ADDIU, SUBIU, MULIU, SLTIU},
    {ANDI , ORI  , XORI , NORI },
    {SLLI , SRLI , SRAI , ROTLI},
    {RSVD , RSVD , RSVD , RSVD },
    {RSVD , RSVD , RSVD , RSVD },
    {J    , JAL  , JR   , JALR },
    {BEQ  , BNE  , RSVD , RSVD },
    {LW   , LB   , SW   , SB   },
    {MFHI , MFLO , MTHI , MTLO },
};


/* Funceion declarations */
int16_t signed_R_rd(int num);


/* Zero and Sign Flag register checker. */
void check_flag_zf_sf(int instr) {	
	if ((instr & 0x8000) != 0) return;

	uint16_t result = signed_R_rd(instr);
	if (result == 0)	
		regs[FL] |= ZF;
	if (result & 0x8000)
		regs[FL] |= SF;
}

/* Choose function and execute */
void exec_func(int instr) {
	func_list[op_row(instr)][func_col(instr)](instr);
	check_flag_zf_sf(instr);
}

/* Helper to mask off opfunc */
int get_args_from_instr(uint16_t instr)
{
	return instr & 0x3ff;
}

/* API for Program Counter used in emulator.c */
uint16_t get_pc()  {return program_counter;}


/* Execute instruction */
void execute(uint16_t instr)
{
	// executor_test(instr);  // DEBUG

	uint16_t tmp_counter = program_counter;

	if (SIMU)
		display_info_with_execution(instr);     
	else
		exec_func(instr);   // execute without display simulation info

	if (tmp_counter == program_counter)
		program_counter += 2;

	regs[FL] = 0;          // reset flag register

	if (SIMU > 1)
		getchar();         // see line-by-line execution
}

void display_info_with_execution(int instr)
{
	printf("[%04x:%04x] (%04x) %s\n", program_counter/2, instr, program_counter ,decode(instr));

	uint16_t reg_prev[REGSIZE];  // Register Array COPY
	int i;
	for (i = 0; i < REGSIZE; i++)
		reg_prev[i] = regs[i];
	regs[PC] = program_counter;

	exec_func(instr);           // execute

	for (i = 0; i < REGSIZE; i++) {
		if (regs[i] == reg_prev[i])
			printf("%s %s %s ", YELO0, regs_str[i], RESET);			
		else
			printf("%s %s %s ", MGNT0, regs_str[i], RESET);			
	}
	printf("\n");

	for (i = 0; i < REGSIZE; i++) {
		if (regs[i] == reg_prev[i])
			printf("%s%04x%s ", YELO1, regs[i], RESET);			
		else
			printf("%s%04x%s ", MGNT1, regs[i], RESET);			
	}
	for (i = 0; i < FLAGSZ; i++) {
		if ((regs[FL] & (1 << i)) != 0)
			printf(" %s%s%s", YELO1, flags_str[i], RESET);			
		else
			printf("   ");			
	}
	printf("\n\n");
}


/* TESTING */
void executor_test(int instr)
{
	ps(decode(instr))
	program_counter = 0;
	// int num = ARGS(instr);
	// regs[R_rd(num)] = 0;
	store_word(0x1000, 0x1234);
	store_word(0x1002, 0x5678);
	regs[AT] = 0x1004;
	regs[RA] = 0xabcd;
	exec_func(instr);  // execute
	// ps(int_to_bin(regs[R_rd(num)]))
	// pd(signed_R_rd(instr))

	// printf("RA is [%x] PC is [%x]\n", regs[RA], program_counter);

	px(regs[AT])
	px(regs[RA])
	px(regs[RB])
	// px(load_word(0x1000))
	// px(get_mem()[0x1000])
	// pd(program_counter);
	printf("\n");
}

/**
* Register Manipulate Functions
*/
/* Helper to get the index of the first TYPE_R argument (rd) */
int R_rd(int num)
{
	return (num & 0b0000001111000000)>>6; 
}

/* Helper to get the index of the second TYPE_R argument (rs) */
int R_rs(int num)
{
	return (num & 0b0000000000111100)>>2; 
}

/* Helper to get the unsigned value of first TYPE_R argument ($rd) */
uint16_t unsigned_R_rd(int num)
{
	// int index = (num & 0b0000001111000000)>>6; 
	return (uint16_t) regs[R_rd(num)];
}

/* Helper to get the signed value of the first TYPE_R argument ($rd) */
int16_t signed_R_rd(int num)
{
	// int index = (num & 0b0000001111000000)>>6; 
	return (int16_t) regs[R_rd(num)];
}

/* Helper to get the unsigned value of the second TYPE_R argument ($rs) */
uint16_t unsigned_R_rs(int num)
{
	// int index = (num & 0b0000000000111100)>>2; 
	return (uint16_t) regs[R_rs(num)];
}

/* Helper to get the signed value of the second TYPE_R argument ($rs) */
int16_t signed_R_rs(int num)
{
	// int index = (num & 0b0000000000111100)>>2; 
	return (int16_t) regs[R_rs(num)];
}

/* Helper to get the unsigned immediate TYPE_I argument (imm) */
uint16_t unsigned_R_imm(int num)
{
	return num & 0b0000000000111111; 
}

/* Helper to get the signed immediate TYPE_I argument (imm) */
int16_t signed_R_imm(int num)
{
	int sign = (num & 0b100000)>>5;
	int rv   = (num & 0b011111);
	if (sign)
		rv = -(((num & 0x3f) ^ 0x3f) + 1);
	return rv; 
}

/* Helper to get the unsigned target TYPE_J argument (target) */
uint16_t unsigned_J_target(int num)
{
	return num & 0b0000001111111111;  // 0x03ff
}

/* Helper to get the unsigned value of the first TYPE_O argument ($rd) */
uint16_t unsigned_O_rd(int num)
{
	int index = (num & 0b0000001110000000)>>7; 
	return (uint16_t) regs[index];	
}


/* Helper to get the index of the first TYPE_O argument (rd) */
int O_rd(int num)
{
	return (num & 0b0000001110000000)>>7; 
}

/* Helper to get the index of the second TYPE_O argument (rs) */
int O_rs(int num)
{
	return (num & 0b0000000001110000)>>4; 
}

/* Helper to get the signed value of the second TYPE_O argument ($rs) */
int16_t signed_O_rd(int num)
{
	return (int16_t) regs[O_rd(num)];
}

/* Helper to get the signed value of the second TYPE_O argument ($rs) */
int16_t signed_O_rs(int num)
{
	return (int16_t) regs[O_rs(num)];
}

/* Helper to get the unsigned value of the second TYPE_O argument ($rs) */
uint16_t unsigned_O_rs(int num)
{
	return (uint16_t) regs[O_rs(num)];
}

/* Helper to get the signed offset TYPE_O argument (offset) */
int16_t signed_O_offset(int num)
{
	int sign = (num & 0b1000)>>3;
	int rv = (num & 0b0111);
	if (sign)
		rv = - (((num & 0x7) ^ 0x7) + 1);
	return rv; 
}


/**
* Instruction Functions
*/
/* Carry Flag works only for unsigned number operation */
void check_flag_cf(int num) 
{
	uint16_t rd;
	uint16_t rs;
	uint16_t ans;
	void (*func)() = func_list[op_row(num)][func_col(num)];
	if (func == ADDU || func == SUBU){
		rd = unsigned_R_rd(num);
		rs = unsigned_R_rs(num);		
	}
	else if (func == ADDIU || func == SUBIU) {
		rd = unsigned_R_imm(num);
		rs = unsigned_R_imm(num);		
	}
	else oops2("check_flag_cf", "unexpected func")

	if (func == ADDIU || func == ADDU)
		ans = rd + rs;
	else
		ans = rd - rs;

	if (ans < rd) 
		regs[FL] |= CF;    // Carry Flag is only for unsigned number
}


/* Overflow Flag works only for signed number operation */
void check_flag_of(int num) 
{
	int16_t rd;
	int16_t rs;
	int16_t ans;
	void (*func)() = func_list[op_row(num)][func_col(num)];
	if (func == ADD || func == SUB){
		rd = signed_R_rd(num);
		rs = signed_R_rs(num);		
	}
	else if (func == ADDI || func == SUBI) {
		rd = signed_R_imm(num);
		rs = signed_R_imm(num);		
	}
	else oops2("check_flag_of", "unexpected func")

	if (func == ADD || func == ADDI)
		ans = rd + rs;
	else if (func == SUB || func == SUBI)
		ans = rd - rs;
	else oops2("check_flag_of", "unexpected func")

	int16_t msb_rd  = (rd & 0x8000) >> 15;
	int16_t msb_rs  = (rs & 0x8000) >> 15;
	int16_t msb_ans = ans >> 15;
	int is_of = msb_rd * msb_rs * !msb_ans + !msb_rd * !msb_rs * msb_ans;
	if (is_of)
		regs[FL] |= OF;	
}

void ADD  (int num) 
{	check_flag_of(num);	
	regs[R_rd(num)] = signed_R_rd(num) + signed_R_rs(num);
}
void SUB  (int num) 
{	check_flag_of(num);
	regs[R_rd(num)] = signed_R_rd(num) - signed_R_rs(num);
}
void MUL  (int num) 
{	regs[R_rd(num)] = signed_R_rd(num) * signed_R_rs(num);	
}
void SLT  (int num)
{	regs[R_rd(num)] = signed_R_rd(num) < signed_R_rs(num) ? 1 : 0;		
}

void ADDU (int num) 
{	check_flag_cf(num);
	regs[R_rd(num)] = unsigned_R_rd(num) + unsigned_R_rs(num);
}
void SUBU (int num) 
{	check_flag_cf(num);
	regs[R_rd(num)] = unsigned_R_rd(num) - unsigned_R_rs(num);
}
void MULU (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) * unsigned_R_rs(num);		
}
void SLTU (int num)
{	regs[R_rd(num)] = unsigned_R_rd(num) < unsigned_R_rs(num) ? 1 : 0;			
}

void AND  (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) & unsigned_R_rs(num);			
}
void OR   (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) | unsigned_R_rs(num);				
}
void XOR  (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) ^ unsigned_R_rs(num);					
}
void NOR  (int num) 
{	regs[R_rd(num)] = ~(unsigned_R_rd(num) | unsigned_R_rs(num));					
}

void SLL  (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) << unsigned_R_rs(num);						
}
void SRL  (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) >> unsigned_R_rs(num);
}
void SRA  (int num) 
{	regs[R_rd(num)] = signed_R_rd(num) >> signed_R_rs(num);
}
void ROTL (int num) 
{	uint16_t left  = unsigned_R_rd(num) << (unsigned_R_rs(num) % 16);
	uint16_t right = unsigned_R_rd(num) >> (16 - unsigned_R_rs(num) % 16);
	regs[R_rd(num)] = left | right;
}

void ADDI (int num) 
{	check_flag_of(num);
	regs[R_rd(num)] = signed_R_rd(num) + signed_R_imm(num);	
}
void SUBI (int num) 
{	check_flag_of(num);
	regs[R_rd(num)] = signed_R_rd(num) - signed_R_imm(num);	
}
void MULI (int num) 
{	regs[R_rd(num)] = signed_R_rd(num) * signed_R_imm(num);		
}
void SLTI (int num)
{	regs[R_rd(num)] = signed_R_rd(num) < signed_R_imm(num) ? 1 : 0;		
}

void ADDIU(int num) 
{	check_flag_cf(num);
	regs[R_rd(num)] = unsigned_R_rd(num) + unsigned_R_imm(num);	
}
void SUBIU(int num) 
{	check_flag_cf(num);
	regs[R_rd(num)] = unsigned_R_rd(num) - unsigned_R_imm(num);	
}
void MULIU(int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) * unsigned_R_imm(num);		
}
void SLTIU(int num)
{	regs[R_rd(num)] = unsigned_R_rd(num) < unsigned_R_imm(num) ? 1 : 0;		
}

void ANDI (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) & unsigned_R_imm(num);
}
void ORI  (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) | unsigned_R_imm(num);				
}
void XORI (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) ^ unsigned_R_imm(num);					
}
void NORI (int num)
{	regs[R_rd(num)] = ~(unsigned_R_rd(num) | unsigned_R_imm(num));					
}

void SLLI (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) << unsigned_R_imm(num);						
}
void SRLI (int num) 
{	regs[R_rd(num)] = unsigned_R_rd(num) >> unsigned_R_imm(num);
}
void SRAI (int num) 
{	regs[R_rd(num)] = signed_R_rd(num) >> signed_R_imm(num);
}
void ROTLI(int num)
{	uint16_t left  = unsigned_R_rd(num) << (unsigned_R_imm(num) % 16);
	uint16_t right = unsigned_R_rd(num) >> (16 - unsigned_R_imm(num) % 16);
	regs[R_rd(num)] = left | right;
}

void J    (int num) 
{	// program_counter = ((program_counter + 2) & 0xfc00) | ((uint16_t) num & 0x03ff);
	program_counter = unsigned_J_target(num);
}
void JAL  (int num) 
{	// program_counter = ((program_counter + 2) & 0xfc00) | ((uint16_t) num & 0x03ff);
	regs[RA] = program_counter + 2;  // next word
	program_counter = unsigned_J_target(num);
}
void JR   (int num) 
{	program_counter = unsigned_R_rd(num);
}
void JALR (int num)
{	regs[R_rs(num)] = program_counter;
	regs[RA] = program_counter + 2;  // next word
	program_counter = unsigned_R_rd(num);	
}

void BEQ  (int num) 
{	if (unsigned_O_rd(num) == unsigned_O_rs(num)) {
		regs[FL] |= EQ;             // mark equal flag
		program_counter += (signed_O_offset(num) << 1);
	}
}
void BNE  (int num) 
{	if (unsigned_O_rd(num) != unsigned_O_rs(num))
		program_counter += (signed_O_offset(num) << 1);
	else
		regs[FL] |= EQ;             // mark equal flag
}
void RSVD (int num)
{	ps("This is Reserved")
}

void LW   (int num) 
{	regs[O_rd(num)] = load_word(signed_O_rs(num) + (signed_O_offset(num) << 1));
}
void LB   (int num) 
{	regs[O_rd(num)] = load_byte(signed_O_rs(num) + (signed_O_offset(num) << 1));
}
void SW   (int num) 
{	store_word(signed_O_rd(num) + (signed_O_offset(num)<<1), unsigned_O_rs(num));
}
void SB   (int num)
{	store_byte(signed_O_rd(num) + (signed_O_offset(num)<<1), unsigned_O_rs(num));
}

void MFHI (int num) 
{	regs[R_rd(num)] = regs[HI];
}
void MFLO (int num) 
{	regs[R_rd(num)] = regs[LO];
}
void MTHI (int num) 
{	regs[HI] = regs[R_rd(num)];	
}
void MTLO (int num) 
{	regs[LO] = regs[R_rd(num)];		
}


