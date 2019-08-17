#include "packetlossresolver.h"


void PLResolver::addCommand(const std::string cmd,const std::string uuid){
	pkmutex.lock();
	auto fr=pkmap.find(uuid);
	pkmap[uuid]=cmd;
	pkmutex.unlock();
}

bool PLResolver::removeCommand(const std::string uuid){
	pkmutex.lock();
	auto fresult=pkmap.find(uuid);
	if(fresult==pkmap.end()){
		pkmutex.unlock();
		return false;
	}
	pkmap.erase(fresult->first);
	pkmutex.unlock();
	return true;
}

bool PLResolver::resendCommand(const std::string uuid){
	pkmutex.lock();
	auto fresult=pkmap.find(uuid);
	if(fresult==pkmap.end()){
		pkmutex.unlock();
		return false;
	}
	session->sendCommand(pkmap[uuid],uuid);
	pkmutex.unlock();
	return true;
}

void PLResolver::clear(){
	pkmutex.lock();
	unsigned int show=0;
	for(auto i:pkmap){
		session->sendCommand(i.second,i.first);
		show++;
	}
	if(show!=0)std::cout<<"[PLResolver|Trace] Still have "<<show<<" packets to send";
	pkmap.clear();
	pkmutex.unlock();
}