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
#include <unordered_map>

using namespace std;

void handle_new_connection(int server_socket, vector<struct pollfd> &pollfds, vector<user_data> &users) {
    // Accept the new connection
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_size);
    if (client_socket == -1) {
        return;
    }

    // wait for the client to send the id
    tcp_comm message;
    if (recv_all(client_socket, (char *) &message, sizeof(message)) < 0) {
        return;
    }

    // check if the id is already taken
    bool id_taken = false;
    for (size_t i = 0; i < users.size(); i++) {
        if (users[i].id == message.id && users[i].socket != NOT_CONNECTED) {
            id_taken = true;
            break;
        }
    }

    // send the response to the client
    message.message_type = INIT;
    message.id_taken = id_taken;
    if (send_all(client_socket, (char *) &message, sizeof(message)) < 0) {
        return;
    }

    if (id_taken) {
        // print the error
        printf("Client C%d already connected.\n", message.id);

        close(client_socket);
        return;
    }

    // add the new user to the list
    user_data new_user;
    new_user.id = message.id;
    new_user.socket = client_socket;
    users.push_back(new_user);

    // add the new client to the poll vector
    pollfds.push_back({client_socket, POLLIN, 0});

    // print the new connection
    printf("New client C%d connected from %s:%d.\n", message.id, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
}

void handle_udp_message(int server_socket, vector<struct pollfd> &pollfds, vector<user_data> &users, unordered_map<string, vector<int>> &topic_subscribers) {
    // udp message, needs to be sent to all subscribers
    udp_comm message;
    memset(&message, 0, sizeof(message));
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int bytes_received = recvfrom(server_socket, (char *) &message, sizeof(message), 0, (struct sockaddr *) &client_address, &client_address_size);
    if (bytes_received < 0) {
        return;
    }

    // get the topic
    char topic[51];
    strncpy(topic, message.topic, 50);
    topic[50] = '\0';

    // get all the users that are subscribed to the topic - keep their IDs
    unordered_set<int> subscribed_users;

    // check if the topic is in the map
    for (auto it = topic_subscribers.begin(); it != topic_subscribers.end(); it++) {
        if (topics_correspond(topic, it->first)) {
            for (size_t i = 0; i < it->second.size(); i++) {
                subscribed_users.insert(it->second[i]);
            }
        }
    }

    unordered_set<int> connected_sub_users;

    // remove users that are not connected
    for (size_t i = 0; i < users.size(); i++) {
        if (subscribed_users.find(users[i].id) != subscribed_users.end()) {
            connected_sub_users.insert(users[i].socket);
        }
    }

    // create tcp message to send to the users
    tcp_comm tcp_message;
    tcp_message.message_type = TOPIC;
    strncpy(tcp_message.topic, topic, 50);
    memcpy(&tcp_message.message, &message, sizeof(message));

    // send the message to all the subscribed users
    for (auto it = connected_sub_users.begin(); it != connected_sub_users.end(); it++) {
        if (send_all(*it, (char *) &tcp_message, sizeof(tcp_message)) < 0) {
            return;
        }
    }
}

int handle_keyboard_input(vector<struct pollfd> &pollfds) {
    // only one option: exit
    char buffer[1500];
    cin.getline(buffer, sizeof(buffer));
    if (strcmp(buffer, "exit") == 0) {
        return -1;
    }
    return 0;
}

void handle_tcp_message(int client_socket, vector<struct pollfd> &pollfds, vector<user_data> &users, unordered_map<string, vector<int>> &topic_subscribers) {
    // read the message
    char buffer[1500];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received < 0) {
        return;
    }

    // check if the client disconnected
    if (bytes_received == 0) {
        // find the user
        for (size_t i = 0; i < users.size(); i++) {
            if (users[i].socket == client_socket) {
                // remove from connected users list
                users.erase(users.begin() + i);
                // print the message
                printf("Client C%d disconnected.\n", users[i].id);
                break;
            }
        }


        // close the socket
        close(client_socket);
        // remove the socket from the poll vector
        for (size_t i = 0; i < pollfds.size(); i++) {
            if (pollfds[i].fd == client_socket) {
                pollfds.erase(pollfds.begin() + i);
                break;
            }
        }

        return;
    }

    // check message type
    tcp_comm *message = (tcp_comm *) buffer;
    if (message->message_type == SUBSCRIBE) {
        // add the user to the topic
        string topic = message->topic;
        // find id
        int id = -1;
        for (size_t i = 0; i < users.size(); i++) {
            if (users[i].socket == client_socket) {
                id = users[i].id;
                break;
            }
        }

        if (id == -1) {
            return;
        }

        // add the user to the topic
        topic_subscribers[topic].push_back(id);

    } else if (message->message_type == UNSUBSCRIBE) {
        // remove the user from the topic
        string topic = message->topic;
        // find id
        int id = -1;
        for (size_t i = 0; i < users.size(); i++) {
            if (users[i].socket == client_socket) {
                id = users[i].id;
                break;
            }
        }

        // remove the user from the topic
        for (size_t i = 0; i < topic_subscribers[topic].size(); i++) {
            if (topic_subscribers[topic][i] == id) {
                topic_subscribers[topic].erase(topic_subscribers[topic].begin() + i);
                break;
            }
        }
    }
}

int main(int argc, char **argv) {
    // deactivate buffering
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // create a listening socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        return -1;
    }
    
    // get PORT from command line arguments
    if (argc != 2) {
        return -1;
    }
    
    int PORT = atoi(argv[1]);

    // bind the socket to an IP address and PORT
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int bind_status = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (bind_status == -1) {
        return -1;
    }

    // listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        return -1;
    }

    // set up poll vector to listen for incoming connections
    vector<struct pollfd> pollfds;
    pollfds.push_back({server_socket, POLLIN, 0});

    // add keyboard input to the poll vector
    pollfds.push_back({0, POLLIN, 0});

    // add udp listening socket to the poll vector (any incoming udp messages)
    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
        return -1;
    }

    // bind socket
    int udp_bind_status = bind(udp_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (udp_bind_status == -1) {
        return -1;
    }

    pollfds.push_back({udp_socket, POLLIN, 0});

    vector<user_data> connected_users;

    // map to store topic subscribers - we keep their IDs
    unordered_map<string, vector<int>> topic_subscribers;

    // deactivate NAGLE
    int flag = 1;
    int opt_result = setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, (void*) &flag, sizeof(int));
    if (opt_result < 0) {
        return -1;
    }

    while (true) {
        // poll for incoming connections
        poll(&pollfds[0], pollfds.size(), -1);
        if (pollfds[0].revents & POLLERR) {
            break;
        }

        // handle new connections
        if (pollfds[0].revents & POLLIN) {
            handle_new_connection(server_socket, pollfds, connected_users);
        }

        // handle keyboard input
        if (pollfds[1].revents & POLLIN) {
            int status = handle_keyboard_input(pollfds);
            if (status < 0) {
                break;
            }
        }

        // handle udp messages
        if (pollfds[2].revents & POLLIN) {
            handle_udp_message(udp_socket, pollfds, connected_users, topic_subscribers);
        }

        // handle all other sockets (TCP clients)
        for (size_t i = 3; i < pollfds.size(); i++) {
            if (pollfds[i].revents & POLLIN) {
                int client_socket = pollfds[i].fd;
                handle_tcp_message(client_socket, pollfds, connected_users, topic_subscribers);
            }
        }
    }
    // close all sockets
    for (size_t i = 0; i < pollfds.size(); i++) {
        close(pollfds[i].fd);
    }

    // close the server socket
    close(server_socket);

    return 0;
}
