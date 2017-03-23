/*
 * pubsub.c
 *
 *  Created on: 25 févr. 2017
 *      Author: jfellus
 */

#include "pubsub.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "signaling.h"
#include "channel.h"


int DDBUS_DBG_LEVEL = 0;

Channel* request(const char* channel) {
	Channel* c = get_channel(channel);
	if(!c) {
		c = add_channel(channel);
		c->state = STATE_REQUESTED;
	}
	if(c->state < STATE_KNOWN) c->request();
	return c;
}

Channel* publish(Channel* c, void (*on_recv)(const char* buf, size_t len)) {
	c->cb_on_recv = on_recv;
	c->state = STATE_PUBLISHED;
	broadcast("PUBLISH:%s\n", c->get_name());
	return c;
}



/////


#ifdef __cplusplus
extern "C" {
#endif

Channel* publish(const char* channel, void (*on_recv)(const char* buf, size_t len)) {
	pubsub_init();
	Channel* c = get_channel(channel);
	if(c) return c; // channel already exists
	return publish(add_channel(channel), on_recv);
}

Subscription* subscribe(const char* channel, void (*on_recv)(const char* buf, size_t len)) {
	pubsub_init();
	Channel* c = request(channel);
	Subscription* s = c->subscribe(on_recv);
	return s;
}


#ifdef __cplusplus
}
#endif
