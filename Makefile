WSC = Websocket/Datastructures.c Websocket/Errors.c Websocket/base64.c Websocket/md5.c Websocket/sha1.c Websocket/Communicate.c Websocket/utf8.c Websocket/Handshake.c

all: libpng/.libs/libpng16.a cJSON/libcjson.a
	node prebuild.js
	gcc -Iinclude -g Websocket.c fbmain.c core/png2raw.c libpng/.libs/libpng16.a cJSON/libcjson.a core/argv.c core/algorithms.c $(WSC) -pthread --static -lm -lz -o m
	rm -rf core/fbbuildinfo.h

libpng/.libs/libpng16.a:
	cd libpng;./configure --enable-static;make -j8

cJSON/libcjson.a:
	cd cJSON;cmake . -DBUILD_SHARED_AND_STATIC_LIBS=On;make -j8