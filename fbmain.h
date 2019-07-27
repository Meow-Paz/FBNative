#pragma once
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <map>
#include "core/algorithms.h"
#include <pthread.h>

class Algorithms;

class FastBuilderSession {
private:
	std::map<std::string,std::string> packetsMap;
	pthread_mutex_t FSTMutex=PTHREAD_MUTEX_INITIALIZER;
	void *sock;
	Algorithms *algorithms;

	void errResend(std::string msgid);
public:
	bool busy;
	pthread_t busythr;
	void killbusy();
	void sendText(std::string text);
	void subscribe(const std::string eventN);
	void sendCommand(const std::string cmd,std::string uuid="null");
	const std::string sendCommandSync(const std::string cmd);
	FastBuilderSession(void *_sock);
	~FastBuilderSession();
	void onMsg(std::string msg);
	void send(void *cl,std::string msg);
};