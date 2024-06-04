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

## sockets
### unix_domain
#### stream_server/stream_client
TCP Client/Server application example using unix domain sockets.

#### datagram_server/datagram_client
UDP Client/Server application example using unix domain sockets.

#### datagram_socketpair
UDP Client/Server application example using unix domain sockets created by
socketpair() and fork() system calls.

### internet_domain
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
#### datagram_server/datagram_client
UDP Client/Server application example using internet domain sockets.

#### dns_example
DNS showing resolving a host using getaddrinfo().

#### datagram_nameinfo_server
UDP Server application that also extract the hostname and service of the client.

### internet_domain_generic
#### utils
Generic implementation for bind, connect and listen using getaddrinfo() that is
able to perform name and port resolution, removing the constraint of always
knowing server ip address or port.

#### tcp_client
Generic implementation for a tcp client that read data from standard input and
send them to server after establishing the connection.

#### it_tcp_server
Iterative tcp server is implemented using the generic functions using
IPv4 and stream sockets for communication. It will only continue with the next
client after the connection with current client is closed.

The it_tcp_server can be tested using **/run/tcp_client**.
```
./run/tcp_client
```

The it_tcp_server can also be tested using **telnet** command.
```
telnet localhost 50001
```

#### it_echo_server/it_echo_client
Iterative udp client-server is implemented using the generic functions using
IPv4 and datagram sockets for communication.

#### proc_con_tcp_server
Concurent tcp server is implemented using the generic functions using
IPv4 and stream sockets for communication. A new process is created for each
connection reaching to the server, dealing with clients in parallel.

The proc_con_tcp_server can be tested using **/run/tcp_client**.
```
./run/tcp_client
```

The proc_con_tcp_server can also be tested using **telnet** command.
```
telnet localhost 50001
```

#### thread_con_tcp_server
Concurent tcp server is implemented using the generic functions using
IPv4 and stream sockets for communication. A new thread is created for each
connection reaching to the server, dealing with clients in parallel.

The proc_con_tcp_server can be tested using **/run/tcp_client**.
```
./run/tcp_client
```

The proc_con_tcp_server can also be tested using **telnet** command.
```
telnet localhost 50001
```

#### thread_pool_con_tcp_server
Concurent tcp server is implemented using the generic functions using
IPv4 and stream sockets for communication. To speed up the process, a thread
pool is used to reduce the overhead of creating and joining a new thread on
each new connection.

The thread_pool_con_tcp_server can be tested using **/run/tcp_client**.
```
./run/tcp_client
```

The thread_pool_con_tcp_server can also be tested using **telnet** command.
```
telnet localhost 50001
```
