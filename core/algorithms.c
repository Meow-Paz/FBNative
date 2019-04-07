#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "../fbmain.h"
#include "../Websocket/Handshake.h"
#include "../Websocket/Communicate.h"
#include "../Websocket/Errors.h"
#include "argv.h"
#include "../cJSON/cJSON.h"
#include "colortable.h"
#include "png2raw.h"
#include <mdbg.h>

#define PI 3.1415926536
typedef struct{
	int last;
	int *array[3];
	char isCSS;char isEnt;
	char **csss;
	char *csd;
} _session;

typedef struct{
	char name[32];
	unsigned char data;
} block;

/*typedef struct{
  char direction;
  char *shape;
  double radius;
  double width;
  double length;
  double height;
  double accuracy;
  char *type;
  int x;int y;int z;
  } argInput;*/

void push(int x,int y,int z,_session *session){
	session->array[0][session->last]=x;
	session->array[1][session->last]=y;
	session->array[2][session->last]=z;
	session->last++;
}

void pushC(int x,int y,int z,char *bl,char data,_session *session){
	if(!session->isCSS)session->isCSS=1;
	session->array[0][session->last]=x;
	session->array[1][session->last]=y;
	session->array[2][session->last]=z;
	sprintf(session->csss[session->last],"%s",bl);
	session->csd[session->last]=data;
	session->last++;
}

_session *initSession(){
	_session *ss=malloc(sizeof(_session));
	memset(ss,0,sizeof(_session));
	ss->array[0]=malloc(20*100000*sizeof(int)/3);
	memset(ss->array[0],0,20*100000*sizeof(int)/3);
	ss->array[1]=malloc(20*100000*sizeof(int)/3);
	memset(ss->array[1],0,20*100000*sizeof(int)/3);
	ss->array[2]=malloc(20*100000*sizeof(int)/3);
	memset(ss->array[2],0,20*100000*sizeof(int)/3);
	ss->csss=(char**)malloc(sizeof(char*)*1024*1024*5);
	for(int i=0;i<1024*1024*5;i++){
		ss->csss[i]=(char*)malloc(32);
		memset(ss->csss[i],0,32);
	}
	ss->csd=malloc(1024*1024*5);
	memset(ss->csd,0,1024*1024*5);
	return ss;
}
void freeSession(_session *bsess){
	free(bsess->array[0]);
	free(bsess->array[1]);
	free(bsess->array[2]);
	/*for(int i=0;i<1024*1024*5;i++){
		free(bsess->csss[i]);
	}*/
	free(bsess->csss);
	free(bsess->csd);
	free(bsess);
}

_session *multiDimensionalUnique(_session *inp){
	_session *out=initSession();
	char jump=0;
	for(int i=0;i<inp->last;i++){
		jump=0;
		for(int j=0;j<out->last;j++){
			if(out->array[0][j]==inp->array[0][i]&&out->array[1][j]==inp->array[1][i]&&out->array[2][j]==inp->array[2][i]){
				jump=1;
				break;
			}else{
				/*printf("[%d,%d][%d,%d][%d,%d]\n",out->array[0][j],inp->array[0][i],out->array[1][j],inp->array[1][i],out->array[2][j],inp->array[2][i]);
				  jump=0;
				  break;*/
			}
		}
		if(jump)continue;
		out->array[0][out->last]=inp->array[0][i];
		out->array[1][out->last]=inp->array[1][i];
		out->array[2][out->last]=inp->array[2][i];
		out->last++;
	}
	freeSession(inp);
	return out;
}

_session *buildround(int x, int y, int z, argInput *input){
	char direction=input->direction;
	double radius=input->radius;
	_session *session = initSession();
	switch (direction) {
		case 'x':
			for (double i = -radius; i <= radius; i++) {
				for (double j = -radius; j <= radius; j++) {
					if (i * i + j * j < radius * radius) {
						session->array[0][session->last]=x;
						session->array[1][session->last]=y + i;
						session->array[2][session->last]=z + j;
						session->last++;
					}
				}
			}
			break;
		case 'y':
			for (double i = -radius; i <= radius; i++) {
				for (double j = -radius; j <= radius; j++) {
					if (i * i + j * j < radius * radius) {
						session->array[0][session->last]=x+i;
						session->array[1][session->last]=y;
						session->array[2][session->last]=z + j;
						session->last++;
					}
				}
			}
			break;
		case 'z':
			for (double i = -radius; i <= radius; i++) {
				for (double j = -radius; j <= radius; j++) {
					if (i * i + j * j < radius * radius) {
						session->array[0][session->last]=x+i;
						session->array[1][session->last]=y + j;
						session->array[2][session->last]=z;
						session->last++;
					}
				}
			}
			break;
		default:
			break;
	}
	return session;
}


