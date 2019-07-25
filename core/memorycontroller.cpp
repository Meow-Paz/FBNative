#include <memory>
#include <memory.h>
#include <string>
#include <map>
#include <stdlib.h>
#include <array>

std::map<std::string,void*> memmap;

extern "C" void *cmalloc(size_t size,std::string name){
	auto result=memmap.find(name);
	if(result!=memmap.end()){
		free(memmap[name]);
		memmap[name]=malloc(size);
		return memmap[name];
	}else{
		memmap[name]=malloc(size);
		return memmap[name];
	}
}

extern "C" void cfree(std::string name){
	auto result=memmap.find(name);
	if(result!=memmap.end()){
		free(memmap[name]);
		memmap.erase(name);
	}
}