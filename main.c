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

  int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (socket_fd == -1) {
      fprintf(stderr, "socket() error: %s\n", strerror(errno));
      return EXIT_FAILURE;
  }
  
  /* traceroute */
  traceroute(address, socket_fd);

  close(socket_fd);

  return EXIT_SUCCESS;
}