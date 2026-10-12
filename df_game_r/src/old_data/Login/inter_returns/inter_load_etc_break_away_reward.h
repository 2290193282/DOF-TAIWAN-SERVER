#ifndef OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_ETC_BREAK_AWAY_REWARD_H_
#define OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_ETC_BREAK_AWAY_REWARD_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

class AttendanceEvent;
class CUser;
struct SIG_ATTENDANCE_EVENT;
struct SIG_LOAD_ETC;

// IDA unit 99_misc_global/Inter_LoadEtc.  Slot 32 owns an otherwise
// state-free ISigDispatcher object: on i386 it consists solely of the base
// vptr.  The full dispatch_sig closure is intentionally declared here even
// while individual post-load providers are being recovered; callers must not
// register this receiver until that closure is complete.
class Inter_LoadEtc : public ISigDispatcher {
public:
    // IDA_EA: 0x84C0264
    // IDA_NAME: _ZN13Inter_LoadEtc12dispatch_sigEP5CUserPci
    int dispatch_sig(CUser* user, char* payload, int payload_size);

    // IDA_EA: 0x84C266E..0x84C311A
    int RequestDBEventArad(CUser* user);
    char _checkUserDisadvantage(CUser* user);
    int SetPvpRecord(CUser* user, SIG_LOAD_ETC* payload);
    int SetRankRecord(CUser* user, SIG_LOAD_ETC* payload);
    int _fnEnumQuestClear(unsigned int quest_id, char* context);
    int SendResult(CUser* user);
    int SetFatigue(CUser* user);

    // IDA_EA: 0x84D8E68
    // IDA_NAME: _ZN13Inter_LoadEtc19giveBreakAwayRewardEP5CUser
    // DEMANGLED: Inter_LoadEtc::giveBreakAwayReward(CUser *)
    static char giveBreakAwayReward(CUser* user);

    // IDA_EA: 0x84D94C4 / 0x84D98A4
    int ProcessOneDayLethe(SIG_LOAD_ETC* payload, CUser* user,
                           int& should_init_skill);
    int ProcessOneDayLethe2ND(SIG_LOAD_ETC* payload, CUser* user,
                              int& should_init_skill);

    // IDA_EA: 0x84E807E / 0x84E834A / 0x84E83C2
    AttendanceEvent* processAttendAnceEvent(SIG_ATTENDANCE_EVENT* payload,
                                            CUser& user);
    void processGrowthWeaponEvent(CUser& user);
    void processGrowthCreatureEvent(CUser& user);

    // IDA_EA: 0x84E8428
    // IDA_NAME: _ZN13Inter_LoadEtc28processSeriaRoomAniDecoEventE17seriaRoom_AniDecoR5CUser
    // DEMANGLED: Inter_LoadEtc::processSeriaRoomAniDecoEvent(seriaRoom_AniDeco,CUser &)
    char processSeriaRoomAniDecoEvent(seriaRoom_AniDeco decoration,
                                      CUser& user);

    // IDA_EA: 0x84E84F6 / 0x84E866C
    int processCheckInvalidExpPerLevel(CUser& user);
    int verifyCharacVillage(CUser* user);

    // IDA_EA: 0x84E9CB6 / 0x84E9CD2 / 0x84E9D02
    Inter_LoadEtc();
    virtual ~Inter_LoadEtc();
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Inter_LoadEtc) == 0x04u,
              "Inter_LoadEtc must retain its original base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_INTER_RETURNS_INTER_LOAD_ETC_BREAK_AWAY_REWARD_H_
