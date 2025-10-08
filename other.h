#ifndef OTHER_H
#define OTHER_H

#include <stdio.h>
#include <stdlib.h>
#include <unordered_set>
#include <string>

#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

#define INIT 0
#define TOPIC 1
#define SUBSCRIBE 2
#define UNSUBSCRIBE 3

// struct for comm over UDP
typedef struct {
    char topic[50];
    u_int8_t type;
    char payload[1500];
} __attribute__((packed)) udp_comm;

// struct for comm over TCP
typedef struct {
    u_int8_t message_type;
    u_int8_t id_taken;
    int id;
    char topic[50];
    udp_comm message;
} __attribute__((packed)) tcp_comm;

#define NOT_CONNECTED -1

// struct for user data
typedef struct {
    int id;
    int socket;
} user_data;

bool topics_correspond(std::string msg_topic, std::string sub_topic);

#endif