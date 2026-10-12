#include "common/manager/global_lifecycle.h"

#include "infra/concurrent/timer/timer_dispatcher_abi.h"
#include "old_data/db/taiwan/taiwan_runtime_dispatch.h"

#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_instance.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/transport/epoll_handler.h"
#include "infra/concurrent/timer/timer_queue.h"

#include <new>


// The following lifecycle-only class shells keep the old server global
// construction/destruction order intact until each matching IDA class body is
// restored in its own module.
class Store {};

template <>
CGameManager* GlobalInstance<CGameManager>::m_p = 0;
template <>
CDataManager* GlobalInstance<CDataManager>::m_p = 0;
template <>
InterDispatcher* GlobalInstance<InterDispatcher>::m_p = 0;
template <>
EpollHandler* GlobalInstance<EpollHandler>::m_p = 0;
template <>
PacketDispatcher* GlobalInstance<PacketDispatcher>::m_p = 0;
template <>
Store* GlobalInstance<Store>::m_p = 0;
template <>
TimerQueue* GlobalInstance<TimerQueue>::m_p = 0;
template <>
TimerDispatcher* GlobalInstance<TimerDispatcher>::m_p = 0;
template <>
GameWorld* GlobalInstance<GameWorld>::m_p = 0;

template <>
CGameManager* GlobalInstance<CGameManager>::inst_ptr();
template <>
CGameManager* GlobalInstance<CGameManager>::create();
template <>
CGameManager* GlobalInstance<CGameManager>::inst();
template <>
void GlobalInstance<CGameManager>::destroy();

template <>
CDataManager* GlobalInstance<CDataManager>::inst_ptr();
template <>
CDataManager* GlobalInstance<CDataManager>::create();
template <>
CDataManager* GlobalInstance<CDataManager>::inst();
template <>
void GlobalInstance<CDataManager>::destroy();

template <>
InterDispatcher* GlobalInstance<InterDispatcher>::inst_ptr();
template <>
InterDispatcher* GlobalInstance<InterDispatcher>::create();
template <>
InterDispatcher* GlobalInstance<InterDispatcher>::inst();
template <>
void GlobalInstance<InterDispatcher>::destroy();

template <>
EpollHandler* GlobalInstance<EpollHandler>::inst_ptr();
template <>
EpollHandler* GlobalInstance<EpollHandler>::create();
template <>
EpollHandler* GlobalInstance<EpollHandler>::inst();
template <>
void GlobalInstance<EpollHandler>::destroy();

template <>
PacketDispatcher* GlobalInstance<PacketDispatcher>::inst_ptr();
template <>
PacketDispatcher* GlobalInstance<PacketDispatcher>::create();
template <>
PacketDispatcher* GlobalInstance<PacketDispatcher>::inst();
template <>
void GlobalInstance<PacketDispatcher>::destroy();

template <>
Store* GlobalInstance<Store>::inst_ptr();
template <>
Store* GlobalInstance<Store>::create();
template <>
Store* GlobalInstance<Store>::inst();
template <>
void GlobalInstance<Store>::destroy();

template <>
TimerQueue* GlobalInstance<TimerQueue>::inst_ptr();
template <>
TimerQueue* GlobalInstance<TimerQueue>::create();
template <>
TimerQueue* GlobalInstance<TimerQueue>::inst();
template <>
void GlobalInstance<TimerQueue>::destroy();

template <>
TimerDispatcher* GlobalInstance<TimerDispatcher>::inst_ptr();
template <>
TimerDispatcher* GlobalInstance<TimerDispatcher>::create();
template <>
TimerDispatcher* GlobalInstance<TimerDispatcher>::inst();
template <>
void GlobalInstance<TimerDispatcher>::destroy();

template <>
GameWorld* GlobalInstance<GameWorld>::inst_ptr();
template <>
GameWorld* GlobalInstance<GameWorld>::create();
template <>
GameWorld* GlobalInstance<GameWorld>::inst();
template <>
void GlobalInstance<GameWorld>::destroy();

template <typename T>
T* LifecycleCreate(T*& ptr) {
    if (ptr == 0) {
        ptr = new T();
    }
    return ptr;
}

template <typename T>
T* LifecycleInst(T*& ptr) {
    return LifecycleCreate(ptr);
}

template <typename T>
void LifecycleDestroy(T*& ptr) {
    if (ptr != 0) {
        delete ptr;
        ptr = 0;
    }
}

