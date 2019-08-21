.PHONY: all static shared
CPPFILES = -Iinclude -Ilibnbtplusplus -Ilibnbtplusplus/include -IuWebSockets/src -IuWebSockets/uSockets/src main.cpp core/argv.cpp core/log.cpp core/crash_handler.cpp core/fbscript.cpp core/algorithms.cpp core/fbws.cpp core/fbsynckeeper.cpp core/packetlossresolver.cpp


all: uWebSockets/uSockets/uSockets.a jsoncpp-1.8.4/src/lib_json/libjsoncpp.a libnbtplusplus/libnbt++.a
	node prebuild.js
	g++ -flto -Os -std=c++17 $(CPPFILES) libnbtplusplus/libnbt++.a uWebSockets/uSockets/uSockets.a jsoncpp-1.8.4/src/lib_json/libjsoncpp.a -lpng -lm -lz -ldl -luv -pthread -o m
	rm -rf core/fbbuildinfo.h
static: libpng/.libs/libpng16.a uWebSockets/uSockets/uSockets.a jsoncpp-1.8.4/src/lib_json/libjsoncpp.a libnbtplusplus/libnbt++.a
	node prebuild.js
	g++ -flto -Os -std=c++17 $(CPPFILES) libnbtplusplus/libnbt++.a uWebSockets/uSockets/uSockets.a jsoncpp-1.8.4/src/lib_json/libjsoncpp.a libpng/.libs/libpng16.a -lm -lz -ldl -luv -pthread -static -o m
	rm -rf core/fbbuildinfo.h
shared: uWebSockets/uSockets/uSockets.a libraries/libjsoncpp.so libraries/libnbt++.so
	node prebuild.js
	g++ -Os -std=c++17 $(CPPFILES) uWebSockets/uSockets/uSockets.a -Llibraries -lnbt++ -ljsoncpp -lpng -lm -lz -ldl -luv -pthread -shared -D shared=true -o libfastbuildern.so
	rm -rf core/fbbuildinfo.h
smallest: uWebSockets/uSockets/uSockets.a libraries/libjsoncpp.so libraries/libnbt++.so
	node prebuild.js
	g++ -Os -std=c++17 $(CPPFILES) uWebSockets/uSockets/uSockets.a -Llibraries -lnbt++ -ljsoncpp -lpng -lm -lz -ldl -luv -pthread -o m
	rm -rf core/fbbuildinfo.h

libpng/.libs/libpng16.a:
	cd libpng;./configure --enable-static;make -j8
uWebSockets/uSockets/uSockets.a:
	cd uWebSockets;make -j8
jsoncpp-1.8.4/src/lib_json/libjsoncpp.a:
	cd jsoncpp-1.8.4;cmake .;make -j8
libnbtplusplus/libnbt++.a:
	cd libnbtplusplus;cmake -DNBT_USE_ZLIB=ON .;make -j8
libraries/libnbt++.so:
	cd libnbtplusplus;cmake -DNBT_BUILD_SHARED=ON -DNBT_USE_ZLIB=ON .;make -j8
	cp libnbtplusplus/libnbt++.so libraries/libnbt++.so
libraries/libjsoncpp.so:
	cd jsoncpp-1.8.4;cmake -DBUILD_SHARED_LIBS=ON .;make -j8
	cp jsoncpp-1.8.4/src/lib_json/libjsoncpp.so* libraries/
