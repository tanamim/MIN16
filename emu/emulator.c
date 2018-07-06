/*
 * emulator.c
 * 
 * Usage: ./emulator filename
 *    
 * `make run` to run this program
 *
 * set SIMU to 0 in common.h to turn off simulation mode, to run simple mode
 * set SIMU to 1 in common.h to turn on display register mode
 * set SIMU to 2 in common.h to turn on line-by-line execution mode
 * 
 * NOTE: For simulation, string output is char-by-char in green color.
 *       The program stall until return key is hit to display next character.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "decoder.h"
#include "strfunc.h"
#include "executor.h"

#define DEPTH 32768
#define WIDTH 16
#define BYTE  8
#define MEMSIZE DEPTH * WIDTH / BYTE
#define WORDSIZE 256

#define REG_IOCONTOL   0xff00
#define REG_IOBUFFER_1 0xff04
#define BIT_SERIAL_INPUTREADY  0b01
#define BIT_SERIAL_OUTPUTREADY 0b10
#define BIT_SERIAL_INPUTFLUSH  0b01
#define BIT_SERIAL_OUTPUTFLUSH 0b10

#define INT(x) hexchar_to_num(x)

/* Static variable */
static char     strbuf[STRLEN];
static char*    strptr = strbuf;
static uint8_t  mem[MEMSIZE];    // Memory
static uint16_t instruction_reg; // Instruction Register
static uint16_t last_mif_addr;   // last mif word address

uint8_t* get_mem() {return mem;}

/**
* Helper Functions
*/
/* convert hexchar to num */
int hexchar_to_num(char c)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	if ('a' <= c && c <= 'z')
		return c - 'a' + 10;
	if ('A' <= c && c <= 'Z')
		return c - 'A' + 10;
	char str[2];
	str[0] = c;
	str[1] = '\0'; 
	oops2(str, "not a hex character")
}

/* convert hexstr to num */
int hexstr_to_num(char* hexstr)
{
	char* s = hexstr;
	int num = 0;
	while(*s)
		num = num * 16 + hexchar_to_num(*s++);
	return num;
}

/**
* Memory Manipulate API to be used by executor.c
*/
/* get memory contents at byte address */
uint8_t load_byte(uint16_t byte_addr)
{
	if (byte_addr == REG_IOBUFFER_1) {
		if (strptr == strbuf) {
			if (SIMU)
				printf("Input number (signed 16bit): ");
			fgets(strbuf, STRLEN, stdin);
		}
		return (uint8_t) *strptr++;
	}
	return mem[byte_addr];
}
/* get memory contents at word address in big endian */
uint16_t load_word(uint16_t byte_addr)
{
	return mem[byte_addr + 1] * WORDSIZE | mem[byte_addr];
}

/* set memory contents at designated byte address */
void store_byte(uint16_t byte_addr, uint16_t word)
{	
	if (byte_addr == REG_IOBUFFER_1) {
		if (SIMU) {
			printf("[stdout] word [%04x] at address [%04x] is char [%s%c%s]\n", word, byte_addr, GRN1, word & 0x00ff, RESET);
			getchar();
		}
		else
			printf("%s%c%s", GRN1, word & 0x00ff, RESET);
	}
	else if (byte_addr == REG_IOCONTOL) {
		memset(strbuf, 0, STRLEN);
		strptr = strbuf;
	}
	mem[byte_addr] = word & 0x00ff;  // only mask byte
}

/* set memory contents at designated byte address */
void store_word(uint16_t byte_addr, uint16_t word)
{	
	mem[byte_addr] = word & 0x00ff;
	mem[byte_addr+1] = word >> 8;
}

/**
* Memory Manipulate Functions
*/
/* store bytes from mif string in little endian */
void mif_to_memory(char* str, int byte_addr)
{
	uint8_t byte0 = INT(str[2]) * 16 + INT(str[3]);
	uint8_t byte1 = INT(str[0]) * 16 + INT(str[1]);
	mem[byte_addr] = byte0;
	mem[byte_addr+1] = byte1;
}

/* get address and instruction from mif line and store */
void process_memory(char* line)
{
	const char* delim = ":;";
	char* sp = strsep(&line, delim);
	uint16_t word_addr = hexstr_to_num(remove_space(sp));
	sp = strsep(&line, delim);
	mif_to_memory(remove_space(sp), word_addr * 2);  // byte_addr = word_addr * 2
}

/* process mif line */
void process_line(char* line)
{
	line = remove_space(line);

	static int is_begin = 0; // FSM
	if (is_begin == 0 && strstr(line, "BEGIN"))
		is_begin = 1;
	else if (is_begin == 1 && strstr(line, "END;"))
		is_begin = 0;
	else if (is_begin == 1 && strlen(line) == 0)
		return;
	else if (is_begin == 1 && strstr(line, "--") == line)
		return;
	else if (is_begin == 1) {
		process_memory(line);
		last_mif_addr = hexstr_to_num(strsep(&line, ":"));
	}
}

/* emulate */
void emulate()
{
	uint16_t pc;
	for(;;) {  // increment by word
		pc = get_pc();
		instruction_reg = load_word(pc);
		if (pc >= last_mif_addr * 2 + 1) break;
		execute(instruction_reg);
	}
}

/* build memory */
void build_memory(char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (!fp) oops ("fopen failed..")

    // init for getline
    char*  line = NULL;
    size_t len = 0;
    int    lines = 0;

    // 1st path to generate simbol table
    while (getline(&line, &len, fp) != -1) {
    	process_line(line);
    	lines++;
    }

    // set I/O memory input/output ready state
    mem[REG_IOCONTOL] = BIT_SERIAL_INPUTREADY | BIT_SERIAL_OUTPUTREADY;

    if (SIMU)
	    printf("LINES READ : %d\n", lines);
}

/* TEST */
void test_show_memory()
{
	uint16_t memory = 0xe01e;    // CHAR_mem label
	int size = 16;

    int i;
    for (i=memory; i < memory + size; i++)
	    printf("mem[%04x] is [%04x]\n", i, mem[i]);	
}

/**
* Start Core Function
*/
void emulator(int ac, char* av[])
{
    ps("-- emulator.c --")
    if (ac < 2) oops("Usage: ./emulator filename.mif\t")

    system("clear");

    build_memory(av[1]);
	emulate();
}

/* main controler */
int main(int ac, char* av[])
{
    /* run core function */
    emulator(ac, av);

    /* test function here */
    // test_show_memory();

    return 0;
}