// IDA_EA: 0x82A7029
// IDA_NAME: _Z19CREATE_CEnvironmentv
// DEMANGLED: CREATE_CEnvironment(void)
int CREATE_CEnvironment() {
    GlobalInstance<CEnvironment>::inst();
    return 0;
}

// IDA_EA: 0x82A7036
// IDA_NAME: _Z20DESTROY_CEnvironmentv
// DEMANGLED: DESTROY_CEnvironment(void)
int DESTROY_CEnvironment() {
    GlobalInstance<CEnvironment>::destroy();
    return 0;
}

// IDA_EA: 0x82A7043
// IDA_NAME: _Z19CREATE_CGameManagerv
// DEMANGLED: CREATE_CGameManager(void)
int CREATE_CGameManager() {
    GlobalInstance<CGameManager>::inst();
    return 0;
}

// IDA_EA: 0x82A7050
// IDA_NAME: _Z20DESTROY_CGameManagerv
// DEMANGLED: DESTROY_CGameManager(void)
int DESTROY_CGameManager() {
    GlobalInstance<CGameManager>::destroy();
    return 0;
}

// IDA_EA: 0x82A705D
// IDA_NAME: _Z19CREATE_CDataManagerv
// DEMANGLED: CREATE_CDataManager(void)
int CREATE_CDataManager() {
    GlobalInstance<CDataManager>::inst();
    return 0;
}

// IDA_EA: 0x82A706A
// IDA_NAME: _Z20DESTROY_CDataManagerv
// DEMANGLED: DESTROY_CDataManager(void)
int DESTROY_CDataManager() {
    GlobalInstance<CDataManager>::destroy();
    return 0;
}

// IDA_EA: 0x82A7077
// IDA_NAME: _Z22CREATE_InterDispatcherv
// DEMANGLED: CREATE_InterDispatcher(void)
int CREATE_InterDispatcher() {
    GlobalInstance<InterDispatcher>::inst();
    return 0;
}

// IDA_EA: 0x82A7084
// IDA_NAME: _Z23DESTROY_InterDispatcherv
// DEMANGLED: DESTROY_InterDispatcher(void)
int DESTROY_InterDispatcher() {
    GlobalInstance<InterDispatcher>::destroy();
    return 0;
}

// IDA_EA: 0x82A7091
// IDA_NAME: _Z19CREATE_EpollHandlerv
// DEMANGLED: CREATE_EpollHandler(void)
int CREATE_EpollHandler() {
    GlobalInstance<EpollHandler>::inst();
    return 0;
}

// IDA_EA: 0x82A709E
// IDA_NAME: _Z20DESTROY_EpollHandlerv
// DEMANGLED: DESTROY_EpollHandler(void)
int DESTROY_EpollHandler() {
    GlobalInstance<EpollHandler>::destroy();
    return 0;
}

// IDA_EA: 0x82A70B8
// IDA_NAME: _Z23CREATE_PacketDispatcherv
// DEMANGLED: CREATE_PacketDispatcher(void)
int CREATE_PacketDispatcher() {
    GlobalInstance<PacketDispatcher>::inst();
    return 0;
}

// IDA_EA: 0x82A70C5
// IDA_NAME: _Z24DESTROY_PacketDispatcherv
// DEMANGLED: DESTROY_PacketDispatcher(void)
int DESTROY_PacketDispatcher() {
    GlobalInstance<PacketDispatcher>::destroy();
    return 0;
}

// IDA_EA: 0x82A70D2
// IDA_NAME: _Z12CREATE_Storev
// DEMANGLED: CREATE_Store(void)
int CREATE_Store() {
    GlobalInstance<Store>::inst();
    return 0;
}

// IDA_EA: 0x82A70DF
// IDA_NAME: _Z13DESTROY_Storev
// DEMANGLED: DESTROY_Store(void)
int DESTROY_Store() {
    GlobalInstance<Store>::destroy();
    return 0;
}

// IDA_EA: 0x82A70EC
// IDA_NAME: _Z17CREATE_TimerQueuev
// DEMANGLED: CREATE_TimerQueue(void)
int CREATE_TimerQueue() {
    GlobalInstance<TimerQueue>::inst();
    return 0;
}

// IDA_EA: 0x82A70F9
// IDA_NAME: _Z18DESTROY_TimerQueuev
// DEMANGLED: DESTROY_TimerQueue(void)
int DESTROY_TimerQueue() {
    GlobalInstance<TimerQueue>::destroy();
    return 0;
}

