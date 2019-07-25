#pragma once

class CrashHandler {

private:
    static bool hasCrashed;

    static void *latestSock;

    static void handleSignal(int signal, void* aptr);

public:
    static void registerCrashHandler();

    static void registerLSock(void *sock);

};