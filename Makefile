#WSC = Websocket/Datastructures.c Websocket/Errors.c Websocket/base64.c Websocket/md5.c Websocket/sha1.c Websocket/Communicate.c Websocket/utf8.c Websocket/Handshake.c

all: libpng/.libs/libpng16.a uWebSockets/uSockets/uSockets.a jsoncpp-1.8.4/src/lib_json/libjsoncpp.a libnbtplusplus/libnbt++.a
	node prebuild.js
	g++ -g -flto -O9 -std=c++17 -Iinclude -Ilibnbtplusplus -Ilibnbtplusplus/include -IuWebSockets/src -IuWebSockets/uSockets/src main.cpp core/argv.cpp core/memorycontroller.cpp core/crash_handler.cpp core/fbscript.cpp core/algorithms.cpp core/fbws.cpp core/fbsynckeeper.cpp libnbtplusplus/libnbt++.a uWebSockets/uSockets/uSockets.a jsoncpp-1.8.4/src/lib_json/libjsoncpp.a libpng/.libs/libpng16.a -lm -lz -ldl -luv -pthread -o m
	rm -rf core/fbbuildinfo.h

libpng/.libs/libpng16.a:
	cd libpng;./configure --enable-static;make -j8
uWebSockets/uSockets/uSockets.a:
	cd uWebSockets;make -j8
jsoncpp-1.8.4/src/lib_json/libjsoncpp.a:
	cd jsoncpp-1.8.4;cmake .;make -j8
libnbtplusplus/libnbt++.a:
	cd libnbtplusplus;cmake -DNBT_USE_ZLIB=ON .;make -j8
