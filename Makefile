WSC = Websocket/Datastructures.c Websocket/Errors.c Websocket/base64.c Websocket/md5.c Websocket/sha1.c Websocket/Communicate.c Websocket/utf8.c Websocket/Handshake.c

all:
	node prebuild.js
	gcc -Iinclude -g Websocket.c fbmain.c cJSON/libcjson.a core/argv.c core/algorithms.c $(WSC) -pthread --static -lm -o m
	rm -rf core/fbbuildinfo.h