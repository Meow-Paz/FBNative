#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include "../fbmain.h"
#include "argv.h"
#include "memorycontroller.h"
#include "../libpng/png.h"
#include "../jsoncpp-1.8.4/include/json/json.h"
#include <mdbg.h>
#include <array>
#include <png.h>
#include <functional>
#include <fstream>
#include <io/izlibstream.h>
#include <io/stream_reader.h>
#include <nbt_tags.h>
#include "algorithms.h"
#include "paletteresolver_lp.h"


class CoreBuildMethods {
public:
	static session *buildround(int x, int y, int z, argInput input){
		char direction=input.direction;
		double radius=input.radius;
		session *sess = new session();
		switch (direction) {
			case 'x':
				for (double i = -radius; i <= radius; i++) {
					for (double j = -radius; j <= radius; j++) {
						if (i * i + j * j < radius * radius) {
							sess->push(x,y+i,z+j);
						}
					}
				}
				break;
			case 'y':
				for (double i = -radius; i <= radius; i++) {
					for (double j = -radius; j <= radius; j++) {
						if (i * i + j * j < radius * radius) {
							sess->push(x+i,y,z+j);
						}
					}
				}
				break;
			case 'z':
				for (double i = -radius; i <= radius; i++) {
					for (double j = -radius; j <= radius; j++) {
						if (i * i + j * j < radius * radius) {
							sess->push(x+i,y + j,z);
						}
					}
				}
				break;
			default:
				break;
		}
		return sess;
	}


	static session *buildcircle(int x, int y, int z, argInput input){
		char direction=input.direction;
		double radius=input.radius;
		session *sess = new session();
		switch (direction) {
			case 'x':
				for (double i = -radius; i <= radius; i++) {
					for (double j = -radius; j <= radius; j++) {
						if (i * i + j * j < radius * radius && i * i + j * j >= (radius - 1) * (radius - 1)) {
							sess->push(x,y+i,z+j);
						}
					}
				}
				break;
			case 'y':
				for (double i = -radius; i <= radius; i++) {
					for (double j = -radius; j <= radius; j++) {
						if (i * i + j * j < radius * radius && i * i + j * j >= (radius - 1) * (radius - 1)) {
							sess->push(x+i,y,z+j);
						}
					}
				}
				break;
			case 'z':
				for (double i = -radius; i <= radius; i++) {
					for (double j = -radius; j <= radius; j++) {
						if (i * i + j * j < radius * radius && i * i + j * j >= (radius - 1) * (radius - 1)) {
							sess->push(x+i,y+j,z);
						}
					}
				}
				break;
			default:
				break;
		}
		return sess;
	}


	static session *buildsphere(int x,int y,int z,argInput input){
		std::string shape=input.shape;
		double radius=input.radius;
		session *sess = new session();
		if(shape=="hollow"){
			for (double i = -radius; i <= radius; i++) {
				for (double j = -radius; j <= radius; j++) {
					for (double k = -radius; k <= radius; k++) {
						if (i * i + j * j + k * k <= radius * radius && i * i + j * j + k * k >= (radius - 1) * (radius - 1)) {
							sess->push(x+i,y+j,z+k);
						}
					}
				}
			}
		}else if(shape=="solid"){
			for (double i = -radius; i <= radius; i++) {
				for (double j = -radius; j <= radius; j++) {
					for (double k = -radius; k <= radius; k++) {
						if (i * i + j * j + k * k <= radius * radius) {
							sess->push(x+i,y+j,z+k);
						}
					}
				}
			}
		}
		return sess;
	}


	static session *buildellipse(int x,int y,int z,argInput input){
		char direction=input.direction;
		double length=input.length;
		double width=input.width;
		session *sess = new session();
		switch (direction) {
			case 'x':
				for (double i = -length; i <= length; i++) {
					for (double j = -width; j <= width; j++) {
						if ((i * i) / (length * length) + (j * j) / (width * width) < 1) {
							sess->push(x,y+i,z+j);
						}
					}
				}
				break;
			case 'y':
				for (double i = -length; i <= length; i++) {
					for (double j = -width; j <= width; j++) {
						if ((i * i) / (length * length) + (j * j) / (width * width) < 1) {
							sess->push(x+i,y,z+j);
						}
					}
				}
				break;
			case 'z':
				for (double i = -length; i <= length; i++) {
					for (double j = -width; j <= width; j++) {
						if ((i * i) / (length * length) + (j * j) / (width * width) < 1) {
							sess->push(x+i,y+z,j);
						}
					}
				}
				break;
			default:
				break;
		}
		return sess;
	}


