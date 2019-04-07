#include "fbbuildinfo.h"

char *profile_getLogo(){
	return "\n\
    ______           __  ____        _ __    __                        \n\
   / ____/___ ______/ /_/ __ )__  __(_) /___/ /__  ___________________ \n\
  / /_  / __ `/ ___/ __/ __  / / / / / / __  / _ \\/ ___/ ___/ ___/ __ \\\n\
 / __/ / /_/ (__  ) /_/ /_/ / /_/ / / / /_/ /  __/ /  / /__/ /  / /_/ /\n\
/_/    \\__,_/____/\\__/_____/\\__,_/_/_/\\__,_/\\___/_/   \\___/_/   \\____/ \n\n";
}

void profile_printBeta(){
	printf("FBCro (FB C Edition) DEADPNG\nBuild %d\nVersion Code:IBv.dp.%d.\n",BUILD,BUILD);
}