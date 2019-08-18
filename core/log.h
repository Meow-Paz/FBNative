#pragma once
#include <iostream>

class log {
private:
	static unsigned char level;
	static bool colored;
public:
	static void init(unsigned char log_level,bool color=true);
	static void fatal(std::string msg,std::string module="Log");
	static void error(std::string msg,std::string module="Log");
	static void warn(std::string msg,std::string module="Log");
	static void info(std::string msg,std::string module="Log");
	static void trace(std::string msg,std::string module="Log");
};