	static session *buildellipsoid(int x,int y,int z,argInput input){
		double length=input.length;
		double width=input.width;
		double height=input.height;
		session *sess = new session();
		for (double i = -length; i <= length; i++) {
			for (double j = -width; j <= width; j++) {
				for (double k = -height; k <= height; k++) {
					if ((i * i) / (length * length) + (j * j) / (width * width) + (k * k) / (height * height) <= 1) {
						sess->push(x+1,y+j,z+k);
					}
				}
			}
		}
		return sess;
	}


	static session *buildtorus(int x,int y,int z,argInput input){
		char direction=input.direction;
		double length=input.length;
		double radius=input.radius;
		double accuracy=input.accuracy;
		session *sess = new session();
		sess->needMD=true;
		accuracy = 1 / accuracy;
		double max = PI * 2;
		switch (direction) {
			case 'x':
				for (double v = 0; v < max; v = v + accuracy) {
					for (double u = 0; u < max; u = u + accuracy) {
						sess->push(round(cos(u) * (length * cos(v) + radius)) + x,round(sin(u) * (length * cos(v) + radius)) + y,round(length * sin(v)) + z);
					}
				}
				break;

			case 'y':
				for (double v = 0; v < max; v = v + accuracy) {
					for (double u = 0; u < max; u = u + accuracy) {
						sess->push(round(cos(u) * (length * cos(v) + radius)) + x,round(length * sin(v)) + y,round(sin(u) * (length * cos(v) + radius)) + z);
					}
				}
				break;
			case 'z':
				for (double v = 0; v < max; v = v + accuracy) {
					for (double u = 0; u < max; u = u + accuracy) {
						sess->push(round(length * sin(v)) + x,round(cos(u) * (length * cos(v) + radius)) + y,round(sin(u) * (length * cos(v) + radius)) + z);
					}
				}
				break;
			default:
				break;
		}
		return sess;//multiDimensionalUnique(session);
	}

	static session *buildcone(int x,int y,int z,argInput input){
		char direction=input.direction;
		double height=input.height;
		double radius=input.radius;
		double accuracy=input.accuracy;
		session *sessio = new session();
		sessio->needMD=true;
		double max = PI * 2;
		accuracy = 1 / accuracy;
		switch (direction) {
			case 'z':
				for (double u = 0; u < height; u++) {
					for (double i = 0; i < max; i = i + accuracy) {
						sessio->push(floor(((height - u) / height) * radius * cos(i)) + x, floor(((height - u) / height) * radius * sin(i)) + y, u + z);
					}
				}
				break;
			case 'y':
				for (double u = 0; u < height; u++) {
					for (double i = 0; i < max; i = i + accuracy) {
						sessio->push(floor(((height - u) / height) * radius * cos(i)) + x, u + y, floor(((height - u) / height) * radius * sin(i)) + z);
					}
				}
				break;
			case 'x':
				for (double u = 0; u < height; u++) {
					for (double i = 0; i < max; i = i + accuracy) {
						sessio->push(u + x, floor(((height - u) / height) * radius * cos(i)) + y, floor(((height - u) / height) * radius * sin(i)) + z);
					}
				}
				break;
			default:
				break;
		}

		return sessio;//multiDimensionalUnique(session);
	}

	static session *buildpyramid(int x,int y,int z,argInput input){
		session *sess = new session();
		std::string shape=input.shape;
		double radius=input.radius;
		double height=input.height;
		for(double y = 0 ; y <= height ; y++){
			radius--;
			for (double x = 0 ; x <= radius ; x++){
				for(double z = 0 ; z <= radius ; z++){
					if((shape!="hollow" && x <= radius && z <= radius) || (x == radius && y == radius)){
						sess->push(x + x, y + y , z + z);
						sess->push(x - x, y + y , z + z);
						sess->push(x + x, y + y , z - z);
						sess->push(x - x, y + y , z - z);
					}
				}
			}
		}
		return sess;
	}

