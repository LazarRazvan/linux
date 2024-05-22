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

## unix_domain_sockets
```
# Create virtual ethernet interfaces
sudo ip link add veth0 type veth peer name veth1

sudo ip addr add 192.168.1.1/24 dev veth0
sudo ip addr add 192.168.1.2/24 dev veth1

sudo ip link set veth0 up
sudo ip link set veth1 up

# Remove virtual ethernet interfaces
sudo ip link del veth0
```
### datagram_server/datagram_client
UDP Client/Server application example using internet domain sockets.

### dns_example
DNS showing resolving a host using getaddrinfo().

### datagram_nameinfo_server
UDP Server application that also extract the hostname and service of the client.

### internet_domain_sockets_generic

#### it_tcp_server/it_tcp_client
Generic implementation for bind, connect and listen using getaddrinfo() that is
able to perform name and port resolution, removing the constraint of always
knowing server ip address or port.

As usecase, a simple iterative tcp client-server is implemented using the
generic functions using IPv4 and stream sockets for communication.

The it_tcp_server can also be tested using **telnet** command.
```
telnet localhost 50001
```
