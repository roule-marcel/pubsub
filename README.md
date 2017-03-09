# pubsub
A publish-subscribe communication mechanism with various backends (TCP, UDP, SHM, ...)

# How to build
````
make
sudo make install
````

# How to use

### Publish a channel "my_channel" and forward stdin to it, writing incoming data to stdout
````cpp
void on_recv(const char* buf, size_t len) {
	fwrite(buf, 1, len, stdout);
	fflush(stdout);
}

int main(int argc, char **argv) {
	char buf[1024];
	Channel* c = publish("my_channel", on_recv);
	while(fgets(buf, 1024, stdin)) {
		c->write(buf, strlen(buf));
	}
}
````

### Subscribe to a channel "my_channel" and forward stdin to it, writing incoming data to stdout
````cpp
void on_recv(const char* buf, size_t len) {
	fwrite(buf, 1, len, stdout);
	fflush(stdout);
}

int main(int argc, char **argv) {
	char buf[1024];
	Subscription* c = subscribe(argv[1], on_recv);
	while(fgets(buf, 1024, stdin)) {
		c->write(buf, strlen(buf));
	}
}
````

