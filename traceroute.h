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
    char *ip;
    struct timeval time;
} responders_t;


void traceroute(const char* address, int socket_fd);

#endif