// IDA_EA: 0x82A7106
// IDA_NAME: _Z22CREATE_TimerDispatcherv
// DEMANGLED: CREATE_TimerDispatcher(void)
int CREATE_TimerDispatcher() {
    GlobalInstance<TimerDispatcher>::inst();
    return 0;
}

// IDA_EA: 0x82A7113
// IDA_NAME: _Z23DESTROY_TimerDispatcherv
// DEMANGLED: DESTROY_TimerDispatcher(void)
int DESTROY_TimerDispatcher() {
    GlobalInstance<TimerDispatcher>::destroy();
    return 0;
}

// IDA_EA: 0x82A7120
// IDA_NAME: _Z16CREATE_GameWorldv
// DEMANGLED: CREATE_GameWorld(void)
int CREATE_GameWorld() {
    GlobalInstance<GameWorld>::inst();
    return 0;
}

// IDA_EA: 0x82A712D
// IDA_NAME: _Z17DESTROY_GameWorldv
// DEMANGLED: DESTROY_GameWorld(void)
int DESTROY_GameWorld() {
    GlobalInstance<GameWorld>::destroy();
    return 0;
}

template <>
// IDA_EA: 0x80CD5B4
// IDA_NAME: _ZN14GlobalInstanceI12CGameManagerE8inst_ptrEv
// DEMANGLED: GlobalInstance<CGameManager>::inst_ptr(void)
CGameManager* GlobalInstance<CGameManager>::inst_ptr() {
    return LifecycleCreate(GlobalInstance<CGameManager>::m_p);
}
template <>
// IDA_EA: 0x80CF0E8
// IDA_NAME: _ZN14GlobalInstanceI12CGameManagerE6createEv
// DEMANGLED: GlobalInstance<CGameManager>::create(void)
CGameManager* GlobalInstance<CGameManager>::create() {
    return LifecycleCreate(GlobalInstance<CGameManager>::m_p);
}
template <>
// IDA_EA: 0x82AB72E
// IDA_NAME: _ZN14GlobalInstanceI12CGameManagerE4instEv
// DEMANGLED: GlobalInstance<CGameManager>::inst(void)
CGameManager* GlobalInstance<CGameManager>::inst() {
    return LifecycleInst(GlobalInstance<CGameManager>::m_p);
}
template <>
// IDA_EA: 0x82AB740
// IDA_NAME: _ZN14GlobalInstanceI12CGameManagerE7destroyEv
// DEMANGLED: GlobalInstance<CGameManager>::destroy(void)
void GlobalInstance<CGameManager>::destroy() {
    LifecycleDestroy(GlobalInstance<CGameManager>::m_p);
}

template <>
// IDA_EA: 0x80CD5C6
// IDA_NAME: _ZN14GlobalInstanceI12CDataManagerE8inst_ptrEv
// DEMANGLED: GlobalInstance<CDataManager>::inst_ptr(void)
CDataManager* GlobalInstance<CDataManager>::inst_ptr() {
    GlobalInstance<CDataManager>::create();
    return GlobalInstance<CDataManager>::m_p;
}
template <>
// IDA_EA: 0x80CF147
// IDA_NAME: _ZN14GlobalInstanceI12CDataManagerE6createEv
// DEMANGLED: GlobalInstance<CDataManager>::create(void)
CDataManager* GlobalInstance<CDataManager>::create() {
    CDataManager* ptr = GlobalInstance<CDataManager>::m_p;
    if (ptr == 0) {
        // IDA @0x80CF147 uses the 0xB678-byte original prefix.  The original
        // set_reward_sp @0x8360BDE then writes 86 ints at +0xDA90, beyond that
        // allocation.  The rebuild owns a typed post-prefix table in
        // CDataManager, so allocation must follow the actual C++ object size.
        ptr = static_cast<CDataManager*>(::operator new(sizeof(CDataManager)));
        new (ptr) CDataManager();
        GlobalInstance<CDataManager>::m_p = ptr;
    }
    return ptr;
}
template <>
// IDA_EA: 0x82AB77A
// IDA_NAME: _ZN14GlobalInstanceI12CDataManagerE4instEv
// DEMANGLED: GlobalInstance<CDataManager>::inst(void)
CDataManager* GlobalInstance<CDataManager>::inst() {
    GlobalInstance<CDataManager>::create();
    return GlobalInstance<CDataManager>::m_p;
}
template <>
// IDA_EA: 0x82AB78C
// IDA_NAME: _ZN14GlobalInstanceI12CDataManagerE7destroyEv
// DEMANGLED: GlobalInstance<CDataManager>::destroy(void)
void GlobalInstance<CDataManager>::destroy() {
    if (GlobalInstance<CDataManager>::m_p != 0) {
        CDataManager* ptr = GlobalInstance<CDataManager>::m_p;
        ptr->~CDataManager();
        ::operator delete(ptr);
        GlobalInstance<CDataManager>::m_p = 0;
    }
}

