#include "log.h"

unsigned char log::level=3;
bool log::colored=true;

void log::init(unsigned char log_level,bool color){
	level=log_level;
	colored=color;
}
void log::fatal(std::string msg,std::string module){
	if(level>=0)std::cout<<(colored?"\033[1;31m":"")<<"["<<module<<"|Fatal] "<<msg<<(colored?"\033[0m":"")<<std::endl;
}
void log::error(std::string msg,std::string module){
	if(level>=0)std::cout<<(colored?"\033[0;31m":"")<<"["<<module<<"|Error] "<<msg<<(colored?"\033[0m":"")<<std::endl;
}
void log::warn(std::string msg,std::string module){
	if(level>=1)std::cout<<(colored?"\033[1;33m":"")<<"["<<module<<"|Warn] "<<msg<<(colored?"\033[0m":"")<<std::endl;
}
void log::info(std::string msg,std::string module){
	if(level>=2)std::cout<<(colored?"\033[1;36m":"")<<"["<<module<<"|Info] "<<msg<<(colored?"\033[0m":"")<<std::endl;
}
void log::trace(std::string msg,std::string module){
	if(level>=3)std::cout<<"["<<module<<"|Trace] "<<msg<<std::endl;
}