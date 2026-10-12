#ifndef OLDSRV_GLOBAL_TAIWAN_INTERFACE_INTER_DISPATCHER_H_
#define OLDSRV_GLOBAL_TAIWAN_INTERFACE_INTER_DISPATCHER_H_

#include <stdint.h>


class CUser;

// The raw enum is an i386 int.  These values are the old-server wire/internal
// routing codes used by the Taiwan DB -> Inter family.  Naming them here keeps
// every concrete handler on one ABI rather than letting B/C recreate local
// integer enums.
namespace TaiwanInternalPack {
enum T : int {
    LOAD_SERVICE_RESTRICT_POLICY = 1,
    SAVE_SERVICE_RESTRICT_POLICY = 2,
    LOAD_GEOLOCATION = 4,
    CHECK_STAY_TIME_EVENT = 5,
    RESET_STAY_TIME_EVENT = 7,
    UPDATE_EXIT_TIME = 8,
    SELECT_CREATE_DNF_EVENT_INFO = 9,
    SELECT_CREATE_DNF_USER_INFO = 10,
    INSERT_CREATE_DNF_USER_INFO = 11,
    SELECT_MOBILE_AUTH_REWARD = 12,
    SELECT_PCROOM_DAILY_REWARD = 13,
    UPDATE_PCROOM_DAILY_REWARD = 14,
    P2P_HOLE_PUNCHING_STATE = 15,
    INSERT_GIVE_BOX_EVENT_REWARD = 16,
    SAVE_HERO_MISSION_EVENT_DATA = 17,
    LOAD_HERO_MISSION_EVENT_DATA = 18,
    EXP_KEEPING_EVENT = 19,
    FREELY_GIVE_ITEM = 20,
    PVP_TOURNAMENT = 21,
    DUAL_RAID_REWARD = 23,
    RACING_DUNGEON = 24,
    MATCHING_CHANNEL_HANDOFF = 25,
    ZOMBIE_MODE_RESULT = 26,
    GOLDEN_COMMERCE_REWARD = 27,
    GLASS_BOTTLE_EVENT = 28,
    BLUE_HORSE_EVENT = 29,
    SUGAR_TIME = 30,
    SHOOTING_STAR_DIMENSION_POCKET = 31,
    SEA_BOTTOM_EVENT = 32,
};
}


// Old 8-byte Taiwan internal-dispatch interface.  Its virtual order is D1,
// D0, dispatch_sig; the base dispatch slot is pure in the original vtable.
class TaiwanInterfaceInterDispatcher {
public:
    // IDA_EA: 0x8161E32
    // IDA_NAME: _ZN30TaiwanInterfaceInterDispatcherC2EN18TaiwanInternalPack1TE
    // DEMANGLED: TaiwanInterfaceInterDispatcher::TaiwanInterfaceInterDispatcher(TaiwanInternalPack::T)
    explicit TaiwanInterfaceInterDispatcher(TaiwanInternalPack::T code);

    // IDA_EA: 0x8161E4A
    // IDA_NAME: _ZN30TaiwanInterfaceInterDispatcherD2Ev
    // DEMANGLED: TaiwanInterfaceInterDispatcher::~TaiwanInterfaceInterDispatcher()
    // IDA_EA: 0x8161E70
    // IDA_NAME: _ZN30TaiwanInterfaceInterDispatcherD0Ev
    // DEMANGLED: TaiwanInterfaceInterDispatcher::~TaiwanInterfaceInterDispatcher()
    virtual ~TaiwanInterfaceInterDispatcher();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size) = 0;

    // IDA_EA: 0x8161E8E
    // IDA_NAME: _ZNK30TaiwanInterfaceInterDispatcher7getCodeEv
    // DEMANGLED: TaiwanInterfaceInterDispatcher::getCode(void)const
    int getCode() const;

private:
    TaiwanInternalPack::T code_;
};


#if defined(__i386__)
static_assert(sizeof(TaiwanInterfaceInterDispatcher) == 0x08u,
              "Taiwan interface dispatcher must retain its i386 8-byte ABI");
#endif

#endif  // OLDSRV_GLOBAL_TAIWAN_INTERFACE_INTER_DISPATCHER_H_
