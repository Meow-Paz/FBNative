#pragma once
#include <string>
#include <stdlib.h>

extern "C" void *cmalloc(size_t size,std::string name);
extern "C" void cfree(std::string name);