/*
 * test.c
 *
 *  Created on: 25 févr. 2017
 *      Author: jfellus
 */

#include <pubsub.h>
#include <string.h>
#include <stdio.h>

void on_recv(const char* buf, size_t len) {
	fwrite(buf, 1, len, stdout);
	fflush(stdout);
}

int main(int argc, char **argv) {
	char buf[1024];
	Channel* c = publish(argv[1], on_recv);
	while(fgets(buf, 1024, stdin)) {
		c->write(buf, strlen(buf));
	}
}