_session *buildcircle(int x, int y, int z, argInput *input){
	char direction=input->direction;
	double radius=input->radius;
	_session *session = initSession();
	switch (direction) {
		case 'x':
			for (double i = -radius; i <= radius; i++) {
				for (double j = -radius; j <= radius; j++) {
					if (i * i + j * j < radius * radius && i * i + j * j >= (radius - 1) * (radius - 1)) {
						session->array[0][session->last]=x;
						session->array[1][session->last]=y + i;
						session->array[2][session->last]=z + j;
						session->last++;
					}
				}
			}
			break;
		case 'y':
			for (double i = -radius; i <= radius; i++) {
				for (double j = -radius; j <= radius; j++) {
					if (i * i + j * j < radius * radius && i * i + j * j >= (radius - 1) * (radius - 1)) {
						session->array[0][session->last]=x+i;
						session->array[1][session->last]=y;
						session->array[2][session->last]=z + j;
						session->last++;
					}
				}
			}
			break;
		case 'z':
			for (double i = -radius; i <= radius; i++) {
				for (double j = -radius; j <= radius; j++) {
					if (i * i + j * j < radius * radius && i * i + j * j >= (radius - 1) * (radius - 1)) {
						session->array[0][session->last]=x+i;
						session->array[1][session->last]=y + j;
						session->array[2][session->last]=z;
						session->last++;
					}
				}
			}
			break;
		default:
			break;
	}
	return session;
}


_session *buildsphere(int x,int y,int z,argInput *input){
	char *shape=input->shape;
	double radius=input->radius;
	_session *session = initSession();
	if(!strcmp(shape,"hollow")){
		for (double i = -radius; i <= radius; i++) {
			for (double j = -radius; j <= radius; j++) {
				for (double k = -radius; k <= radius; k++) {
					if (i * i + j * j + k * k <= radius * radius && i * i + j * j + k * k >= (radius - 1) * (radius - 1)) {
						session->array[0][session->last]=x+i;
						session->array[1][session->last]=y + j;
						session->array[2][session->last]=z+k;
						session->last++;
					}
				}
			}
		}
	}else if(!strcmp(shape,"solid")){
		for (double i = -radius; i <= radius; i++) {
			for (double j = -radius; j <= radius; j++) {
				for (double k = -radius; k <= radius; k++) {
					if (i * i + j * j + k * k <= radius * radius) {
						session->array[0][session->last]=x+i;
						session->array[1][session->last]=y + j;
						session->array[2][session->last]=z+k;
						session->last++;
					}
				}
			}
		}
	}
	return session;
}


_session *buildellipse(int x,int y,int z,argInput *input){
	char direction=input->direction;
	double length=input->length;
	double width=input->width;
	_session *session = initSession();
	switch (direction) {
		case 'x':
			for (double i = -length; i <= length; i++) {
				for (double j = -width; j <= width; j++) {
					if ((i * i) / (length * length) + (j * j) / (width * width) < 1) {
						session->array[0][session->last]=x;
						session->array[1][session->last]=y + i;
						session->array[2][session->last]=z+j;
						session->last++;
					}
				}
			}
			break;
		case 'y':
			for (double i = -length; i <= length; i++) {
				for (double j = -width; j <= width; j++) {
					if ((i * i) / (length * length) + (j * j) / (width * width) < 1) {
						session->array[0][session->last]=x+i;
						session->array[1][session->last]=y;
						session->array[2][session->last]=z+j;
						session->last++;
					}
				}
			}
			break;
		case 'z':
			for (double i = -length; i <= length; i++) {
				for (double j = -width; j <= width; j++) {
					if ((i * i) / (length * length) + (j * j) / (width * width) < 1) {
						session->array[0][session->last]=x+i;
						session->array[1][session->last]=y + z;
						session->array[2][session->last]=j;
						session->last++;
					}
				}
			}
			break;
		default:
			break;
	}
	return session;
}


_session *buildellipsoid(int x,int y,int z,argInput *input){
	double length=input->length;
	double width=input->width;
	double height=input->height;
	_session *session = initSession();
	for (double i = -length; i <= length; i++) {
		for (double j = -width; j <= width; j++) {
			for (double k = -height; k <= height; k++) {
				if ((i * i) / (length * length) + (j * j) / (width * width) + (k * k) / (height * height) <= 1) {
					session->array[0][session->last]=x+i;
					session->array[1][session->last]=y+j;
					session->array[2][session->last]=z+k;
					session->last++;
				}
			}
		}
	}
	return session;
}


