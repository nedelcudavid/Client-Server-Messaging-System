# My Octave
## Copyright 2022 Nedelcu Andrei-David
___________________________________________________________________________________________

### Description

This project represents the implementation of a messaging system, in which
clients (TCP) subscribe to a topic*, and then the server sends
to these subscribers who are connected messages on that topic transmitted by UDP clients*, messages which can have different data types*.

* *TCP clients – connect to the server and can subscribe/unsubscribe from a topic
* *UDP clients – send messages on a topic
* *Topic – a string that can have the form: topic1/topic2/..., and can include wildcard characters: * (can be replaced with 0 or any number of topic levels) and + (can be replaced with exactly one topic level)
* *Data types – messages can be of type: INT, SHORT_REAL, FLOAT, STRING

### Implementation Details

#### Server

The server initializes its sockets: TCP listener, UDP listener, and stdin, in a vector of pollfd structures. It listens for new TCP connections, adding them to the TCP clients list, and receives messages from UDP clients, which it sends to the TCP clients subscribed to that topic.

The server maintains connected clients in a vector of user_data structures, and the topics and the IDs of subscribed clients in an unordered_map (hash table).

#### TCP Client

The TCP client connects to the server and validates the ID given to the server (unique).
Then, it can send subscribe/unsubscribe commands to a topic, or disconnect.
The client receives messages from the server and displays them in the format: topic - type - data.

It also uses a vector of pollfd structures to listen to both the server and stdin.

#### TCP Communication Protocol

In order to communicate between TCP client and server, we use a structure of type tcp_comm, which contains the message type (INIT, TOPIC, SUBSCRIBE, UNSUBSCRIBE), the field id_taken (to validate the client ID), the field ID (for initializing a client), the field topic (to specify the topic to which the client subscribes/unsubscribes), and the field message for the case when a message is sent from the server to the client, redirected from UDP clients.

### Conclusions

The project focused on the approach of a complex system, and it was a challenge to manage so many connections and messages coming and going from different sides.

Considering the improvements I could bring, I could approach in a different way the storage of topics so that I can make more efficient searches, as well as the connected users.
Avand in vedere imbunatatirile pe care le-as putea aduce, as putea sa abordez intr-o
maniera diferita stocarea topic-urilor astfel incat sa pot face cautari mai eficiente,
respectiv a userilor conectati.
