#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

class argInput {
private:
	std::vector<std::string> splited;
	void split(const std::string& s,std::vector<std::string>& sv,const char* delim = " ");

public:
	std::string block;
	unsigned short data;
	char direction;
	std::string shape;
	double radius;
	double width;
	double length;
	double height;
	double accuracy;
	std::string type;
	std::string getval;
	unsigned int tick;
	int x,y,z;
	std::string path;

	~argInput()=default;

	argInput(std::string cmd);
	static void setPos(int x,int y,int z);
};