_session *buildtorus(int x,int y,int z,argInput *input){
	char direction=input->direction;
	double length=input->length;
	double radius=input->radius;
	double accuracy=input->accuracy;
	_session *session = initSession();
	accuracy = 1 / accuracy;
	double max = PI * 2;
	switch (direction) {
		case 'x':
			for (double v = 0; v < max; v = v + accuracy) {
				for (double u = 0; u < max; u = u + accuracy) {
					session->array[0][session->last]=round(cos(u) * (length * cos(v) + radius)) + x;
					session->array[1][session->last]=round(sin(u) * (length * cos(v) + radius)) + y;
					session->array[2][session->last]=round(length * sin(v)) + z;
					session->last++;
				}
			}
			break;

		case 'y':
			for (double v = 0; v < max; v = v + accuracy) {
				for (double u = 0; u < max; u = u + accuracy) {
					session->array[0][session->last]=round(cos(u) * (length * cos(v) + radius)) + x;
					session->array[1][session->last]=round(length * sin(v)) + y;
					session->array[2][session->last]=round(sin(u) * (length * cos(v) + radius)) + z;
					session->last++;
				}
			}
			break;
		case 'z':
			for (double v = 0; v < max; v = v + accuracy) {
				for (double u = 0; u < max; u = u + accuracy) {
					session->array[0][session->last]=round(length * sin(v)) + x;
					session->array[1][session->last]=round(cos(u) * (length * cos(v) + radius)) + y;
					session->array[2][session->last]=round(sin(u) * (length * cos(v) + radius)) + z;
				}
			}
			break;
		default:
			break;
	}
	return multiDimensionalUnique(session);
}

_session *buildcone(int x,int y,int z,argInput *input){
	char direction=input->direction;
	double height=input->height;
	double radius=input->radius;
	double accuracy=input->accuracy;
	_session *session = initSession();
	double max = PI * 2;
	accuracy = 1 / accuracy;
	switch (direction) {
		case 'z':
			for (double u = 0; u < height; u++) {
				for (double i = 0; i < max; i = i + accuracy) {
					push(floor(((height - u) / height) * radius * cos(i)) + x, floor(((height - u) / height) * radius * sin(i)) + y, u + z,session);
				}
			}
			break;
		case 'y':
			for (double u = 0; u < height; u++) {
				for (double i = 0; i < max; i = i + accuracy) {
					push(floor(((height - u) / height) * radius * cos(i)) + x, u + y, floor(((height - u) / height) * radius * sin(i)) + z,session);
				}
			}
			break;
		case 'x':
			for (double u = 0; u < height; u++) {
				for (double i = 0; i < max; i = i + accuracy) {
					push(u + x, floor(((height - u) / height) * radius * cos(i)) + y, floor(((height - u) / height) * radius * sin(i)) + z,session);
				}
			}
			break;
		default:
			break;
	}

	return multiDimensionalUnique(session);
}

_session *buildpyramid(int x,int y,int z,argInput *input){
	_session *session = initSession();
	char *shape=input->shape;
	double radius=input->radius;
	double height=input->height;
	for(double y = 0 ; y <= height ; y++){
		radius--;
		for (double x = 0 ; x <= radius ; x++){
			for(double z = 0 ; z <= radius ; z++){
				if((strcmp(shape,"hollow") && x <= radius && z <= radius) || (x == radius && y == radius)){
					push(x + x, y + y , z + z,session);
					push(x - x, y + y , z + z,session);
					push(x + x, y + y , z - z,session);
					push(x - x, y + y , z - z,session);
				}
			}
		}
	}
	return session;
}

_session *buildellipticTorus(int x,int y,int z,argInput *input){
	double radius=input->radius;
	double accuracy=input->accuracy;
	double length=input->length;
	double width=input->width;
	char direction=input->direction;
	_session *session=initSession();
	accuracy = 1 / accuracy;
	double max = PI * 2;
	switch (direction) {
		case 'z':
			for (double v = 0; v < max; v = v + accuracy) {
				for (double u = 0; u < max; u = u + accuracy) {
					push(x + round((radius + (length * cos(v))) * cos(u)), y + round((radius + (length * cos(v))) * sin(u)), z + round(width * sin(v)),session);
				}
			}
			break;
		case 'y':
			for (double v = 0; v < max; v = v + accuracy) {
				for (double u = 0; u < max; u = u + accuracy) {
					push(x + round((radius + (length * cos(v))) * cos(u)), y + round(width * sin(v)), z + round((radius + (length * cos(v))) * sin(u)),session);
				}
			}
			break;
		case 'x':
			for (double v = 0; v < max; v = v + accuracy) {
				for (double u = 0; u < max; u = u + accuracy) {
					push(x + round(width * sin(v)), y + round((radius + (length * cos(v))) * sin(u)), z + round((radius + (length * cos(v))) * cos(u)),session);
				}
			}
			break;
		default:
			break;
	}
	return multiDimensionalUnique(session);
}

