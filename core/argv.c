#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../fbmain.h"
#include "../cJSON/cJSON.h"
#include <mdbg.h>

typedef struct{
	int x;int y;int z;
} globalPosST;

typedef struct{
	char direction;
	char *shape;
	double radius;
	double width;
	double length;
	double height;
	double accuracy;
	char *type;
	char *get;
	int tick;
	int x;int y;int z;
} argInput;

#define getToken token=strtok(NULL," ");if(token==NULL){free(input);return NULL;}

globalPosST globalPos={0};


void _ZN0argv37_setpos_int_x_int_y_int_zEv3(int x,int y,int z){
	globalPos.x=x;
	globalPos.y=y;
	globalPos.z=z;
}

argInput *processARGV(char *cmd){
	argInput *input=malloc(sizeof(argInput));
	memset(input,0,sizeof(argInput));
	input->accuracy=50;
	input->height=1;
	input->shape="hollow";
	input->type=strtok(cmd," ");
	input->direction='y';
	input->tick=5000;
	if(input->type[0]!='-'){free(input);return NULL;}
	*input->type++;
	char *token;
	do{
		token=strtok(NULL," ");
		if(token==NULL)break;
		if(!strcmp(input->type,"get")&&!strcmp(token,"pos")){
			input->type="getpos";
			return input;
		}
		if(!strcmp(token,"-f")){
			getToken;
			input->direction=token[0];
		}else if(!strcmp(token,"-s")){
			getToken;
			input->shape=token;
		}else if(!strcmp(token,"-r")){
			getToken;
			sscanf(token,"%lf",&input->radius);
		}else if(!strcmp(token,"-a")){
			getToken;
			sscanf(token,"%lf",&input->accuracy);
		}else if(!strcmp(token,"-t")){
			getToken;
			sscanf(token,"%d",&input->tick);
		}else if(!strcmp(token,"-w")){
			getToken;
			sscanf(token,"%lf",&input->width);
		}else if(!strcmp(token,"-l")){
			getToken;
			sscanf(token,"%lf",&input->length);
		}else if(!strcmp(token,"-h")){
			getToken;
			sscanf(token,"%lf",&input->height);
		}
	}while(token!=NULL);
	input->x=globalPos.x;
	input->y=globalPos.y;
	input->z=globalPos.z;
	return input;
}