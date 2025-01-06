#ifndef SOCKETS_H
#define SOCKETS_H

int passive_socket_init(const int server_port, int backlog);

int wait_for_client_connection(int server_socket);

#endif  // SOCKETS_H
