/*
 * shm.cpp
 *
 *  Created on: 26 févr. 2017
 *      Author: jfellus
 */




#include "shm.h"
#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>

extern void on_channel_recv(int channel, const char* buf, size_t len);
extern void broadcast_channel_info(int channel, const char* fmt, ...);


void* thread_shm_connection(void* _p) { ((SHMConnection*)_p)->run(); delete (SHMConnection*)_p; return 0;}
void* thread_shm_server(void* _p) { ((SHMServer*)_p)->run(); return 0;}


SHMServer::SHMServer(const char* shmpath, int channel) : shmpath(shmpath), channel(channel) {
	create_shm(shmpath);
	create_sync(shmpath);
}

void SHMServer::create_shm(const char* shmpath) {
	char _shmpath[256];
	sprintf(_shmpath, "/%s", shmpath);
	shm_fd = shm_open(_shmpath, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {fprintf(stderr, "Couldn't create shm segment %s\n", _shmpath); return; }
}

void SHMServer::set_size(size_t size) {
	if(apply_size(size)) broadcast_channel_info(channel, "SIZE=%lu\n", size);
}

bool SHMServer::apply_size(size_t size) {
	if(this->size == size) return false;
	if(ptr) munmap(ptr, size);
	this->size = size;
	ftruncate(shm_fd, size);

	ptr = (char*) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) { ptr = 0; fprintf(stderr, "mmap failed for %s (%s)\n", shmpath.c_str(), strerror(errno)); return false; }
	return true;
}

void SHMServer::create_sync(const char* shmpath) {
	struct sockaddr_un addr;
	int cl,rc;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	sprintf(addr.sun_path, "/run/lock/%s.sock", shmpath);
	unlink(addr.sun_path);

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) fprintf(stderr, "Can't bind to /run/lock/%s.sock (%s)\n", shmpath, strerror(errno));
	listen(sock, 20);
	pthread_create(&th, NULL, thread_shm_server, this);
}

void SHMServer::run() {
	while(1) {
		int fd = accept(sock, NULL, NULL);
		if(fd==-1) fprintf(stderr, "Error in accept SHMServer\n");
		else connections.push_back(new SHMConnection(fd, this));
	}
}

SHMConnection::SHMConnection(int channel, const char* shmpath) : shmpath(shmpath), channel(channel) {
	ptr = 0;
	open_shm(shmpath);
	open_sync(shmpath);
}

SHMConnection::~SHMConnection() {
	close(fd);
}

void SHMConnection::open_shm(const char* shmpath) {
	char _shmpath[256];
	sprintf(_shmpath, "/%s", shmpath);
	shm_fd = shm_open(_shmpath, O_RDWR, 0666);
	if (shm_fd == -1) { fprintf(stderr, "Couldn't open shm %s (%s)\n", _shmpath, strerror(errno)); }
}

void SHMConnection::set_size(size_t size) {
	apply_size(size);
}

bool SHMConnection::apply_size(size_t size) {
	if(this->size == size) return false;

	if(ptr) munmap(ptr, size);
	this->size = size;
	ftruncate(shm_fd, size);
	ptr = (char*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) { ptr = 0; fprintf(stderr, "mmap failed for %s (%s)\n", shmpath.c_str(), strerror(errno));	return false;  }
	return true;
}

void SHMConnection::open_sync(const char* shmpath) {
	fd = socket(AF_UNIX, SOCK_STREAM, 0);

	struct sockaddr_un addr;
	bzero((char *) &addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	sprintf(addr.sun_path, "/run/lock/%s.sock", shmpath);

    if (connect(fd, (const sockaddr*)&addr, sizeof(addr)) < 0) fprintf(stderr, "ERROR connecting to /run/lock/%s.sock (%s)\n", shmpath, strerror(errno));
    else {
    	pthread_create(&th, NULL, thread_shm_connection, this);
    }
}

SHMConnection::SHMConnection(int fd, SHMServer* server) : server(server), fd(fd) {
	ptr = 0;
	if(server) channel = server->channel;
	pthread_create(&th, NULL, thread_shm_connection, this);
}

void SHMConnection::run() {
	int n;
	size_t s;
	if(server) {
		while( (n = recv(fd, &s, sizeof(size), 0)) > 0) {
			server->apply_size(s);
			on_recv(server->ptr, s);
		}
		server->remove_connection(this);
	} else {
		while( (n = recv(fd, &s, sizeof(size), 0)) > 0) {
			apply_size(s);
			on_recv(ptr, s);
		}
	}
}

void SHMConnection::write(const char* buf, size_t len) {
	set_size(len);
	if(ptr) memcpy(ptr, buf, len);
	write();
}

void SHMConnection::write() {
	if(server) send(fd, &server->size, sizeof(server->size), 0);
	else send(fd, &size, sizeof(size), 0);
	fsync(fd);
}

void SHMConnection::on_recv(const char* buf, size_t len) {
	on_channel_recv(channel, buf, len);
}

void SHMServer::remove_connection(SHMConnection* c) {
	connections.erase(std::remove(connections.begin(), connections.end(), c), connections.end());
}

