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
    printf("FastBuilder Crashed because of signal %i received\n", signal);

    /*struct sigaction act;
    act.sa_handler = nullptr;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGSEGV, &act, 0);
    sigaction(SIGABRT, &act, 0);*/

    hasCrashed = true;
    void** ptr = &aptr;
    void *array[25];
    int count = backtrace(array, 25);
    char **symbols = backtrace_symbols(array, count);
    char *nameBuf = (char*) malloc(256);
    size_t nameBufLen = 256;
    printf("Backtrace elements: %i\n", count);
    for (int i = 0; i < count; i++) {
        if (symbols[i] == nullptr) {
            printf("#%i unk [0x%04x]\n", i, (long)array[i]);
            continue;
        }
        if (symbols[i][0] == '[') { // unknown symbol
            Dl_info symInfo;
            if (dladdr(array[i], &symInfo)) {
                int status = 0;
                nameBuf = abi::__cxa_demangle(symInfo.dli_sname, nameBuf, &nameBufLen, &status);
                printf("#%i HYBRIS %s+%i in %s+0x%04x [0x%04x]\n", i, nameBuf, (unsigned long) array[i] - (unsigned long) symInfo.dli_saddr, symInfo.dli_fname, (unsigned long) array[i] - (unsigned long) symInfo.dli_fbase, (long)array[i]);
                continue;
            }
        }
        printf("#%i %s\n", i, symbols[i]);
    }
    printf("Dumping stack...\n");
    for (int i = 0; i < 1000; i++) {
        void* pptr = *ptr;
        Dl_info symInfo;
        if (dladdr(pptr, &symInfo)) {
            int status = 0;
            nameBuf = abi::__cxa_demangle(symInfo.dli_sname, nameBuf, &nameBufLen, &status);
            printf("#%i HYBRIS %s+%i in %s+0x%04x [0x%04x]\n", i, nameBuf, (unsigned long) pptr - (unsigned long) symInfo.dli_saddr, symInfo.dli_fname, (unsigned long) pptr - (unsigned long) symInfo.dli_fbase, (long)pptr);
        }
        ptr++;
    }
    fflush(stdout);
    printf("Please report this bug.\n");
    std::map<void*,FastBuilderSession*> FBSMap=FastBuilder_Z____GetFBSMap();
    for(auto i:FBSMap){
        i.second->sendText("We're sorry that FBNative is crashed.\nPlease visit console for more details.\nThanks for support");
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