#
# Makefile for CSCI-E93
#
# Type make to run
#

CC   = gcc -g -Wall
EXE  = parser
LINK = -lm
HDRS = regexp.h linkedlist.h directives.h strfunc.h common.h decoder.h encoder.h
SRCS = $(EXE).c regexp.c linkedlist.c directives.c strfunc.c decoder.c encoder.c
OBJS = $(SRCS:.c=.o)
FILE = sample.txt

# declare phony targets
.PHONY: run clean valgrind

# default target
$(EXE): $(OBJS) $(HDRS) Makefile
	@$(CC) $(OBJS) -o $(EXE) $(LINK)

# shortcut for development
run: $(EXE)
	@./$(EXE) $(FILE)

run2: $(EXE)
	@./$(EXE) sample2.txt

test: $(EXE)
	@./$(EXE) sample3.txt

clean:
	@echo "Cleaning done."
	@rm -f $(EXE) $(OBJS)

valgrind:
	@rm -f $(EXE) $(OBJS)
	@make
	@valgrind ./$(EXE) $(FILE)

# dependencies
$(OBJS): $(HDRS)
