/*
 * parser.c
 * 
 * Usage: ./parser filename
 *    
 * `make run` to run this program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "regexp.h"
#include <math.h>
#include "linkedlist.h"
#include "directives.h"
#include "strfunc.h"
#include "common.h"

/* file scope variables */
static FILE*        fp_w = NULL;     /* file pointer for writing */
static int          address = 0;     /* location counter */
static int          is_used_label;   /* FSM to check used label is found */
static struct list* label_list_ptr;  /* list of labels */
static struct list* used_list_ptr;   /* list of used labels */
static char*        linestr;         /* line string with line number */

/* API functions*/
FILE*        get_fp()         { return fp_w; }
int          get_address()    { return address; }
void         update_address(int new_address) { address = new_address; }
struct list* get_label_list() { return label_list_ptr; }
struct list* get_used_list()  { return used_list_ptr; }
void         used_label_found() { is_used_label = 1; }
int          is_label_used()  { return is_used_label; }

/* mif related function declarations */
char*        mif_header();
char*        mifname(char*);
char*        gen_mifstr(int, char*);
void         write_mif(int, int, char*, FILE*);
void         mif_asm_gen_message(int, int);    
void         mif_blank_message();
void         mif_label_message(char*, int);          


/*
  name:    build_labels
  purpose: read one line to encode, then writes to mif file
  field:   line     - one line of assembly code
           llp      - pointer to label_list

  program flow:
      1) counts number of label used, and build a label_list
      2) check if it's directive (directives.c), and call the handler function
      3) check syntax pattern (regexp.c), and call the encode function
      4) encoded instruction is returned but don't write to mif file
 */
void build_labels(char* line, struct list* llp)
{
    is_used_label = 0;          // reset every line read
    int label_address = handle_label(line, address);  

    if (label_address != -1) {  // -1 means not label
        build_label_list(llp, used_list_ptr, label_address, getlabel(line));
    }
    int new_addr = check_directive(line, address, NULL);
    if (new_addr != -1) {
        address = new_addr;     // update address
    }
    int instr = check_pattern(line);   
    if (instr > 0) {            // 0 means blank line, > 0 means instruction
        address += 2;           // 1 instruction word is 2 byte
    }
}

/*
  name:    endoce_line
  purpose: read one line to encode, then writes to mif file
  field:   line     - one line of assembly code
           elp      - pointer to error_list
           fp_write - file pointer to write
           lines    - assembly code line number

  flow:
      1) check if directive (directives.c), and call the handler function
      2) check syntax pattern (regexp.c), and call the encode function
      3) encoded instruction is returned and writes to mif file
      4) no label, no directive, no match pattern means wrong format
 */
void encode_line(char* line, struct list* elp, FILE* fp_write, int lines)
{
    linestr = concat2(int_to_str(lines), line);
    int new_addr = check_directive(line, address, fp_write);
    if (new_addr != -1) {    
        address = new_addr;     // update address
    }    
    int instr = check_pattern(line);
    if (instr > 0) {            // 0 means blank line, > 0 means instruction
        write_mif(address, instr, linestr, fp_write);
        address += 2;           // 1 instruction word
    }
    if (!is_label(line) && new_addr == -1 && instr < 0) {
        build_error_list(elp, concat2("Wrong Format", trimmed(line, '\n')), lines);
    }

    // DEBUG marking on mif file
    char* label;
    if (is_label(line) && new_addr == -1 && instr < 0 &&
        (label = getstr(label_list_ptr, address)) != NULL) {
        mif_label_message(label, address);
    }
}

/* helper to report assemble result */
void asm_report(int lines, struct list* llp, struct list* elp, struct list* ulp)
{
    printf("%-4s\t: %d\n\n", "LINES READ", lines);

    // labels report 
    if (llp->next != NULL) {
        printf("[-- LABEL LIST REPORT --]\n");
        dumplist(llp, "address", 'h');  // hex
        freelist(llp);
    }
    // error report 
    if (elp->next != NULL) {
        printf("[-- ERROR LIST REPORT --]\n");
        dumplist(elp, "line", 'd');    // decimal
        freelist(elp);
    }    
    freelist(ulp);
}

