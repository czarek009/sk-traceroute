#include "traceroute.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>


static inline uint16_t seq_num(int ttl, int i) {
  return (ttl << 2) | i;
}


/* funkcja z wykładu */
u_int16_t compute_icmp_checksum(const void *buff, int length) {
	u_int32_t sum;
	const u_int16_t* ptr = buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}


void create_packet(struct icmphdr* packet, uint16_t seqential_number) {
    assert(packet != NULL);

    packet->type = ICMP_ECHO;
    packet->code = 0;
    packet->un.echo.id = getpid();
    packet->un.echo.sequence = seqential_number;
    packet->checksum = 0;
    packet->checksum = compute_icmp_checksum(packet, sizeof(struct icmphdr));
}


bool recv_response(int socket_fd, int ttl, const char* address) {

  uint8_t responses = 0;
  struct timeval time;
  time.tv_sec = 1;
  time.tv_usec = 0;

  responders_t responders[3] = {0};

  printf("%d. ", ttl);

  while (responses < 3) {

    fd_set descriptors;
    FD_ZERO(&descriptors);
    FD_SET(socket_fd, &descriptors);
    int ready = select(socket_fd+1, &descriptors, NULL, NULL, &time);
    
    if (ready < 0) {
      fprintf(stderr, "select() error: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (ready == TIMEOUT) {
      break;
    }

    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    u_int8_t buffer[IP_MAXPACKET];


    ssize_t packet_len = recvfrom(socket_fd, 
                 buffer, IP_MAXPACKET, 
                 0, 
                 (struct sockaddr*)&sender, 
                 &sender_len);

    if (packet_len == -1) {
      fprintf(stderr, "recvfrom() error: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

    const char* sender_ip = inet_ntop(AF_INET, 
                                      &sender.sin_addr,
                                      &responders[responses].ip, 
                                      20);

    if (sender_ip == NULL) {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }

    /* Jakieś wykrywanie, czy to aby na pewno dobry pakiet? */
    //printf("sender_ip: %s\n", responders[responses].ip);
    ++responses;
  }
  
  for (int i = 0; i < responses; ++i) {
    bool unique = true;

    for (int j = 0; j < i; j++) {
      if (strcmp(responders[i].ip, responders[j].ip) == 0) {
        unique = false;
        break;
      } 
    }

    if (unique) {
      printf("%s ", responders[i].ip);
    }
    if (strcmp(responders[i].ip, address) == 0) {
      printf("\n");
      return true;
    } 
  }

  if (responses == 0) {
    printf("* ");
  } else if (responses < 3) {
    printf("??? ");
  }

  //printf(" a powinno być: ");
  //for (int i = 0; i < 14; ++i) {
  //  printf("%c", addr->sa_data[i]);
  //}
  //printf(" a powinno być: %s", addr->sa_data);
  printf("\n");



  return false;
}


void traceroute(const struct sockaddr* addr, int socket_fd, const char* address) {
  for (int ttl = 1; ttl <= 32; ++ttl) {
    for (uint8_t i = 0; i < 3; ++i) {
      struct icmphdr packet;
      create_packet(&packet, seq_num(ttl, i));

      if (setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) == -1) {
        fprintf(stderr, "setsockopt() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }

      /* send packet */
      //size_t bytes_sent;
      //if((bytes_sent = send(socket_fd, &packet, sizeof(packet), 0)) == -1) {
      //  fprintf(stderr, "send() error: %s\n", strerror(errno));
      //  exit(EXIT_FAILURE);
      //}

      ssize_t bytes = sendto(socket_fd,
                             &packet,
                             sizeof(struct icmphdr),
                             0,
                             addr,
                             sizeof(struct sockaddr));
      if (bytes == -1) {
        fprintf(stderr, "sendto() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      if (bytes != sizeof(packet)) {
        fprintf(stderr, "sendto() error: sent %ld out od %ld bytes\n", bytes, sizeof(packet));
        exit(EXIT_FAILURE);
      }
      // sprawdzanie czy wysłano wszystko?
    }
    /* receive packets */
    if (recv_response(socket_fd, ttl, address)) {
      break;
    }
  }
}