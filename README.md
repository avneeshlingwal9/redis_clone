# REDIS CLONE USING C

It is a simple redis like key-value store. 

It uses event loop as a way to handle concurrent connections among different clients and uses epoll for the same. 

Provides support for `PING` , `GET`, `SET`, `ECHO` commands. 

The reading of RDB is also done. 

Leader follower mechanism is supported, in which a leader can have multiple followers and the handshaking is also supported.

The leader also propagates all the commands to its followers. 

For storing Key-Value pairs, `uthash` is used. 

The support for expiry of keys is also provided. 




