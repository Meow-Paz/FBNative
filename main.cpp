#include <stdio.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <map>
#include <pthread.h>
#include "core/profile.h"
#include "core/algorithms.h"
#include "core/argv.h"
#include "core/fbsynckeeper.h"
#include "core/fbscript.h"
#include "jsoncpp-1.8.4/include/json/json.h"
#include <mdbg.h>
#include "core/crash_handler.h"
#include <App.h>
#include "fbmain.h"
#include <dirent.h>
#include <elf.h>

int isBuilding=0;
int allBlocks=0;
int doneBlocks=0;

Algorithms *algorithms;
std::map<std::string,std::string> packetsMap;
pthread_mutex_t FSTMutex=PTHREAD_MUTEX_INITIALIZER;


void _send(void *cl,std::string msg);
void closews(void *cl);

//#define send _send

char *random_uuid( char buf[37] )
{
	const char *c = "89ab";
	char *p = buf;
	int n;

	for( n = 0; n < 16; ++n )
	{
		int b = rand()%255;

		switch( n )
		{
			case 6:
				sprintf(
						p,
						"4%x",
						b%15 );
				break;
			case 8:
				sprintf(
						p,
						"%c%x",
						c[rand()%strlen( c )],
						b%15 );
				break;
			default:
				sprintf(
						p,
						"%02x",
						b );
				break;
		}

		p += 2;

		switch( n )
		{
			case 3:
			case 5:
			case 7:
			case 9:
				*p++ = '-';
				break;
		}
	}

	*p = 0;

	return buf;
}

void fbinitDone(void){
	printf("Server initialized.\n\n");
	//pthread_t thTT;
	//pthread_create(&thTT,NULL,(void *(*)(void*))FSTCleaner,NULL);
	//pthread_mutex_init(&FSTMutex,NULL);
}

void createWebsocketServer(unsigned short port);

class FastBuilderSession;
class FastBuilder {
public:
	bool shutting_down;
	Algorithms *algorithms;
	std::map<void *,FastBuilderSession*> FBSMap;

	FastBuilder(){
		shutting_down=false;
		std::cout<<Profile::getLogo();
		algorithms=new Algorithms();
		initFBS(algorithms);
		Profile::staticV12printInfo();
	}

	~FastBuilder(){
		shutting_down=true;
		std::cout<<"Shutting down..."<<std::endl;
		std::cout<<"Destroying algorithms..."<<std::endl;
		delete algorithms;
		std::cout<<"Destroying alived sessions..."<<std::endl;
		for(auto i:FBSMap){
			delete i.second;
		}
		std::cout<<"FastBuilder Destroyed"<<std::endl;
	}

};

FastBuilder *fastbuilder;

void FastBuilderSession::sendText(std::string text){
	std::string cmd;
	cmd=std::string("say §b")+text;
	sendCommand(cmd);
}

void FastBuilderSession::subscribe(const std::string eventN){
	char rUUID[37]={0};
	random_uuid(rUUID);
	Json::Value rootVal;
	Json::FastWriter fw;
	Json::Value header;
	Json::Value body;
	header["requestId"]=Json::Value(std::string(rUUID));
	header["messagePurpose"]=Json::Value("subscribe");
	header["version"]=Json::Value(1);
	header["messageType"]=Json::Value("commandRequest");
	rootVal["header"]=header;
	body["eventName"]=Json::Value(eventN);
	rootVal["body"]=body;
	send(sock,fw.write(rootVal));
	return;
}

void FastBuilderSession::sendCommand(const std::string cmd,std::string uuid){
	char oUUID[37]={0};
	std::string rUUID;
	char isATG=0;
	if(uuid=="null"){
		random_uuid(oUUID);
		rUUID=std::string(oUUID);
		isATG=1;
	}else{
		rUUID=uuid;
	}
	Json::Value packetj;
	Json::Value header;
	header["requestId"]=Json::Value(rUUID);
	header["messagePurpose"]=Json::Value("commandRequest");
	header["version"]=Json::Value(1);
	header["messageType"]=Json::Value("commandRequest");
	packetj["header"]=header;
	Json::Value body;
	Json::Value origin;
	origin["type"]=Json::Value("player");
	body["origin"]=origin;
	body["commandLine"]=Json::Value(cmd);
	body["version"]=Json::Value(1);
	packetj["body"]=body;
	Json::FastWriter fwt;
	send(sock,fwt.write(packetj));
	if(isATG){
		//pthread_mutex_lock(&FSTMutex);
		//packetsMap[rUUID]=cmd;
		//pthread_mutex_unlock(&FSTMutex);
	}
}

