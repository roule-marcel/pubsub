/*
 * subscribe.cpp
 *
 *  Created on: 26 févr. 2017
 *      Author: jfellus
 */

#include <pubsub.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

unsigned long start_time;
size_t s = 0;

unsigned long get__time() {
	struct timeval tv;
	struct timezone tzp;
	if(gettimeofday(&tv, &tzp) == -1) fprintf(stderr, "ERROR %s\n", strerror(errno));
	unsigned long ff = tv.tv_sec * 1000 + tv.tv_usec/1000;
	return ff;
}

void print_size(float s) {
	if(s<1024) { printf("%.2f o/s\n", s); return; }
	s/=1024;
	if(s<1024) { printf("%.2f Ko/s\n", s); return; }
	s/=1024;
	if(s<1024) { printf("%.2f Mo/s\n", s); return; }
	s/=1024;
	if(s<1024) { printf("%.2f Go/s\n", s); return; }
	s/=1024;
	printf("%.2f To/s\n", s);
}

void on_recv(const char* buf, size_t len) {
	s+=len;
	float d = (float)(get__time() - start_time)/1000.0f;
	if(d >= 1) {
    	s/=d;
    	print_size(s);
    	s = 0;
    	start_time = get__time();
    }
}

int main(int argc, char **argv) {
	start_time = get__time();
	char buf[1024];
	Subscription* c = subscribe("monitor", on_recv);
	sleep(100);
}
