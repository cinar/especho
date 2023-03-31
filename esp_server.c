/**
 * Copyright (c) 2023 Onur Cinar. All Rights Reserved.
 * The source code is provided under MIT License.
 *
 * https://github.com/cinar/esptimeout
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "esp.h"

#define DEFAULT_SERVER "::"

char *server = NULL;
int protocol = IPPROTO_ESP;

void parse_opts(int argc, char **argv);
void run();

int main(int argc, char **argv) {
  parse_opts(argc, argv);
  run();
  free(server);

  return EXIT_SUCCESS;
}

void parse_opts(int argc, char **argv) {
  int opt;

  while (-1 != (opt = getopt(argc, argv, "s:p:"))) {
    switch (opt) {
    case 's':
      server = strdup(optarg);
      break;

    case 'p':
      protocol = atoi(optarg);
      break;

    default:
      fprintf(stderr, "Usage: %s [-s server] [-p protocol]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (NULL == server) {
    server = strdup(DEFAULT_SERVER);
  }
}

void run() {
  struct sockaddr_in6 server_address;
  int sd;

  sd = socket(AF_INET6, SOCK_RAW, protocol);
  if (sd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin6_family = AF_INET6;
  server_address.sin6_port = htons(protocol);

  if (1 != inet_pton(AF_INET6, server, &(server_address.sin6_addr))) {
    perror("inet_pton");
    exit(EXIT_FAILURE);
  }

  if (-1 == bind(sd, (const struct sockaddr *)&server_address,
                 sizeof(server_address))) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  printf("Waiting for new connections.\n");
  while (1) {
    struct sockaddr_in6 client_address;
    socklen_t client_address_length = sizeof(client_address);
    char client[INET6_ADDRSTRLEN];
    struct esp_packet packet;
    ssize_t size;

    memset(&client_address, 0, sizeof(client_address));
    memset(&packet, 0, sizeof(packet));

    size = recvfrom(sd, &packet, sizeof(packet), 0,
                    (struct sockaddr *)&client_address, &client_address_length);
    if (-1 == size) {
      perror("recvfrom");
      exit(EXIT_FAILURE);
    }

    if (0 == inet_ntop(AF_INET6, &(client_address.sin6_addr), client,
                       sizeof(client))) {
      perror("inet_pton");
      exit(EXIT_FAILURE);
    }

    if (sizeof(packet) != size) {
      fprintf(stderr, "[Client %s] Skipping packet with size %ld != %ld).\n",
              client, size, sizeof(packet));
    } else {
      pid_t pid = fork();
      if (-1 == pid) {
        perror("fork");
        exit(EXIT_FAILURE);
      } else if (0 == pid) {
        short timeout = ntohs(packet.timeout);

        fprintf(stderr, "[Client %s] Requested %d seconds timeout.\n", client,
                timeout);
        sleep(timeout);

        packet.seq = htonl(2);

        fprintf(stderr, "[Client %s] Responding back.\n", client);
        size = sendto(sd, &packet, sizeof(packet), 0,
                      (const struct sockaddr *)&client_address,
                      client_address_length);
        if (-1 == size) {
          perror("sendto");
          exit(EXIT_FAILURE);
        }

        break;
      }
    }
  }

  if (-1 == close(sd)) {
    perror("close");
    exit(EXIT_FAILURE);
  }
}