	static session *buildellipticTorus(int x,int y,int z,argInput input){
		double radius=input.radius;
		double accuracy=input.accuracy;
		double length=input.length;
		double width=input.width;
		char direction=input.direction;
		session *sess=new session();
		sess->needMD=true;
		accuracy = 1 / accuracy;
		double max = PI * 2;
		switch (direction) {
			case 'z':
				for (double v = 0; v < max; v = v + accuracy) {
					for (double u = 0; u < max; u = u + accuracy) {
						sess->push(x + round((radius + (length * cos(v))) * cos(u)), y + round((radius + (length * cos(v))) * sin(u)), z + round(width * sin(v)));
					}
				}
				break;
			case 'y':
				for (double v = 0; v < max; v = v + accuracy) {
					for (double u = 0; u < max; u = u + accuracy) {
						sess->push(x + round((radius + (length * cos(v))) * cos(u)), y + round(width * sin(v)), z + round((radius + (length * cos(v))) * sin(u)));
					}
				}
				break;
			case 'x':
				for (double v = 0; v < max; v = v + accuracy) {
					for (double u = 0; u < max; u = u + accuracy) {
						sess->push(x + round(width * sin(v)), y + round((radius + (length * cos(v))) * sin(u)), z + round((radius + (length * cos(v))) * cos(u)));
					}
				}
				break;
			default:
				break;
		}
		return sess;//multiDimensionalUnique(session);
	}
};


void Algorithms::setTile(argInput input,FastBuilderSession *fbsession,session *bsess){
	char cmd[256]={0};
	for(Vec3 *i:bsess->sort){
		sprintf(cmd,"fill %d %d %d %d %d %d %s",i->x,i->y,i->z,i->x,i->y,i->z,input.block.c_str());
		//printf("%s\n",cmd);
		fbsession->sendCommand(std::string(cmd));
		if(stopFlag){
			stopFlag=false;
			fbsession->sendText("Generating stopped.");
			return;
		}
		usleep(input.tick);
	}
}

void Algorithms::setLongTile(argInput input,FastBuilderSession *fbsession,session *bsess){
	char cmd[256]={0};
	int h=input.height;
	for(auto i:bsess->sort){
		sprintf(cmd,"fill %d %d %d %d %d %d %s",i->x,i->y,i->z,i->x,i->y+h-1,i->z,input.block.c_str());
		//printf("%s\n",cmd);
		fbsession->sendCommand(std::string(cmd));
		if(stopFlag){
			stopFlag=false;
			fbsession->sendText("Generating stopped.");
			return;
		}
		usleep(input.tick);
	}
}


void Algorithms::setCTile(argInput input,FastBuilderSession *fbsession,csession *bsess){	
	char cmd[256]={0};
	std::map<Vec3*,Block*> lp;
	for(Vec3 *i:bsess->sort){
		bool lpe=false;
		if(bsess->lp){
			for(std::string bst:palette_last_place){
				if(bst==bsess->vmap[i]->name){
					lp[i]=bsess->vmap[i];
					lpe=true;
					break;
				}
			}
		}
		if(lpe)continue;
		sprintf(cmd,"fill %d %d %d %d %d %d %s %hhu",i->x,i->y,i->z,i->x,i->y,i->z,bsess->vmap[i]->name,bsess->vmap[i]->data);
		fbsession->sendCommand(cmd);
		if(stopFlag){
			stopFlag=false;
			fbsession->sendText("Generating stopped.");
			return;
		}
		usleep(input.tick);
	}
	if(bsess->lp){
		for(auto i:lp){
			sprintf(cmd,"fill %d %d %d %d %d %d %s %hhu",i.first->x,i.first->y,i.first->z,i.first->x,i.first->y,i.first->z,i.second->name,i.second->data);
			fbsession->sendCommand(cmd);
			if(stopFlag){
				stopFlag=false;
				fbsession->sendText("Generating stopped.");
				return;
			}
			usleep(input.tick);
		}
	}
}

