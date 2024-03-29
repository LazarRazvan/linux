# CC:		Compiler
# CFLAGS:	Compiler flags
# INC:		Project include path
CC := gcc
INC :=-Iinc
CFLAGS := -Wall

1G_FILE := run/1g_file
10M_FILE := run/10m_file
100K_FILE := run/100k_file

###############################################################################
# Default rule
# run install rule and create executable files
##

all: install run/file_buffering run/file_fsync run/my_cp run/open
	@echo "================================================"
	@echo "io build successfully"
	@echo "================================================"

.PHONY: all

###############################################################################
# Install rule
# Create "run" and "obj" directories if not found and 1G file to be used for
# my_cp
##

install:
	test -d "obj" || mkdir obj
	test -d "run" || mkdir run
	dd if=/dev/zero of=$(1G_FILE) bs=1 count=0 seek=1G
	dd if=/dev/zero of=$(10M_FILE) bs=1 count=0 seek=10M
	dd if=/dev/zero of=$(100K_FILE) bs=1 count=0 seek=100K

.PHONY: install

###############################################################################
# Executable files rule
##

run/file_fsync: obj/file_fsync.o obj/process_time.o
	$(CC) $(CFLAGS) $^ -o $@

run/file_buffering: obj/file_buffering.o obj/process_time.o
	$(CC) $(CFLAGS) $^ -o $@

run/my_cp: obj/my_cp.o
	$(CC) $(CFLAGS) $< -o $@

run/open: obj/open.o
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
	@echo "io clean successfully"
	@echo "================================================"

.PHONY: clean
