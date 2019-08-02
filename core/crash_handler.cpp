#include "crash_handler.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>
#include <cxxabi.h>
#include <execinfo.h>
#include <dlfcn.h>
#include "../fbmain.h"
#include <map>

std::map<void *,FastBuilderSession*> FastBuilder_Z____GetFBSMap();


bool CrashHandler::hasCrashed = false;

void CrashHandler::handleSignal(int signal, void *aptr) {
    if (hasCrashed)
        return;
    if(signal==SIGSEGV)printf("SIGSEGV\n");
    if(signal==SIGABRT)printf("SIGABRT\n");
    std::map<void*,FastBuilderSession*> FBSMap=FastBuilder_Z____GetFBSMap();
    for(auto i:FBSMap){
        i.second->sendText("We're sorry that FBNative is crashed.");
    }
    sleep(1);
    abort();
}

void CrashHandler::registerCrashHandler() {
    struct sigaction act;
    act.sa_handler = (void (*)(int)) handleSignal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGSEGV, &act, 0);
    sigaction(SIGABRT, &act, 0);
}