char getMin(unsigned char *arr,int length){
	unsigned char min = arr[0];
	for(int i = 1; i < length; i++) {
		//unsigned char cur = arr[i];
		if(arr[i] < min){min = arr[i];}
	}
	return min;
}

int indexOfL(unsigned char val,unsigned char*List,int length){
	for(int i=0;i<length;i++){
		if(List[i]==val){
			return i;
		}
	}
	return -1;
}

block *get_color(unsigned char r, unsigned char g,unsigned char b) {
	unsigned char r1,g1,b1;
	cJSON *colortable=cJSON_Parse(colortable_json);
	unsigned char *List=malloc(cJSON_GetArraySize(colortable)+1);
	memset(List,0,cJSON_GetArraySize(colortable)+1);
	int listlast=0;
	cJSON *color = NULL;
	cJSON_ArrayForEach(color,colortable){
		r1 = r - cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(color,"color"),0)->valueint;
		g1 = g - cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(color,"color"),1)->valueint;
		b1 = b - cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(color,"color"),2)->valueint;
		List[listlast]=(sqrt((r1 * r1) + (g1 * g1) + (b1 * b1)));
		listlast++;
	}
	block *bl=malloc(30);
	memset(bl,0,sizeof(block));
	cJSON *sel=cJSON_GetArrayItem(colortable,indexOfL(getMin(List,listlast),List,listlast));
	sprintf(bl->name,"%s",cJSON_GetObjectItemCaseSensitive(sel,"name")->valuestring);
	//memcpy(bl->name,cJSON_GetObjectItemCaseSensitive(sel,"name")->valuestring,strlen(cJSON_GetObjectItemCaseSensitive(sel,"name")->valuestring));
	bl->data=cJSON_GetObjectItemCaseSensitive(sel,"data")->valueint;
	free(List);
	cJSON_Delete(colortable);
	return bl;//[color[List.indexOf(getMin(List))].name,color[List.indexOf(getMin(List))].data];
}

