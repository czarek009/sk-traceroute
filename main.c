#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "traceroute.h"

int main(int argc, char* argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Usege: sudo ./traceroute [address]\n");
    return EXIT_FAILURE;
  }

  char* address = argv[1];
  struct sockaddr addr = {0};
  addr.sa_family = AF_INET;
  if (inet_pton(AF_INET, address, &addr.sa_data[2]) != 1) {
    fprintf(stderr, "inet_pton() error: %s is not valid ip address; %s\n", address, strerror(errno));
    return EXIT_FAILURE;
  }

  //printf("Decoded address: %d\n", addr.sin_addr);

  int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (socket_fd == -1) {
      fprintf(stderr, "socket() error: %s\n", strerror(errno));
      return EXIT_FAILURE;
  }

  /*
  if (!bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr))) {
    fprintf(stderr, "bind() error: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  */
  
  /* traceroute */
  traceroute(&addr, socket_fd);

  close(socket_fd);

  return EXIT_SUCCESS;
}