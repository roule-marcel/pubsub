SOURCES:=$(shell find src/ -maxdepth 1 -type f -name '*.cpp')
OBJECTS:=$(SOURCES:src/%.cpp=bin/%.o)

all: libpubsub.so publish subscribe subscribe-async

install:
	cp publish /usr/bin
	cp subscribe /usr/bin
	cp libpubsub.so /usr/lib

libpubsub.so: $(OBJECTS)
	g++ -g -shared -o $@ $^ -pthread -L. -lddbus -lrt
	
publish: bin/test/publish.o
	g++ -g  -o $@ $^ -pthread -lpubsub -L.

subscribe: bin/test/subscribe.o
	g++ -g  -o $@ $^ -pthread -lpubsub -L.

subscribe-async: bin/test/subscribe-async.o
	g++ -g  -o $@ $^ -pthread -lpubsub -L.

bin/%.o: src/%.cpp src/*.h
	mkdir -p `dirname $@`
	g++ -g  -fPIC -o $@ -c $< -I./src
	
clean:
	rm -rf bin
	rm -f *.so
	rm -f publish
	rm -f subscribe