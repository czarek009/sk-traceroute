#ifndef TRACEROUTE_H
#define TRACEROUTE_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define TIMEOUT -16


void traceroute(const struct sockaddr* addr, int socket_fd);

#endif