#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


argInput *processARGV(char *cmd);