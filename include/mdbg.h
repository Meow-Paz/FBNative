#pragma once
//#define malloc(ml) malloc(ml);printf("%s:%d  %dM\n",__FILE__,__LINE__,ml/1024/1024);
//#define free(ml) printf("%s:%d  free\n",__FILE__,__LINE__);free(ml);
#define log(str...) FILE *_lg=fopen("debug.log","a");fprintf(_lg,str);fclose(_lg);