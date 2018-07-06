/*
 * linkedlist.c -- data strucure to store data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "strfunc.h"
#include "regexp.h"
#include "linkedlist.h"
#include "decoder.h"
#include "common.h"

/* Helper function declarations */
void* emalloc(size_t n);

extern int get_address();  // parser.c

/* defined in encoder.c */
extern int opfunc_to_boundary(int num);
extern int opfunc_to_increase(int num);
extern int is_autogen(int num, int opfunc, int address);


/**
* List Building Functions
*/

/* initialize and register add function */
void init_list(struct list* lp, void (*f) (struct list*, char*, int))
{
    lp->str   = NULL;
    lp->count = 0;
    lp->num   = 0;
    lp->next  = NULL;
    lp->add   = f;
}

/* error_list */
void build_error_list(struct list* ls, char* str, int num)
/*
 * purpose: take a str to add to the list
 */
{
    struct list* p = ls;
    // add to the end
    if (p->next == NULL) {
        struct list* newptr = emalloc(sizeof(struct list));
        newptr->next = NULL;
        p->next = newptr;
        p = newptr;
        p->str = emalloc(strlen(str) + 1);
        strcpy(p->str, str);
        p->num = num;
        p->count = 1;
    }
    // go to next item of the list
    else {
        build_error_list(p->next, str, num);
    }    
}


/* Build label list */
void build_label_list(struct list* llp, struct list* ulp, int num, char* str)
/*
 * purpose: take a str to add to the list to be sorted in ABC order
 *    note: recursively find a sorted place and register struct fields.
 *    struct list:
 *              char* str = label
 *              num       = address of label. it can be overwritten.
 *              base      = base address of label.
 *              count     = length of used label list before this func is called.
 *              next      = next label node
 *              add       = maybe no use
 */
{
    struct list* p = llp;

    // very beginning or at the end
    if (p->next == NULL) {
        struct list* newptr = emalloc(sizeof(struct list));
        newptr->next = NULL;
        p->next = newptr;
        p = newptr;
        p->str = emalloc(strlen(str) + 1);
        strcpy(p->str, str);
        p->num = num;
        p->base = num;
        p->count = getlength(ulp);
    }
    // the str already exists
    else if (strcmp(str, p->next->str) == 0) {
        p->next->num = num;  // overwrite
        p->next->base = num;
        p->next->count = getlength(ulp);
    }
    // the str is new
    else if (strcmp(str, p->next->str) < 0) {
        struct list* newptr = emalloc(sizeof(struct list));
        newptr->next = p->next;
        p->next = newptr;
        p = newptr;
        p->str = emalloc(strlen(str) + 1);
        strcpy(p->str, str);
        p->num = num;
        p->base = num;
        p->count = getlength(ulp);
    }
    // go to next item of the list
    else {
        build_label_list(p->next, ulp, num, str);
    }
}


/* Build label-used-before-define list */
/* used in encoder.c encode I,J,O 
 *  ls    - used label list pointer
 *  str   - label string
 *  opstr - operation string such as "ANDI" 
 *  addr  - base address where undefined label is used
 */
void build_used_list(struct list* ls, char* str, char* opstr)
/*
 * purpose: simply add label_str and obfunc converted from opstr
 *    note: new node is added at the end (FIFO)
 *    struct list:
 *              char* str = label
 *              num       = opfunc
 *              base      = address
 *              count     = no use
 *              next      = next used label node
 *              add       = maybe no use
 */
{
    struct list* p = ls;
    for (; p->next ; p = p->next) {}

    // at the end
    if (p->next == NULL) {
        struct list* newptr = emalloc(sizeof(struct list));
        newptr->next = NULL;
        p->next = newptr;
        p = newptr;
        p->str = emalloc(strlen(str) + 1);
        strcpy(p->str, str);
        p->num = opstr_to_opfunc(opstr);
        p->base = get_address();
    }
}


