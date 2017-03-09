/*
 * channel.cpp
 *
 *  Created on: 26 févr. 2017
 *      Author: jfellus
 */


#include "channel.h"
#include <vector>
#include <algorithm>
#include <stdarg.h>

using std::vector;

extern void broadcast_channel_info(int channel, const char* fmt, ...);


int FDS = 0;


/////////////

vector<Channel*> channels;

Channel* add_channel(const char* name) {
	Channel* c = new Channel(name);
	channels.push_back(c);
	return c;
}

Channel* get_channel(const char* name) {
	for(int i=0; i<channels.size(); i++) if(channels[i]->name == name) return channels[i];
	return NULL;
}

void remove_channel(const char* name) {
	Channel* c = get_channel(name);
	if(!c) return;
	delete(channels[c->fd]);
	channels[c->fd] = 0;
}

/////////////
// CHANNEL //
/////////////

Channel::Channel(const char* name) : name(name) {
	_size = 0;
	cb_on_recv = 0;
	state = -1;
	tcp_server = 0;
	tcp_port = -1;
	local_offer = 0;
	remote_offer = 0;
	tcp_connection = 0;
	shm_connection = 0;
	shm_server = 0;
	fd = FDS++;
}


void Channel::remove_subscription(Subscription* s) {
	subscriptions.erase(std::remove(subscriptions.begin(), subscriptions.end(), s), subscriptions.end());
}


//////////

void Channel::on_request(const char* host) {
	if(!strcmp(host, "localhost")) broadcast("LOCAL_OFFER:%s=%s\n", get_name(), create_local_offer());
	else broadcast("REMOTE_OFFER:%s=%s\n", get_name(), create_remote_offer());
	if(shm_server && shm_server->size) broadcast_channel_info(fd, "SIZE=%lu\n", shm_server->size);
}


//////////////////
// SUBSCRIPTION //
//////////////////

void Subscription::write(const char* buf, size_t len) {
	channel->subscription_write(buf, len);
}

void Subscription::write() {
	channel->subscription_write();
}

char* Subscription::ptr() {
	return channel->ptr();
}

size_t Subscription::get_size() {
	return channel->size();
}

void Subscription::set_size(size_t size) {
	channel->set_size(size);
}


//////////////////////
// GLOBAL CALLBACKS //
//////////////////////

void on_channel_recv(int channel, const char* buf, size_t len) {
	Channel* c = channels[channel];
	c->on_recv(buf, len);
}

void broadcast_channel_info(int channel, const char* fmt, ...) {
	Channel* c = channels[channel];
	if(!c) return;
	char s[1024];
	char s2[1024];
	va_list vl;
	va_start(vl,fmt);
	vsprintf(s, fmt, vl);
	va_end(vl);
	sprintf(s2, "INFO:%s:%s", c->get_name(), s);
	broadcast(s2);
}
