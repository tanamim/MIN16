/*
 * linkedlist.h -- data strucure to store data
 */

#ifndef LINKEDLIST_INCL
#define LINKEDLIST_INCL

/*
    name:       list
    purpose:    store str and number, and adjustments
    field:      str   - a str
                num   - a number
                base  - a base number (used in label list)
                count - a counter
                addr  - an address
                next  - a pointer to next node
                add   - a function pointer to add str and number
*/
struct list {
        char *str;
        int   num;
        int   base;
        int   count;
        struct list* next;
        void (*add) (struct list*, char*, int);
};

void  init_list(struct list*, void (*f) (struct list*, char*, int));
void  build_error_list(struct list*, char*, int);
void  build_label_list(struct list* llp, struct list* ulp, int num, char* str);
void  build_used_list(struct list* ls, char* str, char* opstr);
void  addr_resolution(struct list* llp, struct list* ulp);

void  freelist(struct list*);
char* getstr(struct list* ls, int num);
int   getnum(struct list*, char*);
int   getcount(struct list*, char*);
int   getlength(struct list*);
void  dumplist(struct list*, char*, char);
void  linkedlist_test();    // DEBUG

#endif /* LINKEDLIST_INCL */