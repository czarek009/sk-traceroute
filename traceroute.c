// Cezary Stajszczyk 317354
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


void create_packet(struct icmphdr* packet, uint16_t sequential) {
    assert(packet != NULL);

    packet->type = ICMP_ECHO;
    packet->code = 0;
    packet->un.echo.id = getpid();
    packet->un.echo.sequence = sequential;
    packet->checksum = 0; // trzeba najpierw wyzerować, żeby dobrze policzyło checksum
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

    struct iphdr* ip_header = (struct iphdr*)buffer;
    ssize_t ip_header_size  = 4 * ip_header->ihl;
    struct icmphdr* icmp_header = (struct icmphdr*)(buffer+ip_header_size);
    int id = icmp_header->un.echo.id;
    int sequential_number = icmp_header->un.echo.sequence;
    
    if (icmp_header->type == ICMP_TIME_EXCEEDED) {
      void *original_packet = (uint8_t*)icmp_header + sizeof(struct icmphdr);
      struct iphdr* org_ip_header = (struct iphdr*)original_packet;
      ssize_t org_ip_header_size  = 4 * org_ip_header->ihl;
      struct icmphdr* org_icmp_header = (struct icmphdr*)((uint8_t*)original_packet + org_ip_header_size);
      int org_id = org_icmp_header->un.echo.id;
      int org_sequential_number = org_icmp_header->un.echo.sequence;

      if (org_id != getpid() || org_sequential_number>>2 != ttl) {
        continue;
      }
    } else if (icmp_header->type == ICMP_ECHOREPLY) {
      if (id != getpid() || sequential_number>>2 != ttl) {
        continue;
      }
    } else {
      continue;
    }
    
    responders[responses].ip = malloc(4);
    const char* sender_ip = inet_ntop(AF_INET, 
                                      &sender.sin_addr,
                                      responders[responses].ip, 
                                      20);

    if (sender_ip == NULL) {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }

    responders[responses].time.tv_usec = 1000000 - time.tv_usec;
    
    ++responses;
  }
  
  uint64_t time_total = 0;
  bool dest_responded = false;

  for (int i = 0; i < responses; ++i) {
    bool print_addr = true;
    time_total += responders[i].time.tv_usec;

    for (int j = 0; j < i; ++j) {
      if (strcmp(responders[i].ip, responders[j].ip) == 0) {
        print_addr = false;
        break;
      } 
    }

    if (print_addr) {
      printf("%s ", responders[i].ip);
    }
    if (strcmp(responders[i].ip, address) == 0) {
      dest_responded = true;
    } 
  }
  if (responses) {
    uint64_t average = time_total / 1000 / responses;
    printf("%ldms ", average);
  }

  if (responses == 0) {
    printf("* ");
  } else if (responses < 3) {
    printf("??? ");
  }

  printf("\n"); 

  return dest_responded;
}


void traceroute(const char* address, int socket_fd) {

  struct sockaddr addr = {0};
  addr.sa_family = AF_INET;
  if (inet_pton(AF_INET, address, &addr.sa_data[2]) != 1) {
    fprintf(stderr, "inet_pton() error: %s is not valid ip address; %s\n", address, strerror(errno));
    exit(EXIT_FAILURE);
  }

  for (int ttl = 1; ttl <= 32; ++ttl) {
    for (uint8_t i = 0; i < 3; ++i) {
      struct icmphdr packet;
      create_packet(&packet, seq_num(ttl, i));

      if (setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) == -1) {
        fprintf(stderr, "setsockopt() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }

      ssize_t bytes = sendto(socket_fd,
                             &packet,
                             sizeof(struct icmphdr),
                             0,
                             &addr,
                             sizeof(struct sockaddr));
      if (bytes == -1) {
        fprintf(stderr, "sendto() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      if (bytes != sizeof(packet)) {
        fprintf(stderr, "sendto() error: sent %ld out od %ld bytes\n", bytes, sizeof(packet));
        exit(EXIT_FAILURE);
      }
    }

    /* receive packets */
    if (recv_response(socket_fd, ttl, address)) {
      break;
    }
  }
}
