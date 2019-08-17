#pragma once
#include <map>
#include <iostream>
#include <mutex>
#include "../fbmain.h"

class FastBuilderSession;

class PLResolver {
private:
	std::mutex pkmutex;
	std::map<std::string,std::string> pkmap;
public:
	FastBuilderSession *session;
	PLResolver(FastBuilderSession *ss):session(ss){}
	void addCommand(const std::string cmd,const std::string uuid);
	bool removeCommand(const std::string uuid);
	bool resendCommand(const std::string uuid);
	void clear();
};