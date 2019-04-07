#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#include <zlib.h>

uint8_t * loadpng(char * filename,uint32_t * width,uint32_t * height);