/**
* Label List Address Resolution Functions
*
* Address resolution is necessary after building label_list and used_list.
* Because each label address depends on all used labels that can cause autogen.
*
*    For example, the address of tag depends both tag1 and tag2.
*         ADDI $at, tag1 (undefined)
*         ADDI $at, tag2 (undefined)
*         tag1: 
*
*    The following algorithm is used to solve this: 
*        1) keep track of undefined labels in used_list.
*        2) when label appears, build label_list.
*        3) label_list keeps count of undefined labels.
*        4) label_list keeps base as no autogen address.
*        5) updated address is obtained by base + increase by autogen.
*        6) go through the label_list to update all label address.
*        7) because addresses depend on each other, iterate 6) until stabilized.
*/

/*　NOTE: Usage of label list pointer (llp) and used label pointer (ulp)　
 *        
 *  Both llp and ulp is a pointer of struct list, but its fields are used for different purpose.
 *  
 *  label list pointer (llp)
 *      str   - label string such as "tag"
 *      num   - address of label. it can be overwitten during address resolution
 *      base  - base address of label (no autogen) where the label is defined
 *      count - length of used label list before this label is defined.
 *  
 *  used label pointer (ulp)
 *      str   - label string such as 'tag'
 *      num   - opfunc that corresponds to operations such as ADDI
 *      base  - base address where undefined label is used. it can be overwritten during address resolution
 *      count - no use
 */

/* Helper: get address for label and return autogen address increase */
/*
 * llp       - label list pointer
 * label     - label string to be used as a key to find label address
 * opfunc    - used to determine the size of increase
 * base_addr - base address of instruction where undefined label is used
 */
int get_autogen_increase(struct list* llp, char* label, int opfunc, int base_addr)
{
    int num = getnum(llp, label);  // get address of label that is used as argument
    if (num != -1 && is_autogen(num, opfunc, base_addr)) { // regexp.c
        return opfunc_to_increase(opfunc);
    }
    return 0; // no autogen, no increase
}

/* Helper: traverse each node in used list and return sum of autogen_increase */
/*
 * llp     - label list pointer. llp->num keeps the length of ulp at the time the label is defined.
 * ulp     - used label pointer keeps all of labels, so we need to limit the length. ulp->num keeps opfunc.
 * len     - endpoint of ulp to examine autogen_increase.
 */
int sum_autogen_increase(struct list *llp, struct list* ulp, int len)
{
    int rv = 0;
    int increase = 0;
    struct list* p = ulp;

    for (p = p->next; p && len > 0; p = p->next, len -= 1) { // examine upto len
        increase = get_autogen_increase(llp, p->str, p->num, p->base);
        rv += increase;        
    }
    for ( ; p; p = p->next) {  // base address shift globally after this label is used
        p->base += increase;
    }        
    return rv;
}

/* Helper to go through label list one-time to update label list 
   return 1 if updated, 0 if not */
/*
 * llp - label list pointer. llp->count keeps the length of ulp when the label is defined.
 * ulp - used label pointer is not used in this funcion, just passing to sum_autogen_increase
 */
int addr_update(struct list* llp, struct list* ulp)
{
    int is_updated = 0;    // FSM to check if update happens
    int newaddr;
    struct list* p = llp;
    for (p = p->next; p; p = p->next) {
        newaddr = p->base + sum_autogen_increase(llp, ulp, p->count);
        if (p->num != newaddr)
            is_updated = 1;
        p->num = newaddr;  // UPDATE address
    }        
    return is_updated;
}

/* iterate until no addr_update needed */
/*
 * llp - label list pointer keeps defined labels. llp->num is address of label to be resolved during this function.
 * ulp - used label pointer keeps used-before-defined labels. ulp->num is opfunc to keep instructions.
 */
void addr_resolution(struct list* llp, struct list* ulp)
{
    for (;;) {
        if (addr_update(llp, ulp) == 0) break;
    }
}


