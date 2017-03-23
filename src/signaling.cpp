/*
 * misc.cpp
 *
 *  Created on: 26 févr. 2017
 *      Author: jfellus
 */

#include "signaling.h"
#include "pubsub.h"

#include <ddbus.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "channel.h"

const char* bus = "pubsub";

extern int DDBUS_DBG_LEVEL;

int ddbus_fd = -1;

void on_publish(const char* from, const char* channel) {
	Channel* c = get_channel(channel);
	if(!c) return;
	if(c->state != STATE_REQUESTED) return;
	c->on_publish(from);
}

void on_local_offer(const char* channel, const char* spec) {
	Channel* c = get_channel(channel);
	if(!c) return;
	if(c->state == STATE_PUBLISHED) return;
	c->on_local_offer(spec);
}

void on_remote_offer(const char* from, const char* channel, const char* spec) {
	Channel* c = get_channel(channel);
	if(!c) return;
	if(c->state == STATE_PUBLISHED) return;
	c->on_remote_offer(from, spec);
}

void on_request(const char* from, const char* channel) {
	Channel* c = get_channel(channel);
	if(!c) return;
	if(c->state != STATE_PUBLISHED) return;
	c->on_request(from);
}

void on_info(const char* from, const char* channel, const char* info) {
	Channel* c = get_channel(channel);
	if(!c) return;
	char* what = (char*)info;
	char* value = strchr(what, '=');
	*value = 0; value++;
	c->on_info(what, value);
}

////////////////


void broadcast(const char* fmt, ...) {
	if(ddbus_fd == -1) return;
	char s[1024];
	va_list vl;
	va_start(vl,fmt);
	vsprintf(s, fmt, vl);
	va_end(vl);
 	ddbus_write(ddbus_fd, s);
	fsync(ddbus_fd);
	if(DDBUS_DBG_LEVEL) printf("-> %s", s);
}


void on_ddbus_message(const char* from, const char* what, const char* content) {
	if(DDBUS_DBG_LEVEL) printf("<- %s : %s : %s\n", from, what, content);
	if(!strcmp(what, "PUBLISH")) {
		char* channel = (char*)content;
		on_publish(from, channel);
	} else if(!strcmp(what, "LOCAL_OFFER")) {
		if(strcmp(from, "localhost")) return;
		char* channel = (char*)content;
		char* spec = strchr(channel, '=');
		*spec = 0; spec++;
		on_local_offer(channel, spec);
	}
	else if(!strcmp(what, "REMOTE_OFFER")) {
		if(!strcmp(from, "localhost")) return;
		char* channel = (char*)content;
		char* spec = strchr(channel, '=');
		*spec = 0; spec++;
		on_remote_offer(from, channel, spec);
	}
	else if(!strcmp(what, "REQUEST")) on_request(from, content);
	else if(!strcmp(what, "INFO")) {
		char* channel = (char*)content;
		char* info = strchr(channel, ':');
		*info = 0; info++;
		on_info(from, channel, info);
	}
}

static void on_ddbus_message(const char* from, const char* msg) {
	if(!from || !msg || !*msg) return;
	char* what = (char*)msg;
	char* content = strchr(what, ':');
	if(content) { *content = 0; content++; }
	on_ddbus_message(from, what, content);
}


/////////


static bool bInited = false;
void pubsub_init() {
	if(bInited) return;
	bInited = true;
	ddbus_fd = ddbus_open(bus, on_ddbus_message);
}
