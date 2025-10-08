#ifndef UTILS_COMM_H
#define UTILS_COMM_H

#include <stdio.h>
#include <stdlib.h>

int recv_all(int sockfd, void *buffer, size_t len);
int send_all(int sockfd, void *buffer, size_t len);

#endif