/*
 * shm.h
 *
 *  Created on: 26 févr. 2017
 *      Author: jfellus
 */

#ifndef SHM_H_
#define SHM_H_

#include <vector>
#include <string>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>

using std::string;
using std::vector;

class SHMServer;

class SHMConnection {
public:
	int fd;
	int channel;
	SHMServer* server;
	string shmpath;
	pthread_t th;
	int shm_fd;
	char* ptr;
	size_t size;

	SHMConnection(int channel, const char* shmpath);
	SHMConnection(int fd, SHMServer* server = 0);
	~SHMConnection();

	void run();

	void write(const char* buf, size_t len);
	void write();

	void on_recv(const char* buf, size_t len);

	void set_size(size_t size);
	bool apply_size(size_t size);

private:
	void open_sync(const char* shmpath);
	void open_shm(const char* shmpath);
};

class SHMServer {
public:
	pthread_t th;
	vector<SHMConnection*> connections;
	string shmpath;
	int channel;
	int sock;
	int shm_fd;
	char* ptr;
	size_t size;

	SHMServer(const char* shmpath, int channel);
	~SHMServer() {}

	void set_size(size_t size);
	bool apply_size(size_t size);

	void run();

	void write(const char* buf, size_t len) {
		set_size(len);
		if(buf!=ptr) memcpy(ptr, buf, len);
		for(int i=0; i<connections.size(); i++) connections[i]->write();
	}

	void write() {
		for(int i=0; i<connections.size(); i++) connections[i]->write();
	}

	void remove_connection(SHMConnection* c);

private:
	void create_sync(const char* shm_path);
	void create_shm(const char* shm_path);
};




#endif /* SHM_H_ */