void Algorithms::setLongCTileNOTWORK_(argInput input,FastBuilderSession *fbsession,session *bsess){
	std::cout<<"WARN: setLongCTile doesn't work & won't fix"<<std::endl;
	return;
}

int Algorithms::getMethod(argInput input,session *ss){
	if(input.height!=1&&input.type!="cone"&&input.type!="ellipsoid"&&input.type!="pyramid"){
		return 1;
	}else{
		return 0;
	}
}

void Algorithms::doit(int Method,argInput input,FastBuilderSession *fbsession,session *bsess){
	switch(Method){
		case 0:
			setTile(input,fbsession,bsess);
			break;
		case 1:
			setLongTile(input,fbsession,bsess);
			break;
		case 4:
		case 5:
		default:
			break;
	}
	return;
}

Algorithms::Algorithms(){
	sessionMethods={{"round",&CoreBuildMethods::buildround},
	{"circle",&CoreBuildMethods::buildcircle},
	{"sphere",&CoreBuildMethods::buildsphere},
	{"ellipse",&CoreBuildMethods::buildellipse},
	{"ellipsoid",&CoreBuildMethods::buildellipsoid},
	{"torus",&CoreBuildMethods::buildtorus},
	{"cone",&CoreBuildMethods::buildcone},
	{"pyramid",&CoreBuildMethods::buildpyramid},
	{"ellipticTorus",&CoreBuildMethods::buildellipticTorus}
	};
	stopFlag=false;
	/*csessionMethods={
	{"paint",&CoreBuildMethods::Paint},
	{"NBT",&CoreBuildMethods::buildNBT}
	};*/
}

bool Algorithms::isCommandExist(std::string command_name){
	auto result1=sessionMethods.find(command_name);
	auto result2=csessionMethods.find(command_name);
	if(result1!=sessionMethods.end()||result2!=csessionMethods.end()){
		return true;
	}
	return false;
}

bool Algorithms::registerCommand(std::string command_name,std::function<csession *(int,int,int,argInput,FastBuilderSession*)> cmd_function){
	if(isCommandExist(command_name))return false;
	csessionMethods[command_name]=cmd_function;
	return true;
}

bool Algorithms::registerSimpleCommand(std::string command_name,std::function<session *(int,int,int,argInput)> cmd_function){
	if(isCommandExist(command_name))return false;
	sessionMethods[command_name]=cmd_function;
	return true;
}	

