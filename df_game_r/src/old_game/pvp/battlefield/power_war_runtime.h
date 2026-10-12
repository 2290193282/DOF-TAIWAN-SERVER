#ifndef OLDSRV_OLD_GAME_WORLD_POWER_WAR_RUNTIME_H_
#define OLDSRV_OLD_GAME_WORLD_POWER_WAR_RUNTIME_H_

#include <stdint.h>

#include <map>
#include <string>

class CUser;
struct SIG_LOAD_POWER_WAR_STATUE_INFO;

// IDA type name used by CPowerWarPacketLog::InsertPacketLog.  Only the
// ordinal storage is known here; the packet logger keeps it verbatim.
enum eSendType {
    E_SEND_TYPE_UNKNOWN = 0
};

// IDA_EA: 0x8480F9E
// IDA_NAME: _ZN26STPowerWarStatueRankerInfoC2Ev
// DEMANGLED: STPowerWarStatueRankerInfo::STPowerWarStatueRankerInfo(void)
//
// Natural i386 alignment supplies the two observed three-byte alignment gaps
// at +0x01 and +0x51.  They are intentionally not represented as raw tails.
struct STPowerWarStatueRankerInfo {
    uint8_t load_complete;          // +0x00
    uint32_t charac_no;             // +0x04
    char charac_name[30];           // +0x08
    uint8_t job;                    // +0x26
    uint8_t level;                  // +0x27
    uint8_t grow_type;              // +0x28
    char guild_name[40];            // +0x29
    uint32_t guild_id;              // +0x54
    int32_t equip_item_id[11];      // +0x58

    STPowerWarStatueRankerInfo();
};

// IDA unit 99_misc_global/STPacketLog.  The original constructor constructs
// only packet_name at +0x10; the packet logger writes the scalar fields when
// a record is first inserted.
struct STPacketLog {
    eSendType send_type;            // +0x00
    int packet_class;               // +0x04
    uint16_t packet_number;         // +0x08
    int count;                      // +0x0c
    std::string packet_name;        // +0x10

    STPacketLog();
    ~STPacketLog();
    static bool Compare(const STPacketLog* left, const STPacketLog* right);
};

// IDA unit 99_misc_global/CPowerWarPacketLog.  Its sole subobject is the
// original std::map<int, STPacketLog*> tree at +0x0.
class CPowerWarPacketLog {
public:
    CPowerWarPacketLog();
    ~CPowerWarPacketLog();
    int Reset();
    int InsertPacketLog(int packet_class,
                        uint16_t packet_number,
                        eSendType send_type,
                        const char* packet_name);
    int PrintPowerWarPacketLogReport();

private:
    std::map<int, STPacketLog*> packet_logs_;
};

// IDA unit: 99_misc_global/CPowerWarLog.  It is embedded in the concrete
// CPowerManager object at +0x1b4; counter slots intentionally retain their
// observed ordinal representation until all report-only labels are recovered.
class CPowerWarLog {
public:
    CPowerWarLog();
    CPowerWarLog* Reset();
    CPowerWarLog* IncAssaultAttemptCount();
    CPowerWarLog* IncAssaultAttemptSuccessCount();
    CPowerWarLog* IncAssaultAttemptFailureCount();
    CPowerWarLog* IncLoadingTimeOutCountSecede();
    CPowerWarLog* IncP2PTimeOutCountSecede();
    CPowerWarLog* IncP2PBadCountFailure();
    CPowerWarLog* IncP2PBadCountSecede();
    CPowerWarLog* IncStartFailureCount();
    CPowerWarLog* IncStartSuccessCount();
    CPowerWarLog* IncTotalJoinAssaultUser(int count);
    CPowerWarLog* IncTotalLeaveAssaultUser();
    int PrintPowerWarAssultReport();

private:
    int counters_[15];
};

// Canonical fully typed 0x208-byte PowerWar owner.  The original C2 at
// 0x847E998 constructs the three ranker records, CPowerWarLog, and
// CPowerWarPacketLog; GlobalData::Init owns its process-lifetime allocation.
class CPowerManager {
public:
    // IDA_EA: 0x847E998
    // IDA_NAME: _ZN13CPowerManagerC2Ev
    // DEMANGLED: CPowerManager::CPowerManager(void)
    CPowerManager();

    // IDA_EA: 0x847EABC
    // IDA_NAME: _ZN13CPowerManager4InitEv
    // DEMANGLED: CPowerManager::Init(void)
    int Init();

    // IDA_EA: 0x847EB76
    // IDA_NAME: _ZN13CPowerManager12SetPowerInfoEcii
    // DEMANGLED: CPowerManager::SetPowerInfo(char,int,int)
    CPowerManager* SetPowerInfo(char winner_side, int power_a, int power_b);

    // IDA_EA: 0x847EBA0
    // IDA_NAME: _ZN13CPowerManager20GetRankingByCharacNoEj
    // DEMANGLED: CPowerManager::GetRankingByCharacNo(unsigned int)
    unsigned int GetRankingByCharacNo(unsigned int charac_no);

    // IDA_EA: 0x847F4FE
    // IDA_NAME: _ZN13CPowerManager14LoadRankerInfoEv
    // DEMANGLED: CPowerManager::LoadRankerInfo(void)
    void LoadRankerInfo();

