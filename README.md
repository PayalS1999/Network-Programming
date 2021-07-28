# Network-Programming

Socket programming is a way of connecting two nodes on a network to communicate with each other. One socket(node) listens on a particular port at an IP, while other socket reaches out to the other to form a connection. Server forms the listener socket while client reaches out to the server
This is a multi-client, single server application program, where multiple clients ( after  establishing connection with the server) can chat with each other personally or form groups or broadcast messages. ( Over UDP connection)
The clients can transfer text files ( to server or other clients) using TCP.
Various methods, like forking and select (in-built functions) have been used
