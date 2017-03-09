/*
 * channel.h
 *
 *  Created on: 26 févr. 2017
 *      Author: jfellus
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_


#include <string>
#include "tcp.h"
#include "shm.h"
#include "signaling.h"


class Channel;

#define STATE_NOTHING -1
#define STATE_REQUESTED 0
#define STATE_KNOWN 1
#define STATE_SUBSCRIBED 2
#define STATE_PUBLISHED 3

using std::string;


class Subscription {
public:
	Channel* channel;
	void (*on_recv)(const char* buf, size_t len);

	Subscription(Channel* c, void (*on_recv)(const char* buf, size_t len)) : channel(c), on_recv(on_recv) {}
	~Subscription() {}

	// Copy-based write
	void write(const char* buf, size_t len);

	// In-place write
	char* ptr();
	void set_size(size_t size);
	size_t get_size();
	void write();
};

class Channel {
public:
	string name;
	int state;
	int fd;
	size_t _size;

	string host;
	int tcp_port;

	TCPServer* tcp_server;
	TCPConnection* tcp_connection;

	SHMServer* shm_server;
	SHMConnection* shm_connection;

	char* local_offer;
	char* remote_offer;

	void (*cb_on_recv)(const char* buf, size_t len);

	vector<Subscription*> subscriptions;


	// Construction

	Channel(const char* name);
	~Channel() {}

	// Accessors

	void set_size(size_t s) {
		if(this->_size == s) return;
		this->_size = s;
		if(shm_server) shm_server->set_size(_size);
		if(shm_connection) shm_connection->set_size(_size);
	}

	bool is_local() { return host.empty();	}

	bool operator==(const char* channel) { return name == channel; }

	const char* get_name() { return name.c_str(); }


	// Connection

	void connect() {
		if(state != STATE_KNOWN) return;
		if(is_local()) connect_shm();
		else connect_tcp();
	}

	void connect_shm() {
		if(shm_connection) return;
		shm_connection = new SHMConnection(fd, get_name());
	}

	void connect_tcp() {
		if(tcp_connection) return;
		tcp_connection = new TCPConnection(fd, host.c_str(), tcp_port);
	}


	// Offer

	const char* create_local_offer() {
		if(!local_offer) {
			local_offer = (char*) name.c_str();
			shm_server = new SHMServer(local_offer, fd);
		}
		return local_offer;
	}

	const char* create_remote_offer() {
		if(!remote_offer) {
			create_local_offer(); // Needed to create internal memory for inplace access
			remote_offer = new char[128];
			tcp_server = new TCPServer(fd);
			tcp_port = tcp_server->port;
			sprintf(remote_offer, "%u", tcp_port);
		}
		return remote_offer;
	}

	void request() {
		broadcast("REQUEST:%s\n", get_name());
	}

	void on_local_offer(const char* spec) {
		if(state >= STATE_KNOWN) return;
		if(state == -1) return;
		state = STATE_KNOWN;
		host = "";
		connect();
	}

	void on_remote_offer(const char* from, const char* spec) {
		if(state >= STATE_KNOWN) return;
		if(state == -1) return;
		state = STATE_KNOWN;
		host = from;
		tcp_port = atoi(spec);
		connect();
	}

	void on_publish(const char* from) {
		request();
	}

	void on_request(const char* host);

	void on_info(const char* what, const char* value) {
		if(!strcmp(what, "SIZE")) on_set_size((size_t)atol(value));
	}

	void on_set_size(size_t size) {
		_size = size;
		if(shm_connection) shm_connection->apply_size(size);
		if(shm_server) shm_server->apply_size(size);
	}


	// Subscription

	Subscription* subscribe(void (*on_recv)(const char* buf, size_t len)) {
		Subscription* s = new Subscription(this, on_recv);
		subscriptions.push_back(s);
		connect();
		return s;
	}

	void remove_subscription(Subscription* s);

	void subscription_write(const char* buf, size_t len) {
		set_size(len);
		if(tcp_connection) tcp_connection->write(buf, len);
		if(shm_connection) shm_connection->write(buf, len);
	}

	void subscription_write() {
		if(!ptr()) return;
		if(tcp_connection) tcp_connection->write(ptr(), size());
		if(shm_connection) shm_connection->write();
	}


	// Callback

	void on_recv(const char* buf, size_t len) {
		if(cb_on_recv) cb_on_recv(buf, len);
		for(int i=0; i<subscriptions.size(); i++) {
			if(subscriptions[i]->on_recv) subscriptions[i]->on_recv(buf, len);
		}
	}


	// Write

	/** Copy-based write */
	void write(const char* buf, size_t len) {
		set_size(len);
		if(shm_server) shm_server->write(buf, len);
		if(tcp_server) tcp_server->write(buf, len);
	}

	/** In-place write */
	void write() {
		if(!ptr()) return;
		if(shm_server) shm_server->write();
		if(tcp_server) tcp_server->write((const char*)ptr(), size());
	}

	/** @return a pointer to the channel's internal memory */
	char* ptr() {
		if(shm_server) return shm_server->ptr;
		if(shm_connection) return shm_connection->ptr;
		if(tcp_connection) return tcp_connection->ptr;
		return 0;
	}

	/** @return the size of the channel's internal memory */
	size_t size() { return _size; }

};


Channel* add_channel(const char* name);
Channel* get_channel(const char* name);


#endif /* CHANNEL_H_ */
