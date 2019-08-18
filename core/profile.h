#include "fbbuildinfo.h"

class Profile {
public:
	static std::string getLogo(){
		return "    ______           __  ____        _ __    __             _   __\n\
   / ____/___ ______/ /_/ __ )__  __(_) /___/ /__  _____   / | / /\n\
  / /_  / __ `/ ___/ __/ __  / / / / / / __  / _ \\/ ___/  /  |/ / \n\
 / __/ / /_/ (__  ) /_/ /_/ / /_/ / / / /_/ /  __/ /     / /|  /  \n\
/_/    \\__,_/____/\\__/_____/\\__,_/_/_/\\__,_/\\___/_/     /_/ |_/   \n\
	                                                          \n";
	}

	static void staticV12printInfo(){
		printf("FB Native Edition\nBuild %s\nVersion Code:v0.1\nAuthor: LNSSPsd\n",BUILD);
	}
};