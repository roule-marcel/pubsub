/*
 * test.c
 *
 *  Created on: 25 févr. 2017
 *      Author: jfellus
 */

#include <pubsub.h>
#include <string.h>
#include <stdio.h>


int main(int argc, char **argv) {
	size_t size = 1000000;
	if(argc>=2) size = atoi(argv[1]);
	Channel* c = publish("monitor", NULL);
	char* buf = new char[size];
	while(true) {
		if(c->ptr()) buf = c->ptr();
		c->write(buf, size);
	}
}