template <>
// IDA_EA: 0x847E93D
// IDA_NAME: _ZN14GlobalInstanceI15InterDispatcherE8inst_ptrEv
// DEMANGLED: GlobalInstance<InterDispatcher>::inst_ptr(void)
InterDispatcher* GlobalInstance<InterDispatcher>::inst_ptr() {
    return LifecycleCreate(GlobalInstance<InterDispatcher>::m_p);
}
template <>
// IDA_EA: 0x82B645A
// IDA_NAME: _ZN14GlobalInstanceI15InterDispatcherE6createEv
// DEMANGLED: GlobalInstance<InterDispatcher>::create(void)
InterDispatcher* GlobalInstance<InterDispatcher>::create() {
    return LifecycleCreate(GlobalInstance<InterDispatcher>::m_p);
}
template <>
// IDA_EA: 0x82AB7C6
// IDA_NAME: _ZN14GlobalInstanceI15InterDispatcherE4instEv
// DEMANGLED: GlobalInstance<InterDispatcher>::inst(void)
InterDispatcher* GlobalInstance<InterDispatcher>::inst() {
    return LifecycleInst(GlobalInstance<InterDispatcher>::m_p);
}
template <>
// IDA_EA: 0x82AB7D8
// IDA_NAME: _ZN14GlobalInstanceI15InterDispatcherE7destroyEv
// DEMANGLED: GlobalInstance<InterDispatcher>::destroy(void)
void GlobalInstance<InterDispatcher>::destroy() {
    LifecycleDestroy(GlobalInstance<InterDispatcher>::m_p);
}

template <>
// IDA_EA: 0x82AB85E
// IDA_NAME: _ZN14GlobalInstanceI12EpollHandlerE8inst_ptrEv
// DEMANGLED: GlobalInstance<EpollHandler>::inst_ptr(void)
EpollHandler* GlobalInstance<EpollHandler>::inst_ptr() {
    return LifecycleCreate(GlobalInstance<EpollHandler>::m_p);
}
template <>
// IDA_EA: 0x82B64B9
// IDA_NAME: _ZN14GlobalInstanceI12EpollHandlerE6createEv
// DEMANGLED: GlobalInstance<EpollHandler>::create(void)
EpollHandler* GlobalInstance<EpollHandler>::create() {
    return LifecycleCreate(GlobalInstance<EpollHandler>::m_p);
}
template <>
// IDA_EA: 0x82AB812
// IDA_NAME: _ZN14GlobalInstanceI12EpollHandlerE4instEv
// DEMANGLED: GlobalInstance<EpollHandler>::inst(void)
EpollHandler* GlobalInstance<EpollHandler>::inst() {
    return LifecycleInst(GlobalInstance<EpollHandler>::m_p);
}
template <>
// IDA_EA: 0x82AB824
// IDA_NAME: _ZN14GlobalInstanceI12EpollHandlerE7destroyEv
// DEMANGLED: GlobalInstance<EpollHandler>::destroy(void)
void GlobalInstance<EpollHandler>::destroy() {
    LifecycleDestroy(GlobalInstance<EpollHandler>::m_p);
}

