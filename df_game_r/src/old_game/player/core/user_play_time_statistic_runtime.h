#ifndef OLDSRV_OLD_GAME_USERINFO_USER_PLAY_TIME_STATISTIC_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_USER_PLAY_TIME_STATISTIC_RUNTIME_H_

#include <stdint.h>

#include <map>

// IDA packet body used after the two stream header integers (182, -1).
// Statistics.cpp:586 writes at most 20 fixed 12-byte records per stream.
struct SIG_USER_PLAY_TIME_STATISTIC {
    struct Record {
        uint32_t account_id;  // +0x00
        int play_time;        // +0x04
        int play_count;       // +0x08
    };

    int record_count;         // +0x00
    Record records[20];       // +0x04
};

static_assert(sizeof(SIG_USER_PLAY_TIME_STATISTIC::Record) == 0x0cu,
              "SIG_USER_PLAY_TIME_STATISTIC record ABI");
static_assert(sizeof(SIG_USER_PLAY_TIME_STATISTIC) == 0xf4u,
              "SIG_USER_PLAY_TIME_STATISTIC stream ABI");

// IDA type: stUserPlayInfo_t.  The original map value stores a total first,
// followed by the number of updates accumulated for that account.
struct stUserPlayInfo_t {
    int play_time;
    int play_count;
};

static_assert(sizeof(stUserPlayInfo_t) == 0x08u,
              "stUserPlayInfo_t IDA ABI");

class CUserPlayTimeStatisticMgr {
public:
    // IDA_EA: 0x86102C0
    // IDA_NAME: _ZN25CUserPlayTimeStatisticMgrC2Ev
    // DEMANGLED: CUserPlayTimeStatisticMgr::CUserPlayTimeStatisticMgr(void)
    CUserPlayTimeStatisticMgr();

    // IDA_EA: 0x86107EC
    // IDA_NAME: _ZN25CUserPlayTimeStatisticMgrD2Ev
    // DEMANGLED: CUserPlayTimeStatisticMgr::~CUserPlayTimeStatisticMgr()
    ~CUserPlayTimeStatisticMgr();

    // IDA_EA: 0x860CC38
    // IDA_NAME: _ZN25CUserPlayTimeStatisticMgr18UpdateUserPlayTimeEji
    // DEMANGLED: CUserPlayTimeStatisticMgr::UpdateUserPlayTime(unsigned int,int)
    int UpdateUserPlayTime(uint32_t account_id, int play_time);

    // IDA_EA: 0x860CD14
    // IDA_NAME: _ZN25CUserPlayTimeStatisticMgr18InsertUserPlayTimeEv
    // DEMANGLED: CUserPlayTimeStatisticMgr::InsertUserPlayTime(void)
    int InsertUserPlayTime();

private:
    std::map<uint32_t, stUserPlayInfo_t> entries_;
};

// IDA_EA: 0x860CBA5
// IDA_NAME: _Z35GetInstanceUserPlayTimeStatisticMgrv
// DEMANGLED: GetInstanceUserPlayTimeStatisticMgr(void)
CUserPlayTimeStatisticMgr* GetInstanceUserPlayTimeStatisticMgr();

#endif  // OLDSRV_OLD_GAME_USERINFO_USER_PLAY_TIME_STATISTIC_RUNTIME_H_
