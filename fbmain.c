#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "Websocket/Handshake.h"
#include "Websocket/Communicate.h"
#include "Websocket/Errors.h"
#include "cJSON/cJSON.h"
#include "core/profile.h"
#include "core/algorithms.h"
#include "core/argv.h"
#include <mdbg.h>
ws_list *getWSL();
#define sendCommand(cmd,sock) _sendCommand(cmd,sock,"null")

int isBuilding=0;
int allBlocks=0;
int doneBlocks=0;

typedef struct{
char inuse;
char done;
char UUID[37];
char result[512];
} wantedUUIDST;

wantedUUIDST wantedUUIDs[1024]={0};


typedef struct{
char inuse;
char UUID[37];
char cmd[128];
} sendedPacketsT;


sendedPacketsT sendedPackets[4096]={0};
pthread_mutex_t FSTMutex;//=PTHREAD_MUTEX_INITIALIZER;

void setBuildingStat(int stat,int abl){
	isBuilding=stat;
	allBlocks=abl;
	doneBlocks=0;
}

void ssleep(int milliseconds) {

    struct timeval timeout = {

        milliseconds / 1000,

        (milliseconds % 1000)

    };

    struct timeval old_timeout = { timeout.tv_sec, timeout.tv_usec };

    while (1) {

        (void)select(0, NULL, NULL, NULL, &timeout);

        if (timeout.tv_sec<=0 && timeout.tv_usec<=0)

            break;

    }

}

void FSTCleaner(void *n){
	while(1){
		if(!isBuilding){
			memset(&sendedPackets,0,sizeof(sendedPacketsT)*4096);
		}
		sleep(2);
	}
	sendedPacketsT wanteduz[1024]={0};
	sendedPacketsT wantedus[1024]={0};
	memcpy(&wanteduz,sendedPackets,1024);
	while(1){
		sleep(2);
		memcpy(&wantedus,sendedPackets,1024);
		for(int i=0;i<1024;i++){
			if(!strcmp(wantedus[i].UUID,wanteduz[i].UUID)&&wantedus[i].inuse&&wanteduz[i].inuse){
				sendedPackets[i].inuse=0;
				//printf("Killed %d.\n",i);
			}
		}
		memcpy(&wanteduz,sendedPackets,1024);
	}
}

int findInFST(){ssleep(500);
	while(1){
	pthread_mutex_lock(&FSTMutex);
	for(int i=0;i<1024;i++){
		if(!sendedPackets[i].inuse){
			sendedPackets[i].inuse=1;
			memset(sendedPackets[i].UUID,0,37);
			memset(sendedPackets[i].cmd,0,512);//printf("%d\n",i);
			pthread_mutex_unlock(&FSTMutex);
			return i;
		}
	}
	pthread_mutex_unlock(&FSTMutex);
	return -1;
	ssleep(50);
	}
}

void killFSTWithUuid(char *uuid){
	pthread_mutex_lock(&FSTMutex);
	for(int i=0;i<1024;i++){
		if(!strcmp(sendedPackets[i].UUID,uuid)){
			sendedPackets[i].inuse=0;
			pthread_mutex_unlock(&FSTMutex);
			return;//One Shot:(
		}
	}
	pthread_mutex_unlock(&FSTMutex);
}

#define killFSTWithId(id) pthread_mutex_lock(&FSTMutex);sendedPackets[id].inuse=0;pthread_mutex_unlock(&FSTMutex);

int findFSTWithUuid(char *uuid){
	pthread_mutex_lock(&FSTMutex);
	for(int i=0;i<1024;i++){
		if(!strcmp(sendedPackets[i].UUID,uuid)&&sendedPackets[i].inuse){
			return i;
		}
	}
	pthread_mutex_unlock(&FSTMutex);
	return -1;
}

