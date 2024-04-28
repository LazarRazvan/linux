# linux
Examples for Linux programming interface (system calls, library functions, etc ...)

## io
### open.c
Open a file using different flags and modes.

### my_cp.c
Basic example of ```cp``` Linux command.

### file_buffering.c
Kernel buffer mechanism and impact of syscalls.

## time
### calendar_time.c
Calendar time, break down functions and print examples.

### measure.c
Measure process time (system and user time) for a component using process time
implementation.

## processes
### layout.c
Process segments (text, initialized data and uninitialized data)

### program_break.c
Explain heap memory usage, how program break increase and decrease.

## signals
### signal.c
signal() system call to change disposition for a particular signal and ignore a
signal.

## unix_domain_sockets
### stream_server/stream_client
TCP Client/Server application example using unix domain sockets.

### datagram_server/datagram_client
UDP Client/Server application example using unix domain sockets.

### datagram_socketpair
UDP Client/Server application example using unix domain sockets created by
socketpair() and fork() system calls.
