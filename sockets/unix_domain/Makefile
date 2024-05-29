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

all: install run/stream_server run/stream_client run/datagram_server \
	run/datagram_client run/datagram_socketpair
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

run/stream_server: obj/stream_server.o
	$(CC) $(CFLAGS) $< -o $@

run/stream_client: obj/stream_client.o
	$(CC) $(CFLAGS) $< -o $@

run/datagram_server: obj/datagram_server.o
	$(CC) $(CFLAGS) $< -o $@

run/datagram_client: obj/datagram_client.o
	$(CC) $(CFLAGS) $< -o $@

run/datagram_socketpair: obj/datagram_socketpair.o
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
