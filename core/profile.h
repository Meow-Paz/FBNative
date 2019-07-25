#include "fbbuildinfo.h"

std::string profile_getLogo(){
	return "    ______           __  ____        _ __    __             _   __\n\
   / ____/___ ______/ /_/ __ )__  __(_) /___/ /__  _____   / | / /\n\
  / /_  / __ `/ ___/ __/ __  / / / / / / __  / _ \\/ ___/  /  |/ / \n\
 / __/ / /_/ (__  ) /_/ /_/ / /_/ / / / /_/ /  __/ /     / /|  /  \n\
/_/    \\__,_/____/\\__/_____/\\__,_/_/_/\\__,_/\\___/_/     /_/ |_/   \n\
                                                                  \n";
}

void profile_printBeta(){
	printf("FB Native Edition\nBuild %d\nVersion Code:v0.1\n",BUILD,BUILD);
}