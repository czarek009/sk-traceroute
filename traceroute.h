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


#define TIMEOUT 0

typedef struct responders {
    char ip[20];
    struct timeval time;
} responders_t;


void traceroute(const struct sockaddr* addr, int socket_fd, const char* address);

#endif