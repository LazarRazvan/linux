# CC:		Compiler
# CFLAGS:	Compiler flags
# INC:		Project include path
CC := gcc
INC :=-Iinc
CFLAGS := -Wall

###############################################################################
# Default rule
# Run intall rule and create executable files
##

all: install run/program_break run/layout run/fork run/vfork
	@echo "================================================"
	@echo "processes build successfully"
	@echo "================================================"

.PHONY: all

###############################################################################
# Install rule
# Create "run" and "obj" directories if not found.
##

install:
	test -d "obj" || mkdir obj
	test -d "run" || mkdir run

.PHONY: install

###############################################################################
# Executable files rule
##

run/program_break: obj/program_break.o
	$(CC) $(CFLAGS) $< -o $@

run/layout: obj/layout.o
	$(CC) $(CFLAGS) $< -o $@

run/fork: obj/fork.o
	$(CC) $(CFLAGS) $< -o $@

run/vfork: obj/vfork.o
	$(CC) $(CFLAGS) $< -o $@

###############################################################################
# Object file rule
##

obj/%.o : src/%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

###############################################################################
# Clean rule, remove object and executable files directories.
##

clean:
	rm -rf obj run
	@echo "================================================"
	@echo "processes clean successfully"
	@echo "================================================"

.PHONY: clean
