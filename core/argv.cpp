#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "../fbmain.h"
#include "argv.h"
#include <mdbg.h>
#include <string>
#include <vector>
#include <cstring>
#include <memory.h>

typedef struct{
	int x;int y;int z;std::string block;
} globalPosST;
class GPosST {
public:
	int x,y,z;
	std::string block;
	GPosST(){
		x=0,z=0,y=0;
		block="iron_block";
	}
};

GPosST globalPos;


void argInput::split(const std::string& s,std::vector<std::string>& sv,const char* delim) {
	sv.clear();
	char* buffer = new char[s.size() + 1];
	memset((void*)buffer,0,s.size()+1);
	std::copy(s.begin(), s.end(), buffer);
	char* p = std::strtok(buffer, delim);
	do {
	sv.push_back(p);
	} while ((p = std::strtok(NULL, delim)));
	return;
}

void argInput::setPos(int x,int y,int z){
	globalPos.x=x;
	globalPos.y=y;
	globalPos.z=z;
}

argInput::argInput(std::string cmd){
	invcmd=false;
	if(cmd.c_str()[0]!='-'){
		invcmd=true;
		return;
	}
	x=globalPos.x,y=globalPos.y,z=globalPos.z;
	block=globalPos.block;
	accuracy=50;
	height=1;
	length=2;
	width=2;
	shape="hollow";
	direction='y';
	tick=10000;

	split(cmd,splited);
	if(splited.empty()){
		invcmd=true;
		return;
	}
	type=splited[0].substr(1);
	for(int i=1;i<splited.size();i++){
		if(type=="get"&&splited[i]=="pos"){
			type="getpos";
			return;
		}
		/*if(type=="let"&&splited[i]=="block"&&i+1<splited.size()){
			type="letblockdone";
			globalPos.block=
			return;
		}*/
		if(i+1>=splited.size())break;
		if(splited[i]=="-f"){
			i++;
			direction=splited[i].c_str()[0];
		}else if(splited[i]=="-s"){
			i++;
			shape=splited[i];
		}else if(splited[i]=="-z"){
			i++;
			path=splited[i];
		}else if(splited[i]=="-t"){
			i++;
			tick=std::stoi(splited[i]);
		}else if(splited[i]=="-w"){
			i++;
			width=std::stod(splited[i]);
		}else if(splited[i]=="-l"){
			i++;
			length=std::stod(splited[i]);
		}else if(splited[i]=="-h"){
			i++;
			height=std::stoi(splited[i]);
		}else if(splited[i]=="-b"){
			i++;
			block=splited[i];
		}else if(splited[i]=="-r"){
			i++;
			radius=std::stod(splited[i]);
		}else if(splited[i]=="-a"){
			i++;
			accuracy=std::stod(splited[i]);
		}
	}
}