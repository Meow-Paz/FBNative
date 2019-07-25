#include <string>
#include <map>
#include <iostream>
#include <memory.h>
#include "memorycontroller.h"

std::map<std::string,std::string> wantedUUIDMap;
std::map<std::string,bool> needMark;

const char *getUValue(const std::string a){
	auto result=wantedUUIDMap.find(a);
	if(result!=wantedUUIDMap.end()){
		char *ret=(char*)cmalloc(result->second.size()+1,"sendCommandSync");
		memset(ret,0,result->second.size()+1);
		memcpy(ret,result->second.c_str(),result->second.size()+1);
		wantedUUIDMap.erase(a);
		return ret;
	}else{
		return nullptr;
	}
}

void setUValue(const std::string a,const std::string val){
	wantedUUIDMap[a]=val;
	return;
}

bool getNMark(const std::string a){
	//return needMark[a];
	auto result=needMark.find(a);
	if(result!=needMark.end()){
		needMark.erase(a);
		return true;
	}else{
		return false;
	}
}

void setNMark(const std::string a){
	needMark[a]=true;
	return;
}