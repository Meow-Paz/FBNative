#pragma once
#include "argv.h"
#include <map>
#include <memory>
#include <functional>
#include "../fbmain.h"
#include <climits>

#define PI 3.141592653589793

class FastBuilderSession;

class Vec3 {
public:
	int x,y,z;
	~Vec3(){}
	Vec3(int x1,int y1,int z1){
		x=x1;y=y1;z=z1;
	}
};

class Color {
public:
	unsigned char r,g,b;
	Color(unsigned char r1,unsigned char g1,unsigned char b1){
		r=r1,g=g1,b=b1;
	}
	~Color(){}
};

class Block {
public:
	char *name;
	unsigned char data;
	Block(std::string n,unsigned char d){
		const char *dump=n.c_str();
		name=(char*)malloc(n.size()+1);
		memset((void*)name,0,n.size()+1);
		memcpy((void*)name,(void*)dump,n.size());
		data=d;
	}
	~Block(){
		free(name);
	}
};

class session {
public:
	std::vector<Vec3*> sort;
	bool needMD;
	~session(){
		for(Vec3 *i:sort){
			delete i;
		}
		sort.clear();
	}
	
	session(){
		needMD=false;
	}

	void push(int x,int y,int z){
		if(needMD){
			for(Vec3 *i:sort){
				if(i->x==x&&i->y==y&&i->z==z)return;
			}
		}
		int smallestc=0;
		Vec3 smallestVec(INT_MAX,INT_MAX,INT_MAX);
		int c=0;
		for(Vec3 *i:sort){
			if((i->x>x||i->y>y||i->z>z)&&(i->x<smallestVec.x&&i->y<smallestVec.y&&i->z<smallestVec.z)){
				smallestc=c;
				smallestVec.x=i->x;
				smallestVec.y=i->y;
				smallestVec.z=i->z;
			}
			c++;
		}
		sort.insert(sort.begin()+smallestc,new Vec3(x,y,z));
	}

};

class csession {
public:
	std::map<Vec3*,Block*> vmap;
	std::vector<Block*> deleted;
	std::vector<Vec3*> sort;
	bool lp;
	~csession(){
		for(auto i:vmap){
			delete i.first;
			bool break_out=false;
			for(Block *dblk:deleted){
				if(dblk==i.second){
					break_out=true;
					break;
				}
			}
			if(break_out)continue;
			delete i.second;
			deleted.push_back(i.second);
		}
		sort.clear();
		vmap.clear();
	}
	
	csession(){
		lp=false;
	}

	csession(bool lp_used){
		lp=lp_used;
	}

	void push(int x,int y,int z,std::string bn,unsigned char bd){
		Vec3 *tvec=new Vec3(x,y,z);
		vmap[tvec]=new Block(bn,bd);
		int smallestc=0;
		Vec3 smallestVec(INT_MAX,INT_MAX,INT_MAX);
		int c=0;
		for(Vec3 *i:sort){
			if((i->x>x||i->y>y||i->z>z)&&(i->x<smallestVec.x&&i->y<smallestVec.y&&i->z<smallestVec.z)){
				smallestc=c;
				smallestVec.x=i->x;
				smallestVec.y=i->y;
				smallestVec.z=i->z;
			}
			c++;
		}
		sort.insert(sort.begin()+smallestc,tvec);
	}

	void push(int x,int y,int z,Block *block){
		Vec3 *tvec=new Vec3(x,y,z);
		vmap[tvec]=block;
		int smallestc=0;
		Vec3 smallestVec(INT_MAX,INT_MAX,INT_MAX);
		int c=0;
		for(Vec3 *i:sort){
			if((i->x>x||i->y>y||i->z>z)&&(i->x<smallestVec.x&&i->y<smallestVec.y&&i->z<smallestVec.z)){
				smallestc=c;
				smallestVec.x=i->x;
				smallestVec.y=i->y;
				smallestVec.z=i->z;
			}
			c++;
		}
		sort.insert(sort.begin()+smallestc,tvec);
	}
};

//void builder(argInput build,void *sock);
class Algorithms {
private:
	void setTile(argInput input,FastBuilderSession *fbsession,session *bsess);
	void setLongTile(argInput input,FastBuilderSession *fbsession,session *bsess);
	void setCTile(argInput input,FastBuilderSession *fbsession,csession *bsess);
	void setLongCTileNOTWORK_(argInput input,FastBuilderSession *fbsession,session *bsess);
	int getMethod(argInput input,session *ss);
	void doit(int Method,argInput input,FastBuilderSession *fbsession,session *bsess);
	std::map<std::string,std::function<session *(int,int,int,argInput)>> sessionMethods;
	std::map<std::string,std::function<csession *(int,int,int,argInput,FastBuilderSession*)>> csessionMethods;
	bool stopFlag;
public:
	Algorithms();
	void builder(argInput build,FastBuilderSession *fbsession);
	bool isCommandExist(std::string command_name);
	bool registerCommand(std::string command_name,std::function<csession *(int,int,int,argInput,FastBuilderSession*)> cmd_function);
	bool registerSimpleCommand(std::string command_name,std::function<session *(int,int,int,argInput)> cmd_function);
};