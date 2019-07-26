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
#include "cJSON/cJSON.h"
#include "core/profile.h"
#include "core/algorithms.h"
#include "core/argv.h"
#include "core/fbsynckeeper.h"
#include "jsoncpp-1.8.4/include/json/json.h"
#include <mdbg.h>
#include "core/crash_handler.h"

int isBuilding=0;
int allBlocks=0;
int doneBlocks=0;


std::map<std::string,std::string> packetsMap;
pthread_mutex_t FSTMutex=PTHREAD_MUTEX_INITIALIZER;

void setBuildingStat(int stat,int abl){
	isBuilding=stat;
	allBlocks=abl;
	doneBlocks=0;
}

void _send(void *cl,std::string msg);

#define send _send

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


void subscribe(const std::string eventN,void *sock){
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

void sendCommand(const std::string cmd,void *sock,std::string uuid="null"){
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

const std::string sendCommandSync(const std::string cmd,void *sock){
	char rUUID[37]={0};
	random_uuid(rUUID);
	sendCommand(cmd,sock,std::string(rUUID));
	setNMark(std::string(rUUID));
	const char *result=nullptr;
	while((result=getUValue(std::string(rUUID)))==nullptr){usleep(100000);}
	return std::string(result);
}

void calcBSP(void *sock){
if(!isBuilding)return;
doneBlocks++;
//char stu[128]={0};
//sprintf(stu,"title @s actionbar §b%d/%d (%f/100)",doneBlocks,allBlocks,((float)doneBlocks/(float)allBlocks)*100);
//sendCommand(std::string(stu),sock);
}

void sendText(std::string text,void *sock){
	std::string cmd;
	cmd=std::string("say §b")+text;
	sendCommand(cmd,sock);
}

void onConnection(void *cSock){
	CrashHandler::registerLSock(cSock);
	std::cout<<"A client connected."<<std::endl;
	subscribe("PlayerMessage",cSock);
	char vst[25]={0};
	sprintf(vst,"FastbuilderNative Build %d Connected!",BUILD);
	sendText(vst,cSock);
}

void onDisCon_DEPRECATED(void *cSock){
	printf("Client disconnected: %s\n","");
}

void errResend(void *wsc,std::string msgid){
	return;
	pthread_mutex_lock(&FSTMutex);
	auto res=packetsMap.find(msgid);
	if(res==packetsMap.end())return;
	pthread_mutex_unlock(&FSTMutex);
	sendCommand(packetsMap[msgid],wsc);
	pthread_mutex_lock(&FSTMutex);
	packetsMap.erase(msgid);
	pthread_mutex_unlock(&FSTMutex);
	return;
}

void onMsg(void *cSock,std::string msg){
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
		errResend(cSock,requestId);
	}else if(mPur=="commandResponse"){
		//killFSTWithUuid(requestId);
		/*if(proot["body"].isMember("fillCount"))calcBSP(cSock);
		if(proot["body"]["statusCode"].asInt()!=0){
			//printf("Error Message:%s\n",msg);
		}*/
	}else if(mPur=="event"){
		if(!proot["body"]["properties"].isMember("Message"))return;
		std::string pmsg=proot["body"]["properties"]["Message"].asString();
		argInput inp=argInput(pmsg);
		if(!inp.invcmd)builder(inp,cSock);
	}
}

void fbinit(void){
	std::cout<<profile_getLogo();
}

void fbinitDone(void){
	profile_printBeta();
	printf("Server initialized.\n\n");
	//pthread_t thTT;
	//pthread_create(&thTT,NULL,(void *(*)(void*))FSTCleaner,NULL);
	//pthread_mutex_init(&FSTMutex,NULL);
}

void createWebsocketServer(unsigned short port);

int main(){
	CrashHandler::registerCrashHandler();
	fbinit();
	createWebsocketServer(8080);
	return 0;
}