void _send(ws_client *cl,char *msg){
	ws_message *ms=message_new();
	ws_connection_close status;
	ms->len=strlen(msg);
	char *temp = malloc( sizeof(char)*(ms->len+1) );

	ws_list *l=getWSL();
	
	
	if (temp == NULL) {
		raise(SIGINT);		
		return;
	}
	memset(temp, '\0', (ms->len+1));
	memcpy(temp, msg, ms->len);
	ms->msg = temp;
	temp = NULL;

	if ( (status = encodeMessage(ms)) != CONTINUE) {
		message_free(ms);
		free(ms);
		raise(SIGINT);
		return;
	}

	list_multicast_one(l,cl,ms);
	message_free(ms);
	free(ms);
}

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


void subscribe(const char *eventN,ws_client *sock){
	char rUUID[37]={0};
	random_uuid(rUUID);
	cJSON *packetj=cJSON_CreateObject();
	cJSON *header=cJSON_CreateObject();
	cJSON_AddStringToObject(header,"requestId",rUUID);
	cJSON_AddStringToObject(header,"messagePurpose","subscribe");
	cJSON_AddNumberToObject(header,"version",1);
	cJSON_AddStringToObject(header,"messageType","commandRequest");
	cJSON_AddItemToObject(packetj,"header",header);
	cJSON *body=cJSON_CreateObject();
	cJSON_AddStringToObject(body,"eventName",eventN);
	cJSON_AddItemToObject(packetj,"body",body);
	char *prTed=cJSON_Print(packetj);
	send(sock,prTed);
	free(prTed);
	cJSON_Delete(packetj);
}

void _sendCommand(char *cmd,ws_client *sock,const char *uuid){
	char rUUID[37]={0};
	char isATG=0;
	if(!strcmp(uuid,"null")){
		random_uuid(rUUID);
		isATG=1;
	}else{
		memcpy(rUUID,uuid,37);
	}
	cJSON *packetj=cJSON_CreateObject();
	cJSON *header=cJSON_CreateObject();
	cJSON_AddStringToObject(header,"requestId",rUUID);
	cJSON_AddStringToObject(header,"messagePurpose","commandRequest");
	cJSON_AddNumberToObject(header,"version",1);
	cJSON_AddStringToObject(header,"messageType","commandRequest");
	cJSON_AddItemToObject(packetj,"header",header);
	cJSON *body=cJSON_CreateObject();
	cJSON *origin=cJSON_CreateObject();
	cJSON_AddStringToObject(origin,"type","player");
	cJSON_AddItemToObject(body,"origin",origin);
	cJSON_AddStringToObject(body,"commandLine",cmd);
	cJSON_AddNumberToObject(body,"version",1);
	cJSON_AddItemToObject(packetj,"body",body);
	char *prTed=cJSON_Print(packetj);
	send(sock,prTed);
	/*if(isATG){
		int id=findInFST();
		if(id==-1){
			free(prTed);
			cJSON_Delete(packetj);
			return;
		}
		memcpy(sendedPackets[id].UUID,rUUID,37);
		memcpy(sendedPackets[id].cmd,cmd,strlen(cmd));
	}*/
	free(prTed);
	cJSON_Delete(packetj);
}

char *sendCommandSync(char *cmd,ws_client *sock){
	char rUUID[37]={0};
	random_uuid(rUUID);
	_sendCommand(cmd,sock,rUUID);
	int doneid=-1;
	for(int i=0;i<1024;i++){
		if(!wantedUUIDs[i].inuse){
			wantedUUIDs[i].inuse=1;
			wantedUUIDs[i].done=0;
			memcpy(wantedUUIDs[i].UUID,rUUID,37);
			doneid=i;
			break;
		}
	}
	if(doneid==-1){
		//fprintf(stderr,"FATAL! sendCommandSync:All wantedUUID structures are currently inuse.Returning null\n");
		return "null";
	}
	while(!wantedUUIDs[doneid].done){usleep(100000);}
	char *strr=malloc(512);
	memcpy(strr,wantedUUIDs[doneid].result,512);
	wantedUUIDs[doneid].inuse=0;
	return strr;//wantedUUIDs[doneid].result;
}

void calcBSP(ws_client *sock){
if(!isBuilding)return;
doneBlocks++;
char stu[128]={0};
sprintf(stu,"title @s actionbar §b%d/%d (%f/100)",doneBlocks,allBlocks,((float)doneBlocks/(float)allBlocks)*100);
sendCommand(stu,sock);
}

