/*
 * tcp.h
 *
 *  Created on: 25 févr. 2017
 *      Author: jfellus
 */

#ifndef TCP_H_
#define TCP_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>

using std::vector;



class TCPServer;


class TCPConnection {
public:
	int fd;
	pthread_t th;
	TCPServer* server;
	int channel;
	char* ptr;

	TCPConnection(int channel, const char* host, int port);

	TCPConnection(int fd, TCPServer* server = 0);

	void run();

	void write(const char* buf, size_t len) {
		::write(fd, buf, len);
	}

	void on_recv(const char* buf, size_t len);
};



class TCPServer {
public:
	int sock;
	int port;
	vector<TCPConnection*> connections;
	pthread_t th;
	int channel;

	TCPServer(int channel);

	void run();

	void write(const char* msg, size_t len) {
		for(int i=0; i<connections.size(); i++) connections[i]->write(msg, len);
	}

	void remove_connection(TCPConnection* c);
};




#endif /* TCP_H_ */
