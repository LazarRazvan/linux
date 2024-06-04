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

all: install run/it_tcp_server run/it_echo_server run/tcp_client\
	run/it_echo_client run/proc_con_tcp_server run/thread_con_tcp_server\
	run/thread_pool_con_tcp_server

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

run/it_tcp_server: obj/utils.o obj/it_tcp_server.o
	$(CC) $(CFLAGS) $^ -o $@

run/it_echo_server: obj/utils.o obj/it_echo_server.o
	$(CC) $(CFLAGS) $^ -o $@

run/tcp_client: obj/utils.o obj/tcp_client.o
	$(CC) $(CFLAGS) $^ -o $@

run/it_echo_client: obj/utils.o obj/it_echo_client.o
	$(CC) $(CFLAGS) $^ -o $@

run/proc_con_tcp_server: obj/utils.o obj/proc_con_tcp_server.o
	$(CC) $(CFLAGS) $^ -o $@

run/thread_con_tcp_server: obj/utils.o obj/thread_con_tcp_server.o
	$(CC) $(CFLAGS) $^ -o $@

run/thread_pool_con_tcp_server: obj/utils.o obj/thread_pool_con_tcp_server.o
	$(CC) $(CFLAGS) $^ -o $@

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