/* DEBUG: Display label and associative label list */
void dump_label_list(struct list* llp, struct list* ulp)
{
    void dump_used_list_len(struct list* ls, int len);

    struct list* p = llp;
    for (p = p->next; p; p = p->next) {
        printf("label is [%s]\t address is [%d] len is [%d]\n", p->str, p->num, p->count);
        dump_used_list_len(ulp, p->count);
    }        
}

/* DEBUG: Display used label info upto the length */
void dump_used_list_len(struct list* ls, int len)
{    
    struct list* p = ls;
    for (p = p->next; p && len > 0; p = p->next, len -= 1) {
        printf("\t[%s]\topfunc is [%s]\n", p->str, opfunc_to_opstr(p->num));
    }    
}



/**
* General List Operation Functions
*/

/* traverse the linked list to free the heap memory */
void freelist(struct list* ls)
{
    struct list* p = ls;
    for (p = p->next; p; ) {
        free(p->str);
        struct list* oldptr = p;
        p = p->next;
        free(oldptr);
    }    
}

/* return string from list when num is matched */
char* getstr(struct list* ls, int num)
{
    struct list* p = ls;
    for (p = p->next; p; p = p->next) {
        if(p->num == num) {
            return p->str;
        }
    }
    return NULL;  // not found
}

/* return num when string is matched */
int getnum(struct list* ls, char* str)
{
    struct list* p = ls;
    for (p = p->next; p; p = p->next) {
        if(strcmp(str, p->str) == 0)
            return p->num;
    }
    return -1;  // not found
}

/* return count when string is matched */
int getcount(struct list* ls, char* str)
{
    struct list* p = ls;
    for (p = p->next; p; p = p->next) {
        if(strcmp(str, p->str) == 0)
            return p->count;
    }
    return -1;  // not found
}

/* return length of the list */
int getlength(struct list* ls)
{
    int rv = 0;
    struct list* p = ls;
    for (p = p->next; p; p = p->next) {
        rv += 1;
    }
    return rv;
}


/* traverse the linked list and display str with count, and total */
void dumplist(struct list* ls, char* num_name, char num_dec_hex)
{
    struct list* p = ls;
    int total = 0;
    for (p = p->next; p; p = p->next) {
        total += 1;
        if (num_dec_hex == 'h')  // hex representation
            printf("%-16s\t: %s is 0x[%04x] 0x[%04x] (mif)\n", p->str, num_name, p->num, p->num/2);
        else                     // decimal representation
            printf("%-4s  \t: %s %d\n", p->str, num_name, p->num);            
    }
    printf("--\n%-4s\t: %d\n", "TOTAL LINES", total);
}


/* DEBUG */
void linkedlist_test()
{
    ps(__func__)

    struct list label_list;
    struct list* llp = &label_list;
    init_list(llp, NULL);

    struct list used_list;
    struct list* ulp = &used_list;
    init_list(ulp, NULL);

    /*
    TEST for addr_resolution
    R boundary is 0x3, increase is 24
    J boundary is 0x7, increase is 20 
    After resolution, tag will be 50, tag2 will be 96
    */  
    build_used_list(ulp, "tag",  "ADDI");
    build_used_list(ulp, "tag2", "ORI");

    build_label_list(llp, ulp, 2, "tag");

    build_used_list(ulp, "tag",  "ANDI");
    build_used_list(ulp, "tag2", "J");

    build_label_list(llp, ulp, 4, "tag2");


    addr_resolution(llp, ulp);
    dump_label_list(llp, ulp);

    freelist(ulp);
    freelist(llp);
}


/**
* Helper Functions
*/

void * emalloc(size_t n)
/*
 * purpose: wrapp malloc to handle error
 */
{
    void *rv ;
    if ( (rv = malloc(n)) == NULL )
        oops("malloc");
    return rv;
}


