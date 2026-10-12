#include "core/lifecycle/boot/app_lifecycle.h"

#include "common/manager/global_lifecycle.h"
#include "old_game/pvp/match/channel_zombie_matching_authority.h"

#include <ctime>
#include <cstdio>
#include <iostream>


namespace {

int setCodePage(int code_page) {
    // IDA target function is outside app.cpp. Keep this call boundary explicit
    // until the code page utility module is restored from its own export.
    return code_page;
}

}  // namespace

// IDA_EA: 0x808EC64
// IDA_NAME: _Z18setCodePageByLocalv
// DEMANGLED: setCodePageByLocal(void)
int setCodePageByLocal() {
    return setCodePage(950);
}

// IDA_EA: 0x829F966
// IDA_NAME: _Z8ShowLogov
// DEMANGLED: ShowLogo(void)
int ShowLogo() {
    std::cout << std::endl;
    std::puts("**********************************************************");
    std::puts("* +---   +-+ +  +----         DUNGEON & FIGHTER          *");
    std::puts("* |   |  | | |  |                                        *");
    std::puts("* |   |  | | |  +----                                    *");
    std::puts("* |   |  | | |  |      Open Beta Test on 2005.08.10~     *");
    std::puts("* +---   + +-+  |      Copyright(c) 2004,2005 Neople Co. *");
    std::puts("**********************************************************");

    std::time_t now = std::time(0);
    std::tm tmv;
    localtime_r(&now, &tmv);
    return std::printf("[!] Service Date (%02d-%02d-%02d/%02d:%02d)\n",
                       (tmv.tm_year + 1900) % 100,
                       tmv.tm_mon + 1,
                       tmv.tm_mday,
                       tmv.tm_hour,
                       tmv.tm_min);
}

// IDA_EA: 0x829F7C5
// IDA_NAME: _Z27Neof_startupGlobalInstancesv
// DEMANGLED: Neof_startupGlobalInstances(void)
int Neof_startupGlobalInstances() {
    CREATE_CEnvironment();
    CREATE_CGameManager();
    CREATE_CDataManager();
    CREATE_InterDispatcher();
    CREATE_EpollHandler();
    CREATE_PacketDispatcher();
    CREATE_Store();
    CREATE_TimerQueue();
    CREATE_TimerDispatcher();
    return CREATE_GameWorld();
}

// IDA_EA: 0x829F7FF
// IDA_NAME: _Z28Neof_shutdownGlobalInstancesv
// DEMANGLED: Neof_shutdownGlobalInstances(void)
int Neof_shutdownGlobalInstances() {
    channel_zombie_matching_authority::Shutdown();
    DESTROY_GameWorld();
    DESTROY_TimerDispatcher();
    DESTROY_TimerQueue();
    DESTROY_Store();
    DESTROY_PacketDispatcher();
    DESTROY_EpollHandler();
    DESTROY_InterDispatcher();
    DESTROY_CDataManager();
    DESTROY_CGameManager();
    return DESTROY_CEnvironment();
}
