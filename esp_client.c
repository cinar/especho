/**
 * Copyright (c) 2023 Onur Cinar. All Rights Reserved.
 * The source code is provided under MIT License.
 *
 * https://github.com/cinar/especho
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "esp.h"

#ifndef DEFAULT_SERVER
#define DEFAULT_SERVER "::1"
#endif

#define DEFAULT_CLIENT "::"
#define DEFAULT_TIMEOUT_BEGIN 10
#define DEFAULT_TIMEOUT_END 600
#define DEFAULT_TIMEOUT_INCREMENT 10

char *server = NULL;
char *client = NULL;
int protocol = IPPROTO_ESP;
short timeout_begin = DEFAULT_TIMEOUT_BEGIN;
short timeout_end = DEFAULT_TIMEOUT_END;
short timeout_increment = DEFAULT_TIMEOUT_INCREMENT;

void parse_opts(int argc, char **argv);
void run();

int main(int argc, char **argv) {
  parse_opts(argc, argv);
  fprintf(stderr, "Connecting to %s with timeout %d to %d with %d.\n", server,
          timeout_begin, timeout_end, timeout_increment);
  run();
  free(server);
  free(client);

  return EXIT_SUCCESS;
}

void parse_opts(int argc, char **argv) {
  int opt;

  while (-1 != (opt = getopt(argc, argv, "s:c:p:b:e:i:"))) {
    switch (opt) {
    case 's':
      server = strdup(optarg);
      break;

    case 'c':
      client = strdup(optarg);
      break;

    case 'p':
      protocol = atoi(optarg);
      break;

    case 'b':
      timeout_begin = atoi(optarg);
      break;

    case 'e':
      timeout_end = atoi(optarg);
      break;

    case 'i':
      timeout_increment = atoi(optarg);
      break;

    default:
      fprintf(stderr,
              "Usage: %s [-s server] [-c client] [-p protocol] [-b begin] [-e end] [-i "
              "increment]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (NULL == server) {
    server = strdup(DEFAULT_SERVER);
  }

  if (NULL == client) {
    client = strdup(DEFAULT_CLIENT);
  }
}

void run() {
  struct sockaddr_in6 client_address;
  struct sockaddr_in6 server_address;
  int sd;
  struct esp_packet packet;
  ssize_t size;
  struct timeval tv;
  short timeout_max;
  short timeout;

  sd = socket(AF_INET6, SOCK_RAW, protocol);
  if (sd == -1) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  memset(&client_address, 0, sizeof(client_address));
  client_address.sin6_family = AF_INET6;
  client_address.sin6_port = htons(protocol);

  if (1 != inet_pton(AF_INET6, client, &(client_address.sin6_addr))) {
    perror("inet_pton");
    exit(EXIT_FAILURE);
  }

  if (-1 == bind(sd, (const struct sockaddr *)&client_address,
                 sizeof(client_address))) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin6_family = AF_INET6;

  if (1 != inet_pton(AF_INET6, server, &(server_address.sin6_addr))) {
    perror("inet_pton");
    exit(EXIT_FAILURE);
  }

  timeout_max = -1;

  for (timeout = timeout_end; timeout >= timeout_begin;
       timeout -= timeout_increment) {
    memset(&packet, 0, sizeof(packet));
    packet.spi = htonl(2023);
    packet.seq = htonl(1);
    packet.timeout = htons(timeout);

    size = sendto(sd, &packet, sizeof(packet), 0,
                  (const struct sockaddr *)&server_address,
                  sizeof(server_address));
    if (-1 == size) {
      perror("sendto");
      exit(EXIT_FAILURE);
    }

    tv.tv_sec = timeout + 4;
    tv.tv_usec = 0;

    if (-1 == setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv,
                         sizeof(tv))) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Waiting for %d seconds.\n", timeout);

    memset(&packet, 0, sizeof(packet));
    size = recv(sd, &packet, sizeof(packet), 0);
    if (-1 == size) {
      continue;
    }

    timeout_max = timeout;
    break;
  }

  if (-1 != timeout_max) {
    fprintf(stderr, "Max working timeout %d.\n", timeout);
  } else {
    fprintf(stderr, "No working timeout found.\n");
  }

  if (-1 == close(sd)) {
    perror("close");
    exit(EXIT_FAILURE);
  }
}
