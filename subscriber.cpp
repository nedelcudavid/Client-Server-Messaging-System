#include <iostream>
#include <string>

// socket libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <poll.h>
#include <netinet/tcp.h>

#include "other.h"
#include "utils_comm.h"

using namespace std;

void print_message(udp_comm message) {
    // get message topic
    char topic[51];
    strncpy(topic, message.topic, 50);
    topic[50] = '\0';
    string topic_str(topic);

    // get message type and payload
    u_int8_t type = message.type;
    string message_type;
    switch (type) {
        case 0:
            message_type = "INT";
            break;
        case 1:
            message_type = "SHORT_REAL";
            break;
        case 2:
            message_type = "FLOAT";
            break;
        case 3:
            message_type = "STRING";
            break;
        default:
            message_type = "UNKNOWN";
            break;
    }

    string payload;
    uint8_t sign, exp;
    uint16_t num_short_real;
    uint32_t num;
    float num_float;
    float power = 1.0f;

    // parse the payload
    switch (type) {
        case 0:
            sign = message.payload[0];
            num = ntohl(*(uint32_t *) (message.payload + 1));
            printf("%s - %s - %d\n", topic_str.c_str(), message_type.c_str(), (sign == 0 ? num : -num));
            break;
        case 1:
            printf("%s - %s - ", topic_str.c_str(), message_type.c_str());
            num_short_real = ntohs(*(uint16_t *) message.payload);
            if (num_short_real % 100 == 0) {
                printf("%d.00\n", num_short_real / 100);
                break;
            }
            if (num_short_real % 100 < 10) {
                printf("%d.0%d\n", num_short_real / 100, num_short_real % 100);
                break;
            }
            printf("%d.%d\n", num_short_real / 100, num_short_real % 100);
            break;
        case 2:
            sign = message.payload[0];
            exp = message.payload[5];
            num = ntohl(*(uint32_t *) (message.payload + 1));

            num_float = (float) num;
            for (int i = 0; i < exp; i++) {
                power *= 10.0f;
            }

            num_float /= power;

            printf("%s - %s - ", topic_str.c_str(), message_type.c_str());
            if (sign == 1) {
                printf("-");
            }
            // dont print 0s after the decimal point if they are not significant
            if (num_float == (int) num_float) {
                printf("%.0f\n", num_float);
            } else {
                printf("%.4f\n", num_float);
            }

            break;
        case 3:
            payload = string(message.payload);
            printf("%s - %s - %s\n", topic_str.c_str(), message_type.c_str(), payload.c_str());
            break;
        default:
            payload = string(message.payload);
            printf("%s - %s - %s\n", topic_str.c_str(), message_type.c_str(), payload.c_str());
            break;
    }
}

int main(int argc, char **argv) {
    // terminal input: subscriber <id> <ip> <port>
    if (argc != 4) {
        return -1;
    }
    
    // deactivate buffering
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int id = atoi(argv[1] + 1);
    std::string ip = argv[2];
    int port = atoi(argv[3]);

    // create a socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        return -1;
    }

    // connect to the server
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_aton(ip.c_str(), &server_address.sin_addr);

    int connect_status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (connect_status == -1) {
        return -1;
    }

    // send the id to the server
    tcp_comm message;
    message.message_type = INIT;    
    message.id = id;
    if (send_all(client_socket, (char *) &message, sizeof(message)) < 0) {
        return -1;
    }

    // receive the server response
    if (recv_all(client_socket, (char *) &message, sizeof(message)) < 0) {
        return -1;
    }

    // check if the server accepted the connection
    if (message.id_taken == 1) {
        return -1;
    }

    // create a poll vector
    std::vector<struct pollfd> pollfds;
    pollfds.push_back({client_socket, POLLIN, 0});
    // add the keyboard input
    pollfds.push_back({0, POLLIN, 0});

    // deactivate NAGLE
    int flag = 1;
    int opt_result = setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (void*) &flag, sizeof(int));
    if (opt_result < 0) {
        return -1;
    }

    // receive messages from the server
    while (true) {
        int poll_status = poll(pollfds.data(), pollfds.size(), -1);
        if (poll_status < 0) {
            return -1;
        }

        memset(&message, 0, sizeof(message));

        // check for keyboard input
        if (pollfds[1].revents & POLLIN) {
            char buffer[1500];
            fgets(buffer, 1500, stdin);
            buffer[strlen(buffer) - 1] = '\0';

            // check if the user wants to exit
            if (strcmp(buffer, "exit") == 0) {
                // close the socket
                close(client_socket);
                return 0;
            }

            // check if the user wants to subscribe
            if (strncmp(buffer, "subscribe", 9) == 0) {
                // get topic from buffer 
                char *topic = buffer + 10;
                // send the topic to the server
                message.message_type = SUBSCRIBE;
                strcpy(message.topic, topic);
                if (send_all(client_socket, (char *) &message, sizeof(message)) < 0) {
                    return -1;
                }
                printf("Subscribed to topic %s\n", topic);
            }

            // check if the user wants to unsubscribe
            if (strncmp(buffer, "unsubscribe", 11) == 0) {
                // get topic from buffer
                char *topic = buffer + 12;
                // send the topic to the server
                message.message_type = UNSUBSCRIBE;
                strcpy(message.topic, topic);
                if (send_all(client_socket, (char *) &message, sizeof(message)) < 0) {
                    return -1;
                }
                printf("Unsubscribed from topic %s\n", topic);
            }
        }

        // check for messages from the server
        if (pollfds[0].revents & POLLIN) {
            // receive the message
            int recv_status = recv_all(client_socket, (char *) &message, sizeof(message));
            if (recv_status < 0) {
                return -1;
            }

            // check for serv disconnect
            if (recv_status == 0) {
                // close the socket
                close(client_socket);
                return 0;
            }

            // probably a topic message - print it
            print_message(message.message);
        }
    }

    return 0;
}