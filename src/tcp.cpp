/*
 * tcp.cpp
 *
 *  Created on: 26 févr. 2017
 *      Author: jfellus
 */


#include "tcp.h"
#include <pthread.h>
#include <algorithm>
#include <stdio.h>
#include <errno.h>


extern void on_channel_recv(int channel, const char* buf, size_t len);


void* thread_tcp_connection(void* _p) { ((TCPConnection*)_p)->run(); delete (TCPConnection*)_p; return 0;}
void* thread_tcp_server(void* _p) { ((TCPServer*)_p)->run(); return 0;}


TCPConnection::TCPConnection(int channel, const char* host, int port) : channel(channel) {
	fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	bzero((char *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_aton(host, &addr.sin_addr);
	addr.sin_port = htons(port);

	ptr = new char[1024];

    if (connect(fd, (const sockaddr*)&addr, sizeof(addr)) < 0) fprintf(stderr, "ERROR connecting to %s:%d : %s\n", host, port, strerror(errno));
    else pthread_create(&th, NULL, thread_tcp_connection, this);
}

TCPConnection::TCPConnection(int fd, TCPServer* server) : server(server), fd(fd) {
	if(server) channel = server->channel;
	ptr = new char[1024];
	pthread_create(&th, NULL, thread_tcp_connection, this);
}

void TCPConnection::run() {
	int n;

	while( (n = read(fd, ptr, 1024)) > 0) on_recv(ptr, n);
	if(server) server->remove_connection(this);
}

void TCPConnection::on_recv(const char* buf, size_t len) {
	on_channel_recv(channel, buf, len);
}




TCPServer::TCPServer(int channel) : channel(channel) {
    struct sockaddr_in addr;
    socklen_t addrLen;

    port = 10000;
    do {
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == -1) { fprintf(stderr, "Failed to create socket\n"); return; }
		int optval = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

		addr.sin_family = AF_INET;
		addr.sin_port = htons((unsigned short)port);
		addr.sin_addr.s_addr = INADDR_ANY;
		if (bind(sock, (const struct sockaddr *)&addr, sizeof(addr)) != -1) break;

		close(sock);
		sock = -1;
		port++;
    } while(true);

	listen(sock, 5);
	pthread_create(&th, NULL, thread_tcp_server, this);
}

void TCPServer::run() {
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	while(1) {
		int fd = accept(sock, (struct sockaddr*)&addr, &len);
		if(fd == -1) { fprintf(stderr, "ERROR in TCPServer : %s\n", strerror(errno)); break; }
		connections.push_back(new TCPConnection(fd, this));
	}
}

void TCPServer::remove_connection(TCPConnection* c) {
	connections.erase(std::remove(connections.begin(), connections.end(), c), connections.end());
}