const std::string FastBuilderSession::sendCommandSync(const std::string cmd){
	char rUUID[37]={0};
	random_uuid(rUUID);
	sendCommand(cmd,std::string(rUUID));
	setNMark(std::string(rUUID));
	const char *result=nullptr;
	while((result=getUValue(std::string(rUUID)))==nullptr){usleep(100000);}
	return std::string(result);
}

FastBuilderSession::FastBuilderSession(void *_sock){
	busythr=(pthread_t)0;
	busy=true;
	algorithms=fastbuilder->algorithms;
	sock=_sock;
	std::cout<<"A client connected."<<std::endl;
	subscribe("PlayerMessage");
	char vst[25]={0};
	sprintf(vst,"FastbuilderNative Build %d Connected!",BUILD);
	sendText(vst);
	busy=false;
}

FastBuilderSession::~FastBuilderSession(){
	if(busy){
		std::cout<<"Session is busy.Killing..."<<std::endl;
		killbusy();
	}
	std::cout<<"Connection closed."<<std::endl;
	closews(sock);
}

void FastBuilderSession::killbusy(){
	if(busy&&busythr!=(pthread_t)0)pthread_kill(busythr,SIGUSR1);
	busy=false;
}

void FastBuilderSession::errResend(std::string msgid){
	return;
	pthread_mutex_lock(&FSTMutex);
	auto res=packetsMap.find(msgid);
	if(res==packetsMap.end())return;
	pthread_mutex_unlock(&FSTMutex);
	sendCommand(packetsMap[msgid]);
	pthread_mutex_lock(&FSTMutex);
	packetsMap.erase(msgid);
	pthread_mutex_unlock(&FSTMutex);
	return;
}

void FastBuilderSession::onMsg(std::string msg){
	//printf("Message: %s\n",msg);
	Json::Reader reader;
	Json::Value proot;
	if(!reader.parse(msg,proot)){
		//std::cout<<"WARN(onMsg): Unable to parse json message."<<std::endl;
		return;
	}
	
	std::string mPur=proot["header"]["messagePurpose"].asString();
	std::string requestId=proot["header"]["requestId"].asString();
	if(getNMark(requestId)){
		setUValue(requestId,msg);
	}
	if(mPur=="error"){
		errResend(requestId);
	}else if(mPur=="commandResponse"){
		//killFSTWithUuid(requestId);
		/*if(proot["body"].isMember("fillCount"))calcBSP();
		if(proot["body"]["statusCode"].asInt()!=0){
			//printf("Error Message:%s\n",msg);
		}*/
	}else if(mPur=="event"){
		if(!proot["body"]["properties"].isMember("Message"))return;
		std::string pmsg=proot["body"]["properties"]["Message"].asString();
		argInput inp=argInput(pmsg);
		if(!inp.invcmd)algorithms->builder(inp,this);
	}
}

void FastBuilderSession::send(void*cl,std::string msg){
	_send(cl,msg);
}

void onConnection(void *cSock){
	if(fastbuilder->shutting_down){
		closews(cSock);
		return;
	}
	fastbuilder->FBSMap[cSock]=new FastBuilderSession(cSock);
}

void onMsg(void *cSock,std::string msg){
	if(fastbuilder->shutting_down)return;
	auto result=fastbuilder->FBSMap.find(cSock);
	if(result==fastbuilder->FBSMap.end())return;
	fastbuilder->FBSMap[cSock]->onMsg(msg);
}

void FastBuilder_Z____Disconnection(void* ws){
	if(fastbuilder->shutting_down)return;
	auto result=fastbuilder->FBSMap.find(ws);
	if(result==fastbuilder->FBSMap.end())return;
	fastbuilder->FBSMap[ws]->killbusy();
	delete fastbuilder->FBSMap[ws];
	fastbuilder->FBSMap.erase(ws);
}

std::map<void *,FastBuilderSession*> FastBuilder_Z____GetFBSMap(){
	return fastbuilder->FBSMap;
}

#if !defined(shared)
int main(){
#else
int FastBuilder_Start(){
#endif
	CrashHandler::registerCrashHandler();
	fastbuilder=new FastBuilder();
	signal(SIGINT,[](int signal){delete fastbuilder;exit(0);});
	createWebsocketServer(8080);
	return 0;
}