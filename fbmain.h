#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "Websocket/Handshake.h"
#include "Websocket/Communicate.h"
#include "Websocket/Errors.h"


void onConnection(void *cSock);

void onMsgThr(ws_client *cSock,char *msg);

void onDisCon(ws_client *cSock);

void fbinit(void);

void fbinitDone(void);

void sendText(char *text,ws_client *sock);

void _sendCommand(char *cmd,ws_client *sock,char *uuid);

char *sendCommandSync(char *cmd,ws_client *sock);

#define sendCommand(cmd,sock) _sendCommand(cmd,sock,"null")

void setBuildingStat(int stat,int);