void Algorithms::builder(argInput build,FastBuilderSession *fbsession){
	if(build.type=="stop"){
		stopFlag=true;
		return;
	}
	if(fbsession->busy){
		fbsession->sendText("Session is busy,please wait until current job done.");
		return;
	}
	signal(SIGUSR1,[](int s){
		pthread_exit(nullptr);
	});
	if(build.type=="letblockdone"){
		fbsession->sendText("Data wrote.");
		return;
	}else if(build.type=="getpos"){
		std::string cmdpacket=fbsession->sendCommandSync("testforblock ~~~ air");
		int x;int y;int z;
		Json::Value root;
		Json::Reader rd;
		if(!rd.parse(cmdpacket,root)){
			cfree("sendCommandSync");
			fbsession->sendText("Unable to get pos :(");
			return;
		}
		x=root["body"]["position"]["x"].asInt();
		y=root["body"]["position"]["y"].asInt();
		z=root["body"]["position"]["z"].asInt();
		char msag[32]={0};
		sprintf(msag,"Position got: %d,%d,%d.",x,y,z);
		fbsession->sendText(std::string(msag));
		argInput::setPos(x,y,z);
		cfree("sendCommandSync");
		return;
	}
	auto result=csessionMethods.find(build.type);
	if(result!=csessionMethods.end()){
		fbsession->busy=true;
		fbsession->busythr=pthread_self();
		csession *sess=result->second(build.x,build.y,build.z,build,fbsession);
		if(sess==nullptr){
			fbsession->sendText("Failed.");
			fbsession->busy=false;
			return;
		}
		fbsession->sendText("Generating structure...");
		setCTile(build,fbsession,sess);
		fbsession->sendText("Structure has been generated!");
		delete sess;
		fbsession->busy=false;
		fbsession->lossresolver->clear();
		return;
	}
	auto resb=sessionMethods.find(build.type);
	if(resb!=sessionMethods.end()){
		fbsession->busy=true;
		fbsession->busythr=pthread_self();
		session *sess=resb->second(build.x,build.y,build.z,build);
		if(sess==nullptr){
			fbsession->sendText("Failed");
			fbsession->busy=false;
			return;
		}
		fbsession->sendText("Generating structure...");
		doit(getMethod(build,sess),build,fbsession,sess);
		fbsession->sendText("Structure has been generated!");
		delete sess;
		fbsession->busy=false;
		fbsession->lossresolver->clear();
		return;
	}
	fbsession->sendText("ERROR: No such method.");
	return;
	/*if(!strcmp(build->type,"round")){
		bsess=buildround(build->x,build->y,build->z,build);
	}else if(!strcmp(build->type,"circle")){
		bsess=buildcircle(build->x,build->y,build->z,build);
	}else if(!strcmp(build->type,"sphere")){
		bsess=buildsphere(build->x,build->y,build->z,build);
	}else if(!strcmp(build->type,"ellipse")){
		bsess=buildellipse(build->x,build->y,build->z,build);
	}else if(!strcmp(build->type,"ellipsoid")){
		bsess=buildellipsoid(build->x,build->y,build->z,build);
	}else if(!strcmp(build->type,"torus")){
		bsess=buildtorus(build->x,build->y,build->z,build);
	}else if(!strcmp(build->type,"cone")){
		bsess=buildcone(build->x,build->y,build->z,build);
	}else if(!strcmp(build->type,"pyramid")){
		bsess=buildpyramid(build->x,build->y,build->z,build);
	}else if(!strcmp(build->type,"ellipticTorus")){
		bsess=buildellipticTorus(build->x,build->y,build->z,build);
	}else if(!strcmp(build->type,"paint")){
		csess=Paint(build->x,build->y,build->z,build,sock);
		if(csess==nullptr){
			sendText("Failed");
			free(build);
			return;
		}
		isCsess=true;
	}else if(!strcmp(build->type,"nbt")){
		csess=Paint(build->x,build->y,build->z,build,sock);
		if(csess==nullptr){
			sendText("Failed",sock);
			free(build);
			return;
		}
		isCsess=true;
	}else if(!strcmp(build->type,"getpos")){
		std::string cmdpacket=sendCommandSync("testforblock ~~~ air",sock);
		int x;int y;int z;
		Json::Value root;
		Json::Reader rd;
		if(!rd.parse(cmdpacket,root)){
			cfree("sendCommandSync");
			free(build);
			sendText("Unable to get pos :(",sock);
			return;
		}
		x=root["body"]["position"]["x"].asInt();
		y=root["body"]["position"]["y"].asInt();
		z=root["body"]["position"]["z"].asInt();
		char msag[32]={0};
		sprintf(msag,"Position got: %d,%d,%d.",x,y,z);
		sendText(std::string(msag),sock);
		_ZN17argv37_setpos_intERintIvint4Ev3bb(x,y,z);
		cfree("sendCommandSync");
		free(build);
		return;
	}else{
		char nosuch[50]={0};
		sprintf(nosuch,"No such method:%s.",build->type);
		sendText(nosuch,sock);
		//sendText("No such method.",sock);
		//printf("%s\n",build->type);
		free(build);
		return;
	}
	if(isCsess){
		setBuildingStat(1,csess->vmap.size());
		setCTile(build,sock,csess);
	}else{
		setBuildingStat(1,bsess->vmap.size());
		doit(getMethod(build,bsess),build,sock,bsess);
	}
	sendText("Structure has been generated!",sock);
	setBuildingStat(0,0);
	free(build);
	if(isCsess)delete csess;
	if(!isCsess)delete bsess;*/
}