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

void sendText(std::string text,void *sock);

void sendCommand(const std::string cmd,void *sock,std::string uuid="null");

const std::string sendCommandSync(const std::string cmd,void *sock);

void setBuildingStat(int stat,int);