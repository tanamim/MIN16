#
# Makefile for CSCI-E93
#
# Type make to run
#

CC   = gcc -Wall
EXE  = emulator
LINK =
HDRS = common.h strfunc.h decoder.h executor.h
SRCS = $(EXE).c strfunc.c decoder.c executor.c
OBJS = $(SRCS:.c=.o)
FILE = ../asm/parser/sample3.mif

# declare phony targets
.PHONY: run test clean valgrind

# default target
$(EXE): $(OBJS) $(HDRS) Makefile
	@$(CC) $(OBJS) -o $(EXE) $(LINK)

# shortcut for development
run: $(EXE)
	@./$(EXE) $(FILE)

test: $(EXE)
	@./$(EXE) ../asm/parser/sample.mif

clean:
	@echo "Cleaning done."
	@rm -f $(EXE) $(OBJS)

valgrind:
	@rm -f $(EXE) $(OBJS)
	@make
	@valgrind ./$(EXE) $(FILE)

# dependencies
$(OBJS): $(HDRS)