/* 
  name:    assemble
  purpose: read line-by-line two times to assemble mif file 
  field:   filename    - file to read in the same directory

  flow:
    1st path:  build_labels - add label_list if label found
    2nd path:  encode_line  - add error_list if format error found
    reports:   label list and error list    
*/
void assemble(char* filename)
{
    // init lists
    struct list label_list;
    struct list used_list;
    struct list error_list;
    init_list(&label_list, NULL);    // sorted, overwrite on duplication
    init_list(&used_list,  NULL);    // no sorted, keep everything
    init_list(&error_list, NULL);    // no sorted, keep everything
    label_list_ptr = &label_list;    // register to static address
    used_list_ptr = &used_list;      // register to static address

    // init streams
    FILE* fp_read = fopen(filename, "r");
    FILE* fp_write = fopen(mifname(filename), "w");
    if (!fp_read || !fp_write) oops("fopen failed..")

    // init for getline
    char* line = NULL;
    size_t len = 0;
    int lines = 0;

    // 1st path to generate simbol table
    while (getline(&line, &len, fp_read) != -1) {
        build_labels(strip(line), &label_list);
    }

    // prepare for 2nd path
    rewind(fp_read);
    address = 0;       // location counter reset 
    fp_w = fp_write;   // static fp set. fp_w is null while 1st path
    addr_resolution(label_list_ptr, used_list_ptr);

    // 2nd path to encode and make error list
    fprintf(fp_write, "%s\n", mif_header());
    while (getline(&line, &len, fp_read) != -1) {
        lines += 1;

        // DEBUG marking on mif file
        if (strstr(line, "DEBUG")) {
            fprintf(fp_write, "%s%s\n", "\n\t-- ", line);
            continue;
        }
        encode_line(strip(line), &error_list, fp_write, lines);
    }
    fprintf(fp_write, "%s\n", "END;");

    asm_report(lines, &label_list, &error_list, &used_list);
    free(line);
    fclose(fp_read);        
    fclose(fp_write);        
    regfree_all();    // regexp.c
}



/**
 * Memory Initialization File related functions
 * http://sites.fas.harvard.edu/~cscie287/fall2017/def_mif.htm
 */

/* return mif header */
char* mif_header()
{
    char* header = "DEPTH = 32768;\nWIDTH = 16;\n\
ADDRESS_RADIX = HEX;\nDATA_RADIX = HEX;\nCONTENT\nBEGIN";
    return header;
}

/* generate mif filename */
char* mifname(char* filename)
{
    static char mifname[STRLEN];
    char* p = strrchr(filename, '.');
    if(p) *p = '\0';
    sprintf(mifname, "%s.%s", filename, "mif");
    return mifname;
}

/* helper to generate string for mif file from instruction value and original string */
char* gen_mifstr(int value, char* str)
{
    static char mifstr[STRLEN] = {0,};
    sprintf(mifstr, "%04x;    --   [%s] -> [%s]", value, int_to_bin(value), trimmed(str, '\n'));
    return mifstr;
}

/* API to write mif string */
void write_mif(int address, int value, char* str, FILE* fp)
{
    char* instruction = gen_mifstr(value, str); 
    fprintf(fp, "\t%04x : %s\n", address/2, instruction); // convert byte address to mif word address
    if (DEBUG) printf("\t%04x : %s\n", address/2, instruction);
}

/* API to write assembler message for automatic generation */
void mif_asm_gen_message(int bitlen, int num)
{
    if (fp_w) fprintf(fp_w, "%20s--   auto-gen (0x%x > %dbits) <- [%s]\n", "", num, bitlen, trimmed(linestr, '\n'));
}

/* API to write assembler message for label */
void mif_label_message(char* label, int addr)
{
    if (fp_w) fprintf(fp_w, "%20s--   %s: %04x <- [%s]\n", "", "label", addr/2, trimmed(linestr, '\n'));
    // if (fp_w) fprintf(fp_w, "%20s--   %s: %04x <- [%s]\n", "", label, addr/2, trimmed(linestr, '\n'));
}

/* API to write blank line */
void mif_blank_message()
{
    if (fp_w) fprintf(fp_w, "\n");
}


/**
* Start function
*/
void parser(int ac, char* av[])
{
    ps("-- parser.c --")
    if (ac < 2) oops("Usage: ./parser filename.asm\t")

    assemble(av[1]);
}

/* main controler */
int main(int ac, char* av[])
{
    /* run core function */
    parser(ac, av);

    // strfunc_test();     // DEBUG
    // regexp_test();      // DEBUG
    // linkedlist_test();  // DEBUG
    return 0;
}