void sendText(char *text,ws_client *sock){
	char cmd[1024]={0};
	sprintf(cmd,"say §b%s",text);
	sendCommand(cmd,sock);
}

void onConnection(void *cSockd){
	ws_client *cSock=(ws_client*)cSockd;
	printf("Client connected: %s\n",cSock->client_ip);
	subscribe("PlayerMessage",cSock);
	sendText("Fastbuilder Connected!",cSock);
}

void onDisCon(ws_client *cSock){
	printf("Client disconnected: %s\n",cSock->client_ip);
}

void errResend(ws_client *wsc,char *msgid){
	return;
	int id=findFSTWithUuid(msgid);
	if(id==-1)return;
	char rcmd[128]={0};
	memcpy(rcmd,sendedPackets[id].cmd,128);
	printf("%s\n",rcmd);
	pthread_mutex_unlock(&FSTMutex);
	killFSTWithId(id);
	sendCommand(rcmd,wsc);
	return;
}

void onMsg(ws_client *cSock,char *msg){
	//printf("Message: %s\n",msg);
	cJSON *jmsg=cJSON_Parse(msg);
	cJSON *header=cJSON_GetObjectItem(jmsg,"header");
	cJSON *body=cJSON_GetObjectItem(jmsg,"body");
	cJSON *properties=cJSON_GetObjectItem(body,"properties");
	char *mPur=cJSON_GetObjectItem(header,"messagePurpose")->valuestring;
	char *requestId=cJSON_GetObjectItem(header,"requestId")->valuestring;
	for(int i=0;i<1024;i++){
		if(wantedUUIDs[i].inuse){
			if(!strcmp(wantedUUIDs[i].UUID,requestId)){
				memset(wantedUUIDs[i].result,0,512);
				memcpy(wantedUUIDs[i].result,msg,512);
				wantedUUIDs[i].done=1;
				//cJSON_Delete(jmsg);
				return;
			}
			break;
		}
	}
	if(!strcmp(mPur,"error")){
		errResend(cSock,requestId);
	}else if(!strcmp(mPur,"commandResponse")){
		//killFSTWithUuid(requestId);
		if(cJSON_IsNumber(cJSON_GetObjectItem(body,"fillCount")))calcBSP(cSock);
	}else if(!strcmp(mPur,"event")){
		char *pmsg=cJSON_GetObjectItem(properties,"Message")->valuestring;
		//if(!strcmp(pmsg,"doit")){
			argInput *inp=processARGV(pmsg);
			if(inp!=NULL)builder(inp,cSock);
			/*argInput inp={0};
			inp.x=872;
			inp.y=150;
			inp.z=1244;
			inp.radius=10;
			inp.type="round";
			inp.direction='y';
			builder(&inp,cSock);*/
		//}
	}
	cJSON_Delete(jmsg);
	free(msg);
}

typedef struct{
	ws_client *cSock;
	char *msg;
} onMsgSTR;

void onMsgThrD(void *args){
	onMsgSTR *oms=(onMsgSTR*)args;
	onMsg(oms->cSock,oms->msg);
	free(oms);
}

void onMsgThr(ws_client *cSock,char *msg){
	onMsgSTR *oms=malloc(sizeof(onMsgSTR));
	cJSON *cj=cJSON_Parse(msg);
	char *rmsg=cJSON_Print(cj);
	free(msg);
	free(cj);
	oms->cSock=cSock;
	oms->msg=rmsg;
	pthread_t thrT;
	pthread_create(&thrT,NULL,(void *(*)(void*))onMsgThrD,(void*)oms);
}

void fbinit(void){
	printf("%s",profile_getLogo());
}

void fbinitDone(void){
	profile_printBeta();
	printf("Server initialized.\n\n");
	pthread_t thTT;
	pthread_create(&thTT,NULL,(void *(*)(void*))FSTCleaner,NULL);
	pthread_mutex_init(&FSTMutex,NULL);
}