template <>
// IDA_EA: 0x8236A72
// IDA_NAME: _ZN14GlobalInstanceI16PacketDispatcherE8inst_ptrEv
// DEMANGLED: GlobalInstance<PacketDispatcher>::inst_ptr(void)
PacketDispatcher* GlobalInstance<PacketDispatcher>::inst_ptr() {
    return LifecycleCreate(GlobalInstance<PacketDispatcher>::m_p);
}
template <>
// IDA_EA: 0x82392FB
// IDA_NAME: _ZN14GlobalInstanceI16PacketDispatcherE6createEv
// DEMANGLED: GlobalInstance<PacketDispatcher>::create(void)
PacketDispatcher* GlobalInstance<PacketDispatcher>::create() {
    return LifecycleCreate(GlobalInstance<PacketDispatcher>::m_p);
}
template <>
// IDA_EA: 0x82AB870
// IDA_NAME: _ZN14GlobalInstanceI16PacketDispatcherE4instEv
// DEMANGLED: GlobalInstance<PacketDispatcher>::inst(void)
PacketDispatcher* GlobalInstance<PacketDispatcher>::inst() {
    return LifecycleInst(GlobalInstance<PacketDispatcher>::m_p);
}
template <>
// IDA_EA: 0x82AB882
// IDA_NAME: _ZN14GlobalInstanceI16PacketDispatcherE7destroyEv
// DEMANGLED: GlobalInstance<PacketDispatcher>::destroy(void)
void GlobalInstance<PacketDispatcher>::destroy() {
    LifecycleDestroy(GlobalInstance<PacketDispatcher>::m_p);
}

template <>
// IDA_EA: 0x8152659
// IDA_NAME: _ZN14GlobalInstanceI5StoreE8inst_ptrEv
// DEMANGLED: GlobalInstance<Store>::inst_ptr(void)
Store* GlobalInstance<Store>::inst_ptr() {
    return LifecycleCreate(GlobalInstance<Store>::m_p);
}
template <>
// IDA_EA: 0x815393E
// IDA_NAME: _ZN14GlobalInstanceI5StoreE6createEv
// DEMANGLED: GlobalInstance<Store>::create(void)
Store* GlobalInstance<Store>::create() {
    return LifecycleCreate(GlobalInstance<Store>::m_p);
}
template <>
// IDA_EA: 0x82AB8BC
// IDA_NAME: _ZN14GlobalInstanceI5StoreE4instEv
// DEMANGLED: GlobalInstance<Store>::inst(void)
Store* GlobalInstance<Store>::inst() {
    return LifecycleInst(GlobalInstance<Store>::m_p);
}
template <>
// IDA_EA: 0x82AB8CE
// IDA_NAME: _ZN14GlobalInstanceI5StoreE7destroyEv
// DEMANGLED: GlobalInstance<Store>::destroy(void)
void GlobalInstance<Store>::destroy() {
    LifecycleDestroy(GlobalInstance<Store>::m_p);
}

template <>
// IDA_EA: 0x80F6585
// IDA_NAME: _ZN14GlobalInstanceI10TimerQueueE8inst_ptrEv
// DEMANGLED: GlobalInstance<TimerQueue>::inst_ptr(void)
TimerQueue* GlobalInstance<TimerQueue>::inst_ptr() {
    return LifecycleCreate(GlobalInstance<TimerQueue>::m_p);
}
template <>
// IDA_EA: 0x80F67E1
// IDA_NAME: _ZN14GlobalInstanceI10TimerQueueE6createEv
// DEMANGLED: GlobalInstance<TimerQueue>::create(void)
TimerQueue* GlobalInstance<TimerQueue>::create() {
    return LifecycleCreate(GlobalInstance<TimerQueue>::m_p);
}
template <>
// IDA_EA: 0x82AB8F6
// IDA_NAME: _ZN14GlobalInstanceI10TimerQueueE4instEv
// DEMANGLED: GlobalInstance<TimerQueue>::inst(void)
TimerQueue* GlobalInstance<TimerQueue>::inst() {
    return LifecycleInst(GlobalInstance<TimerQueue>::m_p);
}
template <>
// IDA_EA: 0x82AB908
// IDA_NAME: _ZN14GlobalInstanceI10TimerQueueE7destroyEv
// DEMANGLED: GlobalInstance<TimerQueue>::destroy(void)
void GlobalInstance<TimerQueue>::destroy() {
    LifecycleDestroy(GlobalInstance<TimerQueue>::m_p);
}

