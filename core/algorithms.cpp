#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include "../fbmain.h"
#include "argv.h"
#include "../cJSON/cJSON.h"
#include "colortable.h"
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
#include "paletteresolver.h"

#define PI 3.141592653589793

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
	std::map<Vec3*,unsigned char> vmap;
	bool needMD;
	~session(){
		for(auto i:vmap){
			delete i.first;
		}
		vmap.clear();
	}
	
	session(){
		needMD=false;
	}

	void push(int x,int y,int z){
		if(needMD){
			for(auto i:vmap){
				if(i.first->x==x&&i.first->y==y&&i.first->z==z)return;
			}
		}
		vmap[new Vec3(x,y,z)]=1;
	}
};

class csession {
public:
	std::map<Vec3*,Block*> vmap;
	std::vector<Block*> deleted;
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
		vmap.clear();
	}
	
	csession(){
		lp=false;
	}

	csession(bool lp_used){
		lp=lp_used;
	}

	void push(int x,int y,int z,std::string bn,unsigned char bd){
		vmap[new Vec3(x,y,z)]=new Block(bn,bd);
	}

	void push(int x,int y,int z,Block *block){
		vmap[new Vec3(x,y,z)]=block;
	}
};

session *buildround(int x, int y, int z, argInput *input){
	char direction=input->direction;
	double radius=input->radius;
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


session *buildcircle(int x, int y, int z, argInput *input){
	char direction=input->direction;
	double radius=input->radius;
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


session *buildsphere(int x,int y,int z,argInput *input){
	char *shape=input->shape;
	double radius=input->radius;
	session *sess = new session();
	if(!strcmp(shape,"hollow")){
		for (double i = -radius; i <= radius; i++) {
			for (double j = -radius; j <= radius; j++) {
				for (double k = -radius; k <= radius; k++) {
					if (i * i + j * j + k * k <= radius * radius && i * i + j * j + k * k >= (radius - 1) * (radius - 1)) {
						sess->push(x+i,y+j,z+k);
					}
				}
			}
		}
	}else if(!strcmp(shape,"solid")){
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


session *buildellipse(int x,int y,int z,argInput *input){
	char direction=input->direction;
	double length=input->length;
	double width=input->width;
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


session *buildellipsoid(int x,int y,int z,argInput *input){
	double length=input->length;
	double width=input->width;
	double height=input->height;
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


session *buildtorus(int x,int y,int z,argInput *input){
	char direction=input->direction;
	double length=input->length;
	double radius=input->radius;
	double accuracy=input->accuracy;
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

session *buildcone(int x,int y,int z,argInput *input){
	char direction=input->direction;
	double height=input->height;
	double radius=input->radius;
	double accuracy=input->accuracy;
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

session *buildpyramid(int x,int y,int z,argInput *input){
	session *sess = new session();
	char *shape=input->shape;
	double radius=input->radius;
	double height=input->height;
	for(double y = 0 ; y <= height ; y++){
		radius--;
		for (double x = 0 ; x <= radius ; x++){
			for(double z = 0 ; z <= radius ; z++){
				if((strcmp(shape,"hollow") && x <= radius && z <= radius) || (x == radius && y == radius)){
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

session *buildellipticTorus(int x,int y,int z,argInput *input){
	double radius=input->radius;
	double accuracy=input->accuracy;
	double length=input->length;
	double width=input->width;
	char direction=input->direction;
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

unsigned int getMin(unsigned int *arr,int length){
	unsigned int min = arr[0];
	for(int i = 1; i < length; i++) {
		//unsigned char cur = arr[i];
		if(arr[i] < min){min = arr[i];}
	}
	return min;
}

int indexOfL(unsigned int val,unsigned int*List,int length){
	for(int i=0;i<length;i++){
		if(List[i]==val){	
			return i;
		}
	}
	return -1;
}

Block *get_color(unsigned char r, unsigned char g,unsigned char b) {
	int r1,g1,b1;
	Json::Value rootc;
	Json::Reader reader;
	reader.parse(colortable_json,rootc);
	const int size=rootc.size();
	std::map<unsigned short,unsigned short> list;
	for(unsigned short i=0;i<size;i++){
		r1=r-rootc[i]["color"][0].asInt();
		g1=g-rootc[i]["color"][1].asInt();
		b1=b-rootc[i]["color"][2].asInt();
		list[i]=sqrt((r1 * r1) + (g1 * g1) + (b1 * b1));
	}
	unsigned short min=list[0];
	unsigned short index=0;
	for(unsigned short i=1;i<size;i++){
		if(list[i] < min){min = list[i];index=i;}
	}
	Block *block=new Block(rootc[index]["name"].asString(),(unsigned char)rootc[index]["data"].asInt());
	return block;
}

int getFS(char* filename)
{
	FILE *fp=fopen(filename,"rb");
	if(!fp)return -1;
	fseek(fp,0L,SEEK_END);
	int size=ftell(fp);
	fclose(fp);
	return size;
}

csession *draw(std::vector<Block*> list,unsigned int w,unsigned int h,int xx,int yy,int zz){
	int x=xx,y=yy,z=zz+h;
	csession *rsl=new csession();
	int max = w + x;
	int min = x;
	int t = 0;
	while(1){
		if(x == max){
			z = z - 1;
			x = min;
		}
		x+=1;

		rsl->push(x,y,z,list[t]->name,list[t]->data);

		t++;
		if(t == list.size()){
			break;
		}
	}
	return rsl;
}

float ColorDistance(Color color1,Color color2){
	int r1=color1.r,g1=color1.r,b1=color1.b;
	int r2=color2.r,g2=color2.r,b2=color2.b;
	int rmean=(r1+r2)>>1;
	int red=r1-r2;
	int green=g1-g2;
	int blue=b1-b2;
	float result=(((512+rmean)*red*red)>>8) + 4*green*green + (((767-rmean)*blue*blue)>>8);
	return sqrt(result);
}

int findMin(std::vector<float> t){
	int min=t[0];
	int index=0;
	for(int n=1;n<t.size();n++){
		if(t[n]<min){
			min=t[n];
			index=n;
		}
	}
	return index;
}

Block *getBlock(Color c){
	std::vector<float> list;
	Json::Value rootc;
	Json::Reader Jreader;
	Jreader.parse(colortable_json,rootc);
	const int size=rootc.size();
	for(unsigned short i=0;i<size;i++){
		list.push_back(ColorDistance(c,Color(rootc[i]["color"][0].asInt(),rootc[i]["color"][1].asInt(),rootc[i]["color"][2].asInt())));
	}
	int min=findMin(list);
	return new Block(rootc[min]["name"].asString(),rootc[min]["data"].asInt());
}

csession *Paint(int x, int y, int z,argInput*input,void *sock){
	FILE *img=fopen(input->path,"r");
	if(!img){
		sendText("Non-Exist file.",sock);
		return nullptr;
	}
	unsigned char sig[8];
	fread(sig,1,8,img);
	if(!png_check_sig(sig,8)){
		sendText("Not a png file.",sock);
		return nullptr;
	}
	png_structp png=png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	png_infop info=png_create_info_struct(png);
	png_init_io(png,img);
	png_set_sig_bytes(png,8);
	png_read_info(png,info);

	png_uint_32 width = png_get_image_width(png, info);
	png_uint_32 height = png_get_image_height(png, info);
	png_uint_32 bitDepth = png_get_bit_depth(png, info);
	png_uint_32 channels = png_get_channels(png, info);

	png_bytep* rowPtrs=new png_bytep[height];
	unsigned long dataSize = width * height * bitDepth * channels / 8;
	unsigned char* data = new unsigned char[dataSize];
	const unsigned int stride = width * bitDepth * channels / 8;
	for (size_t i = 0; i < height; ++i) {
		png_uint_32 q = (height - i - 1) * stride;
		rowPtrs[i] = (png_bytep)data + q;
	}
	png_read_image(png, rowPtrs);

	std::vector<unsigned char> _d={0,0,0};
	int _dlast=0;
	std::vector<Block*> BuildList;

	for (int i = 0 ; i < width*height*4; i++){
		_d[_dlast]=data[i];
		_dlast++;
		if(_dlast==4/*i != 0 && (i + 1) % 4 == 0*/){
			BuildList.push_back(getBlock(Color(_d[0],_d[1],_d[2])));
			_dlast=0;
		}
	}
	
	csession *ss=draw(BuildList, width, height, x,y,z);
	delete[] rowPtrs;
	delete[] data;
	fclose(img);
	return ss;
}

csession *buildNBT(int x, int y, int z,argInput *input,void *sock){
	std::ifstream file(/*std::string(input->path)*/"/home/user/FastBuilder/ahh.nbt",std::ios::binary);
	zlib::izlibstream nbtz(file);
	auto rootpair=nbt::io::read_compound(nbtz);
	nbt::tag_list palette=rootpair.second->at("palette").as<nbt::tag_list>();
	std::map<unsigned int,Block*> resolvedPalettes;
	unsigned int _end=0;
	for(nbt::value &item:palette){
		std::string name=static_cast<const std::string&>(item.at("Name"));
		unsigned char data=0;
		name.replace(name.begin(),name.begin()+10,"");
		std::string::size_type st=name.find("door");
		std::string::size_type st_=name.find("trapdoor");
		if(st!=std::string::npos&&st_==std::string::npos){
			if(static_cast<const std::string&>(item.at("Properties").at("half"))=="upper"){
				resolvedPalettes[_end]=nullptr;
				_end++;
				continue;
			}
		}
		Json::Reader reader;
		Json::Value root;
		reader.parse(palette_json,root);
		if(root["minecraft"][name].isArray()){
			int size=root["minecraft"][name].size();
			bool ok=true;
			for(int i=0;i<size;i++){
				ok=true;
				Json::Value::Members memb=root["minecraft"][name][i]["block_state"].getMemberNames();
				for(auto it=memb.begin();it!=memb.end();it++){
					if(*it!="shape"&&item.at("Properties").as<nbt::tag_compound>().has_key(*it)&&(static_cast<const std::string&>(item.at("Properties").at(*it))!=root["minecraft"][name][i]["block_state"][*it].asString())){
						//std::cout<<name<<std::endl;
						//std::cout<<static_cast<const std::string&>(item.at("Properties").at(*it))<<"||"<<root["minecraft"][name][i]["block_state"][*it].asString()<<std::endl;
						ok=false;
						break;
					}
				}
				if(ok){
					data=root["minecraft"][name][i]["data_value"]["meta"].asInt();
					break;
				}
			}
		}
		resolvedPalettes[_end]=new Block(name,data);
		_end++;
	}
	nbt::tag_list blocks=rootpair.second->at("blocks").as<nbt::tag_list>();
	csession *ssr=new csession(true);
	for(nbt::value &item:blocks){
		Block *block=resolvedPalettes[static_cast<int64_t>(item.at("state"))];
		if(block==nullptr)continue;
		nbt::tag_list pos=item.at("pos").as<nbt::tag_list>();
		ssr->push(x+static_cast<int64_t>(pos[0]),y+static_cast<int64_t>(pos[1]),z+static_cast<int64_t>(pos[2]),block);
	}
	return ssr;
}

extern "C" void _ZN17argv37_setpos_intERintIvint4Ev3bb(int x,int y,int z);

void setTile(argInput *input,void *sock,session *bsess){
	char cmd[256]={0};
	for(auto i:bsess->vmap){
		sprintf(cmd,"fill %d %d %d %d %d %d %s",i.first->x,i.first->y,i.first->z,i.first->x,i.first->y,i.first->z,input->block);
		sendCommand(std::string(cmd),sock);
		usleep(input->tick);
	}
}

void setLongTile(argInput *input,void *sock,session *bsess){
	char cmd[256]={0};
	int h=input->height;
	for(auto i:bsess->vmap){
		sprintf(cmd,"fill %d %d %d %d %d %d %s",i.first->x,i.first->y,i.first->z,i.first->x,i.first->y+h-1,i.first->z,input->block);
		sendCommand(cmd,sock);
		usleep(input->tick);
	}
}


void setCTile(argInput *input,void *sock,csession *bsess){	
	char cmd[256]={0};
	std::map<Vec3*,Block*> lp;
	for(auto i:bsess->vmap){
		bool lpe=false;
		if(bsess->lp){
			for(std::string bst:palette_last_place){
				if(bst==i.second->name){
					lp[i.first]=i.second;
					lpe=true;
					break;
				}
			}
		}
		if(lpe)continue;
		sprintf(cmd,"fill %d %d %d %d %d %d %s %hhu",i.first->x,i.first->y,i.first->z,i.first->x,i.first->y,i.first->z,i.second->name,i.second->data);
		sendCommand(cmd,sock);
		usleep(input->tick);
	}
	for(auto i:lp){
		sprintf(cmd,"fill %d %d %d %d %d %d %s %hhu",i.first->x,i.first->y,i.first->z,i.first->x,i.first->y,i.first->z,i.second->name,i.second->data);
		sendCommand(cmd,sock);
		usleep(input->tick);
	}
}

void setLongCTile(argInput *input,void *sock,session *bsess){
	std::cout<<"WARN: setLongCTile doesn't work & won't fix"<<std::endl;
	return;
}

int getMethod(argInput *input,session *ss){
	/*if(ss->isCSS){
		if(input->height!=1){
			return 3;
		}else{
			return 2;
		}
	}else if(ss->isEnt){
		if(input->height!=1){
			return 5;
		}else{
			return 4;
		}
	}*/
	if(input->height!=1){
		return 1;
	}else{
		return 0;
	}
}

void doit(int Method,argInput *input,void *sock,session *bsess){
	switch(Method){
		case 0:
			setTile(input,sock,bsess);
			break;
		case 1:
			setLongTile(input,sock,bsess);
			break;
		/*case 2:
			setCTile(input,sock,bsess);
			break;
		case 3:
			setLongCTile(input,sock,bsess);
			break;*/
		case 4:
		case 5:
		default:
			break;
	}
	return;
}

std::map<std::string,std::function<session *(int,int,int,argInput *)>> sessionMethods={{"round",buildround},
	{"circle",buildcircle},
	{"sphere",buildsphere},
	{"ellipse",buildellipse},
	{"ellipsoid",buildellipsoid},
	{"torus",buildtorus},
	{"cone",buildcone},
	{"pyramid",buildpyramid},
	{"ellipticTorus",buildellipticTorus}
	};
std::map<std::string,std::function<csession *(int,int,int,argInput *,void*)>> csessionMethods={
	{"paint",Paint},
	{"NBT",buildNBT}
};

void builder(argInput *build,void *sock){
	if(!strcmp(build->type,"letblockdone")){
		sendText("Data wrote.",sock);
		free(build);
		return;
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
	}
	auto result=csessionMethods.find(std::string(build->type));
	if(result!=csessionMethods.end()){
		csession *sess=result->second(build->x,build->y,build->z,build,sock);
		if(sess==nullptr){
			sendText("Failed",sock);
			free(build);
			return;
		}
		setBuildingStat(1,sess->vmap.size());
		setCTile(build,sock,sess);
		sendText("Structure has been generated!",sock);
		setBuildingStat(0,0);
		free(build);
		delete sess;
		return;
	}
	auto resb=sessionMethods.find(std::string(build->type));
	if(resb!=sessionMethods.end()){
		session *sess=resb->second(build->x,build->y,build->z,build);
		if(sess==nullptr){
			sendText("Failed",sock);
			free(build);
			return;
		}
		setBuildingStat(1,sess->vmap.size());
		doit(getMethod(build,sess),build,sock,sess);
		sendText("Structure has been generated!",sock);
		setBuildingStat(0,0);
		free(build);
		delete sess;
		return;
	}
	sendText("ERROR: No such method.",sock);
	free(build);
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
