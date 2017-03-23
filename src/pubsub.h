/*
 * pubsub.h
 *
 *  Created on: 25 févr. 2017
 *      Author: jfellus
 */

#ifndef PUBSUB_H_
#define PUBSUB_H_

#include "channel.h"


#ifdef __cplusplus
extern "C" {
#endif

Channel* publish(const char* channel, void (*on_recv)(const char* buf, size_t len));

Subscription* subscribe(const char* channel, void (*on_recv)(const char* buf, size_t len) = 0);


extern int DDBUS_DBG_LEVEL;

#ifdef __cplusplus
}
#endif


#endif /* PUBSUB_H_ */