template <>
// IDA_EA: 0x847E94F
// IDA_NAME: _ZN14GlobalInstanceI15TimerDispatcherE8inst_ptrEv
// DEMANGLED: GlobalInstance<TimerDispatcher>::inst_ptr(void)
TimerDispatcher* GlobalInstance<TimerDispatcher>::inst_ptr() {
    return LifecycleCreate(GlobalInstance<TimerDispatcher>::m_p);
}
template <>
// IDA_EA: 0x82B6518
// IDA_NAME: _ZN14GlobalInstanceI15TimerDispatcherE6createEv
// DEMANGLED: GlobalInstance<TimerDispatcher>::create(void)
TimerDispatcher* GlobalInstance<TimerDispatcher>::create() {
    return LifecycleCreate(GlobalInstance<TimerDispatcher>::m_p);
}
template <>
// IDA_EA: 0x82AB942
// IDA_NAME: _ZN14GlobalInstanceI15TimerDispatcherE4instEv
// DEMANGLED: GlobalInstance<TimerDispatcher>::inst(void)
TimerDispatcher* GlobalInstance<TimerDispatcher>::inst() {
    return LifecycleInst(GlobalInstance<TimerDispatcher>::m_p);
}
template <>
// IDA_EA: 0x82AB954
// IDA_NAME: _ZN14GlobalInstanceI15TimerDispatcherE7destroyEv
// DEMANGLED: GlobalInstance<TimerDispatcher>::destroy(void)
void GlobalInstance<TimerDispatcher>::destroy() {
    LifecycleDestroy(GlobalInstance<TimerDispatcher>::m_p);
}

template <>
// IDA_EA: 0x80DA4BB
// IDA_NAME: _ZN14GlobalInstanceI9GameWorldE8inst_ptrEv
// DEMANGLED: GlobalInstance<GameWorld>::inst_ptr(void)
GameWorld* GlobalInstance<GameWorld>::inst_ptr() {
    return LifecycleCreate(GlobalInstance<GameWorld>::m_p);
}
template <>
// IDA_EA: 0x80DAA64
// IDA_NAME: _ZN14GlobalInstanceI9GameWorldE6createEv
// DEMANGLED: GlobalInstance<GameWorld>::create(void)
GameWorld* GlobalInstance<GameWorld>::create() {
    return LifecycleCreate(GlobalInstance<GameWorld>::m_p);
}
template <>
// IDA_EA: 0x82AB98E
// IDA_NAME: _ZN14GlobalInstanceI9GameWorldE4instEv
// DEMANGLED: GlobalInstance<GameWorld>::inst(void)
GameWorld* GlobalInstance<GameWorld>::inst() {
    return LifecycleInst(GlobalInstance<GameWorld>::m_p);
}
template <>
// IDA_EA: 0x82AB9A0
// IDA_NAME: _ZN14GlobalInstanceI9GameWorldE7destroyEv
// DEMANGLED: GlobalInstance<GameWorld>::destroy(void)
void GlobalInstance<GameWorld>::destroy() {
    LifecycleDestroy(GlobalInstance<GameWorld>::m_p);
}

// IDA_EA: 0x80CC18E
// IDA_NAME: _Z14G_CGameManagerv
// DEMANGLED: G_CGameManager(void)
CGameManager* G_CGameManager() {
    return GlobalInstance<CGameManager>::inst_ptr();
}

// IDA_EA: 0x80CC19B
// IDA_NAME: _Z14G_CDataManagerv
// DEMANGLED: G_CDataManager(void)
CDataManager* G_CDataManager() {
    return GlobalInstance<CDataManager>::inst_ptr();
}

// IDA_EA: 0x80DA3A7
// IDA_NAME: _Z11G_GameWorldv
// DEMANGLED: G_GameWorld(void)
GameWorld* G_GameWorld() {
    return GlobalInstance<GameWorld>::inst_ptr();
}

// IDA_EA: 0x8151ADD
// IDA_NAME: _Z7G_Storev
// DEMANGLED: G_Store(void)
Store* G_Store() {
    return GlobalInstance<Store>::inst_ptr();
}

// IDA_EA: 0x823453E
// IDA_NAME: _Z18G_PacketDispatcherv
// DEMANGLED: G_PacketDispatcher(void)
PacketDispatcher* G_PacketDispatcher() {
    return GlobalInstance<PacketDispatcher>::inst_ptr();
}

// IDA_EA: 0x847E8B7
// IDA_NAME: _Z17G_InterDispatcherv
// DEMANGLED: G_InterDispatcher(void)
InterDispatcher* G_InterDispatcher() {
    return GlobalInstance<InterDispatcher>::inst_ptr();
}

// IDA_EA: 0x847E8C4
// IDA_NAME: _Z17G_TimerDispatcherv
// DEMANGLED: G_TimerDispatcher(void)
TimerDispatcher* G_TimerDispatcher() {
    return GlobalInstance<TimerDispatcher>::inst_ptr();
}
