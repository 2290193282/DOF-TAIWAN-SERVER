#ifndef OLDSRV_GLOBAL_GLOBAL_LIFECYCLE_H_
#define OLDSRV_GLOBAL_GLOBAL_LIFECYCLE_H_

#include "common/manager/data_manager.h"
#include "common/manager/game_manager.h"
#include "infra/security/policy/packet_dispatcher.h"


// IDA: CREATE_* / DESTROY_* wrappers around GlobalInstance<T>. These wrappers
// are the old df_game_r process-wide object lifecycle used by
// Neof_startupGlobalInstances and Neof_shutdownGlobalInstances.
int CREATE_CEnvironment();
int DESTROY_CEnvironment();
int CREATE_CGameManager();
int DESTROY_CGameManager();
int CREATE_CDataManager();
int DESTROY_CDataManager();
int CREATE_InterDispatcher();
int DESTROY_InterDispatcher();
int CREATE_EpollHandler();
int DESTROY_EpollHandler();
int CREATE_PacketDispatcher();
int DESTROY_PacketDispatcher();
int CREATE_Store();
int DESTROY_Store();
int CREATE_TimerQueue();
int DESTROY_TimerQueue();
int CREATE_TimerDispatcher();
int DESTROY_TimerDispatcher();
int CREATE_GameWorld();
int DESTROY_GameWorld();

class InterDispatcher;
class Store;
class TimerDispatcher;
class GameWorld;

// IDA G_* accessors. These are global-instance accessors only; the owning
// manager business classes are restored in their own modules.
CGameManager* G_CGameManager();
CDataManager* G_CDataManager();
GameWorld* G_GameWorld();
Store* G_Store();
PacketDispatcher* G_PacketDispatcher();
InterDispatcher* G_InterDispatcher();
TimerDispatcher* G_TimerDispatcher();


#endif  // OLDSRV_GLOBAL_GLOBAL_LIFECYCLE_H_
