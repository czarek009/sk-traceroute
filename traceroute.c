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
    //packet->checksum = 0;
    packet->checksum = compute_icmp_checksum(packet, sizeof(struct icmphdr));
}


bool recv_response(int socket_fd, uint8_t ttl) {

  struct timeval time;
  time.tv_sec = 1;
  time.tv_usec = 0;
  uint8_t responses = 0;

  while (responses < 3) {
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    u_int8_t buffer[IP_MAXPACKET];

    ssize_t packet_len = recvfrom(socket_fd, 
                                  buffer, IP_MAXPACKET, 
                                  0, 
                                  (struct sockaddr*)&sender, 
                                  &sender_len);

    char ip_str[20];
    inet_ntop (AF_INET, &(sender.sin_addr), ip_str, sizeof(ip_str));
    printf ("IP packet with ICMP content from: %s\n", ip_str);
  }

  return true;
}


void traceroute(const struct sockaddr* addr, int socket_fd) {
  for (uint8_t ttl = 1; ttl <= 32; ++ttl) {
    for (uint8_t i = 0; i < 3; ++i) {
      struct icmphdr packet;
      create_packet(&packet, seq_num(ttl, i));

      if (!setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int))) {
        fprintf(stderr, "setsockopt() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }

      /* send packet */
      //size_t bytes_sent;
      //if((bytes_sent = send(socket_fd, &packet, sizeof(packet), 0)) == -1) {
      //  fprintf(stderr, "send() error: %s\n", strerror(errno));
      //  exit(EXIT_FAILURE);
      //}

      if (sendto(socket_fd,
                 &packet,
                 sizeof(struct icmphdr),
                 0,
                 addr,
                 sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "sendto() error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      // sprawdzanie czy wysłano wszystko?
    }
    /* receive packets */
    recv_response(socket_fd, ttl);
  }
}