_session *paintpng(int x,int y,int z,argInput *input){
	uint32_t w,h;
	uint8_t*pixels=loadpng(input->path,&w,&h);
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

_session *draw(block **map,int mapl,int w,int h,int x,int y,int z){
	_session *rsl=initSession();
	int max = w + x;
	int min = x;
	int t = 0;
	while(1){
		if(x == max){
			z = z + 1;
			x = min;
		}
		x+=1;

		/*ss.sendCommand([
		  'setblock',
		  x = x + 1,
		  y,
		  z,
		  map[t][0],
		  map[t][1]
		  ].join(' '));*/
		pushC(x,y,z,map[t]->name,map[t]->data,rsl);

		t++;
		if(t == mapl){
			break;
		}
	}
	return rsl;
}

_session *Paint(int x, int y, int z,argInput*input){
	char *path=input->path;
	int fsize=getFS(input->path);
	block **BuildList = malloc(sizeof(block)*fsize);
	memset(BuildList,0,fsize*sizeof(block));
	int BListlast=0;

	uint32_t w,h;
	uint8_t *arr = loadpng(input->path,&w,&h);
	unsigned char **All = malloc(fsize);
	unsigned char *_d = malloc(w+1);
	int Allast=0,_dlast=0;
	memset(_d,0,w+1);

	int co=0;
	for (int i = 0 ; i < w*h*4; i++){
		if(co==3){
			co=0;
			continue;
		}
		_d[_dlast]=arr[i];
		_dlast++;
		if(co == 2){
			All[Allast]=_d;
			_d = malloc(w+1);
			memset(_d,0,w+1);
			_dlast=0;
			Allast++;
		}
		co++;
	}

	for(int i = 0 ; i < Allast ; i++){
		block *bn=get_color(All[i][0], All[i][1], All[i][2]);
		BuildList[BListlast]=bn;
		BListlast++;
	}

	_session *ss=draw(BuildList,BListlast, w, h, x,y,z);
	for(int i=0;i<BListlast;i++){
		free(BuildList[BListlast]);
	}
	free(BuildList);
	/*for(int i=0;i<Allast-1;i++){
		printf("%d,%d\n",i,Allast);
		free(All[i]);
	}*/
	//free(All);
	return ss;
}

void _ZN0argv37_setpos_int_x_int_y_int_zEv3(int x,int y,int z);

void setTile(argInput *input,ws_client *sock,_session *bsess){
	char cmd[256]={0};
	for(int i=0;i<bsess->last;i++){
		sprintf(cmd,"fill %d %d %d %d %d %d %s",bsess->array[0][i],bsess->array[1][i],bsess->array[2][i],bsess->array[0][i],bsess->array[1][i],bsess->array[2][i],input->block);
		sendCommand(cmd,sock);
		usleep(input->tick);
	}
}

void setLongTile(argInput *input,ws_client *sock,_session *bsess){
	char cmd[256]={0};
	int h=input->height;
	for(int i=0;i<bsess->last;i++){
		sprintf(cmd,"fill %d %d %d %d %d %d %s",bsess->array[0][i],bsess->array[1][i],bsess->array[2][i],bsess->array[0][i],bsess->array[1][i]+h-1,bsess->array[2][i],input->block);
		sendCommand(cmd,sock);
		usleep(input->tick);
	}
}


void setCTile(argInput *input,ws_client *sock,_session *bsess){	
	char cmd[256]={0};
	for(int i=0;i<bsess->last;i++){
		sprintf(cmd,"fill %d %d %d %d %d %d %s %d",bsess->array[0][i],bsess->array[1][i],bsess->array[2][i],bsess->array[0][i],bsess->array[1][i],bsess->array[2][i],bsess->csss[i],bsess->csd[i]);
		sendCommand(cmd,sock);
		usleep(input->tick);
	}
}

void setLongCTile(argInput *input,ws_client *sock,_session *bsess){
	char cmd[256]={0};
	int h=input->height;
	for(int i=0;i<bsess->last;i++){
		sprintf(cmd,"fill %d %d %d %d %d %d %s %d",bsess->array[0][i],bsess->array[1][i],bsess->array[2][i],bsess->array[0][i],bsess->array[1][i]+h-1,bsess->array[2][i],bsess->csss[i],bsess->csd[i]);
		sendCommand(cmd,sock);
		usleep(input->tick);
	}
}

int getMethod(argInput *input,_session *ss){
	if(ss->isCSS){
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
	}
	if(input->height!=1){
		return 1;
	}else{
		return 0;
	}
}

void doit(int Method,argInput *input,ws_client *sock,_session *bsess){
	switch(Method){
		case 0:
			setTile(input,sock,bsess);
			break;
		case 1:
			setLongTile(input,sock,bsess);
			break;
		case 2:
			setCTile(input,sock,bsess);
			break;
		case 3:
			setLongCTile(input,sock,bsess);
			break;
		case 4:
			break;
		case 5:
			break;
		default:
			break;
	}
	return;
}

void builder(argInput *build,ws_client *sock){
	_session *bsess;
	if(!strcmp(build->type,"round")){
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
		bsess=Paint(build->x,build->y,build->z,build);
	}else if(!strcmp(build->type,"letblockdone")){
		sendText("Data wrote.",sock);
		free(build);
		return;
	}else if(!strcmp(build->type,"getpos")){
		char *cmdpacket=sendCommandSync("testforblock ~~~ air",sock);
		int x;int y;int z;
		cJSON *jpack=cJSON_Parse(cmdpacket);
		//printf("%s\n",cmdpacket);
		free(cmdpacket);
		cJSON *body=cJSON_GetObjectItem(jpack,"body");;
		cJSON *position=cJSON_GetObjectItem(body,"position");
		x=cJSON_GetObjectItem(position,"x")->valueint;
		y=cJSON_GetObjectItem(position,"y")->valueint;
		z=cJSON_GetObjectItem(position,"z")->valueint;
		cJSON_Delete(jpack);
		char msag[30]={0};
		sprintf(msag,"Position get: %d,%d,%d.",x,y,z);
		sendText(msag,sock);
		_ZN0argv37_setpos_int_x_int_y_int_zEv3(x,y,z);
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
	setBuildingStat(1,bsess->last);
	doit(getMethod(build,bsess),build,sock,bsess);
	sendText("Structure has been generated!",sock);
	setBuildingStat(0,0);
	free(build);
	freeSession(bsess);
}