    // IDA_EA: 0x8480D98
    // IDA_NAME: _ZN13CPowerManager16LoadPowerWarInfoEv
    // DEMANGLED: CPowerManager::LoadPowerWarInfo(void)
    void LoadPowerWarInfo();

    // IDA_EA: 0x847F5DE
    // IDA_NAME: _ZN13CPowerManager12ClearMVPInfoEv
    // DEMANGLED: CPowerManager::ClearMVPInfo(void)
    void ClearMVPInfo();

    static int IsPowerWarEventOn();
    // IDA_EA: 0x847F952
    // IDA_NAME: _ZN13CPowerManager15IsAbleToRevivalEP5CUserjRi
    // DEMANGLED: CPowerManager::IsAbleToRevival(CUser *,unsigned int,int &)
    int IsAbleToRevival(CUser* user,
                        uint32_t current_second,
                        int& remaining_seconds);
    // IDA_EA: 0x837501A.  The PowerWar manager's first byte is the winning
    // side, initialized by C2 and updated by SetPowerInfo.
    int GetWinnerSide();
    // IDA_EA: 0x847EBEC
    // IDA_NAME: _ZN13CPowerManager26SetPowerWarUserRankingInfoEPj
    // DEMANGLED: CPowerManager::SetPowerWarUserRankingInfo(unsigned int *)
    bool SetPowerWarUserRankingInfo(unsigned int* charac_no);
    // IDA_EA: 0x847ED4E
    // IDA_NAME: _ZN13CPowerManager25SetPowerWarUserStatueInfoEP30SIG_LOAD_POWER_WAR_STATUE_INFO
    // DEMANGLED: CPowerManager::SetPowerWarUserStatueInfo(SIG_LOAD_POWER_WAR_STATUE_INFO *)
    void SetPowerWarUserStatueInfo(SIG_LOAD_POWER_WAR_STATUE_INFO* info);
    // IDA_EA: 0x847F1B0 / 0x847F200
    int CheckCompleteDBLoadStatueInfo();
    void SendPowerWarUserStatueInfo(CUser* user);

    // IDA_EA: 0x847FA7C
    // IDA_NAME: _ZN13CPowerManager23CheckPowerWarEnterLimitEP5CUser
    // DEMANGLED: CPowerManager::CheckPowerWarEnterLimit(CUser *)
    int CheckPowerWarEnterLimit(CUser* user);

    // IDA_EA: 0x847FB02
    // IDA_NAME: _ZN13CPowerManager19ProcessJoinPowerWarEP5CUserc
    // DEMANGLED: CPowerManager::ProcessJoinPowerWar(CUser *,char)
    int ProcessJoinPowerWar(CUser* user, char join);

    // IDA_EA: 0x8480102
    // IDA_NAME: _ZN13CPowerManager22IncreasePowerUserCountEc
    // DEMANGLED: CPowerManager::IncreasePowerUserCount(char)
    CPowerManager* IncreasePowerUserCount(char power_side);

    CPowerManager* DecreasePowerUserCount(char power_side);
    CPowerManager* IncreaePowerWarPointInChannel(char power_side,
                                                 int point);
    CPowerManager* SetPowerWarStartTime(uint8_t hour,
                                        uint8_t minute,
                                        uint8_t second,
                                        uint8_t day);
    CPowerManager* SetPowerWarPoint(int power_a, int power_b);
    int GetPowerWarGhostTime(CUser* user, float response_penalty);
    long double GetPowerWarResponPenalty(CUser* user);

    // IDA_EA: 0x82F0906
    // IDA_NAME: _ZN13CPowerManager14GetPowerWarLogEv
    // DEMANGLED: CPowerManager::GetPowerWarLog(void)
    CPowerWarLog* GetPowerWarLog();

private:
    uint8_t winner_side_;                      // +0x00
    int32_t power_a_;                          // +0x04
    int32_t power_b_;                          // +0x08
    int32_t channel_power_a_;                  // +0x0c
    int32_t channel_power_b_;                  // +0x10
    int32_t power_user_count_a_;               // +0x14
    int32_t power_user_count_b_;               // +0x18
    int32_t power_up_remain_time_;             // +0x1c
    int32_t power_up_timer_period_;            // +0x20
    uint8_t power_war_start_hour_;             // +0x24
    uint8_t power_war_start_minute_;           // +0x25
    uint8_t power_war_start_second_;           // +0x26
    uint8_t power_war_start_day_;              // +0x27
    STPowerWarStatueRankerInfo ranker_[3];     // +0x28
    CPowerWarLog power_war_log_;               // +0x1b4
    CPowerWarPacketLog power_war_packet_log_;  // +0x1f0
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(STPowerWarStatueRankerInfo) == 0x84u,
              "Power-war statue ranker IDA ABI must stay 0x84");
static_assert(sizeof(STPacketLog) == 0x14u,
              "Power-war packet log record IDA ABI must stay 0x14");
static_assert(sizeof(CPowerWarPacketLog) == 0x18u,
              "Power-war packet log IDA ABI must stay 0x18");
static_assert(sizeof(CPowerWarLog) == 0x3cu,
              "CPowerWarLog IDA ABI size must stay 0x3c");
static_assert(sizeof(CPowerManager) == 0x208u,
              "CPowerManager IDA ABI size must stay 0x208");
#endif

#endif  // OLDSRV_OLD_GAME_WORLD_POWER_WAR_RUNTIME_H_
