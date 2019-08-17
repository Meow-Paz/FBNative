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

void sh(int sig,siginfo_t *inff,void* ptrr){
  void *array[52];
  char **messages;
  int size;
  
  printf("Signal %d (%s) received!\n\nDumping Stacks...\n",sig,strsignal(sig));
  size=backtrace(array,52);
  messages=backtrace_symbols(array,size);
  
  for(int i=0;i<size && messages!=NULL;i++){
    printf("#%d: %s\n",i,messages[i]);
  }
  free(messages);
  exit(sig);
  
}

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
    exit(16);
}

void CrashHandler::registerCrashHandler() {
struct sigaction sa;
  sa.sa_sigaction=sh;
  sa.sa_flags=SA_RESTART|SA_SIGINFO;
  sigaction(SIGSEGV,&sa,(struct sigaction *)NULL);
return;
    struct sigaction act;
    act.sa_handler = (void (*)(int)) handleSignal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGSEGV, &act, 0);
    sigaction(SIGABRT, &act, 0);
}