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
#include <iostream>
#include <map>

typedef struct{
	int x;int y;int z;std::string block;unsigned short data;
} globalPosST;
class GPosST {
public:
	int x,y,z;
	std::string block;
	unsigned short data;
	GPosST(){
		x=0,z=0,y=0,data=0;
		block="iron_block";
	}
};

GPosST globalPos;

unsigned int str_f_replace(std::string& str,const std::string willreplace,const std::string to){
	unsigned int i=0;
	std::string::size_type n;
	while((n=str.find(willreplace))!=std::string::npos){
		str.replace(str.begin()+n,str.begin()+n+willreplace.size(),to);
		i++;
	}
	return i;
}

void argInput::split(const std::string& s,std::vector<std::string>& svn,const char* delim) {
	svn.clear();
	std::vector<std::string> sv;
	char* buffer = new char[s.size() + 1];
	memset((void*)buffer,0,s.size()+1);
	std::copy(s.begin(), s.end(), buffer);
	char* p = std::strtok(buffer, delim);
	do {
	sv.push_back(p);
	} while ((p = std::strtok(NULL, delim)));
	std::string lwa;
	bool lw=false;
	for(std::string str:sv){
		std::string rs=str;
		str_f_replace(rs,"\\\"","}}}}||||/||||{{{{");
		unsigned int iww=str_f_replace(rs,"\"","");
		str_f_replace(rs,"}}}}||||/||||{{{{","\"");
		if(0!=(iww%2)){
			if(!lw){
				lw=true;
				lwa=rs;
			}else{
				lw=false;
				lwa+=(std::string(" ")+rs);
				svn.push_back(lwa);std::cout<<lwa<<std::endl;
			}
		}else{
			if(!lw){
				svn.push_back(rs);
			}else{
				lwa+=(std::string(" ")+rs);
			}
		}
	}
	if(lw)throw std::string("Unterminated string.");
	return;
}

void argInput::setPos(int x,int y,int z){
	globalPos.x=x;
	globalPos.y=y;
	globalPos.z=z;
}

argInput::argInput(std::string cmd){
	if(cmd.c_str()[0]!='-'){
		throw (unsigned char)254;
		return;
	}
	x=globalPos.x,y=globalPos.y,z=globalPos.z;
	block=globalPos.block;
	data=globalPos.data;
	accuracy=50;
	height=1;
	length=2;
	width=2;
	shape="hollow";
	direction='y';
	tick=15000;

	split(cmd,splited);
	if(splited.empty()){
		throw (unsigned char)254;
		return;
	}
	type=splited[0].substr(1);
	for(int i=1;i<splited.size();i++){
		if(type=="get"&&splited[i]=="pos"){
			type="getpos";
			return;
		}
		if(type=="let"&&splited[i]=="block"&&i+1<splited.size()){
			type="dwrote";
			globalPos.block=splited[i+1];
			return;
		}
		if(type=="let"&&splited[i]=="data"&&i+1<splited.size()){
			type="dwrote";
			globalPos.data=std::stoi(splited[i+1]);
			return;
		}
		if(type=="get"){
			throw std::string("Unexpected GET format.");
		}else if(type=="let"){
			throw std::string("Unexpected LET format.");
		}
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
		}else if(splited[i]=="-bd"){
			i++;
			data=std::stoi(splited[i]);
		}else if(splited[i]=="-r"){
			i++;
			radius=std::stod(splited[i]);
		}else if(splited[i]=="-a"){
			i++;
			accuracy=std::stod(splited[i]);
		}
	}
}