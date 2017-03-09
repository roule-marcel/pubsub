# Workflow

## @ Publisher

* publish(name, cb)
* create channel object if not exists
* register cb
* state = STATE_PUBLISHED
* broadcast(PUBLISH:name)

#### On REQUEST message 

* get_channel(name)
* state == STATE_PUBLISHED ? -> yes
* c->on_request
	* local ? -> create_local_offer -> SHMServer -> offer=shm_name -> broadcast(LOCAL_OFFER:name=offer)
	* remote ? -> create_remote_offer -> TCPServer -> offer=tcp_port -> broadcast(REMOTE_OFFER:name=offer)


## @ Subscriber

* subscribe(name, cb)
* create channel object if not exists -> state = STATE_REQUESTED
* if not known -> broadcast(REQUEST:name)
* c->subscribe(cb) -> new Subscription(cb) 
* if known -> 
	* if local -> connect_shm
	* if not local -> connect_tcp -> if not connected -> TCPConnection(host,port)
 
* write(buf,len) -> channel.write(buf,len) -> 
	* TCPConnection.write
	* SHMConnection.write

* on_recv -> on_channel_recv(c, buf, len) -> channel.on_recv(buf,len)
	* if cb_on_recv -> cb_on_recv(buf, len)
	* all subscriptions.on_recv(buf, len)

#### On OFFER message

* if local 
	* on_local_offer(channel, spec) -> if not known -> state = STATE_KNOWN -> connect
	* on_remote_offer(channel, spec) -> if not known -> state = STATE_KNOWN -> tcp_port = spec, host = from -> connect
* connect()	
* if known -> 
	* if local -> connect_shm
	* if not local -> connect_tcp -> if not connected -> TCPConnection(host,port)
 