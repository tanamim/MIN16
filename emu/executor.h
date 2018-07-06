/*
 * executor.h -- execute instruction functions
 */

#ifndef EXECUTOR_INCL
#define EXECUTOR_INCL

uint16_t get_pc();
void execute(uint16_t);
void executor_test(int);
void display_info_with_execution(int);

#endif /* EXECUTOR_INCL */