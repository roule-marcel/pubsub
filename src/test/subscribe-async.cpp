/*
 * subscribe-async.cpp
 *
 *  Created on: 26 févr. 2017
 *      Author: jfellus
 */

#include <pubsub.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

pthread_t th;
void* read_async(void* _sub) {
	Subscription* s = (Subscription*)_sub;
	while(true) {
		if(s->ptr() && s->get_size())
			fwrite(s->ptr(), 1, s->get_size(), stdout);
		fflush(stdout);
		sleep(1);
	}
}

int main(int argc, char **argv) {
	char buf[1024];
	Subscription* c = subscribe(argv[1]);
	pthread_create(&th, NULL, read_async, c);
	while(fgets(buf, 1024, stdin)) {
		c->write(buf, strlen(buf));
	}
}
