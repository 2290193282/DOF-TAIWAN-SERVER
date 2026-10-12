#ifndef OLDSRV_GLOBAL_CRASHDOWN_ABI_H_
#define OLDSRV_GLOBAL_CRASHDOWN_ABI_H_

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "infra/network/packet/PacketHeader.h"
#include "infra/concurrent/thread/mutex.h"


class DB_CrashDownReport;

// IDA unit: 99_misc_global/StatisticsToObserveCrash.
// This is an overlay for the object already constructed in GameWorld at +0x28
// by the canonical world lifecycle provider.  It is deliberately not a second
// singleton or a replacement GameWorld allocation.
class StatisticsToObserveCrash {
public:
    int lock();
    int unlock();

    StatisticsToObserveCrash();
    ~StatisticsToObserveCrash();

    int reset();
    int getAbnormalDownUserCount();
    int setAbnormalDownUserCount(int count);
    int incAbnormalDownUserCount();
    int incUserLastCrashPacketID(int packet_id);
    int getUserLastCrashPacketID(int packet_id);
    int incUserLastCrashDungeonID(int dungeon_id);
    int getUserLastCrashDungeonID(int dungeon_id);
    int incUserLastCrashPlayStatus(int play_status);
    int getUserLastCrashPlayStatus(int play_status);

private:
    friend class DB_CrashDownReport;

    int abnormal_down_user_count_;                 // +0x00
    Mutex mutex_;                                  // +0x04
    std::map<int, int> packet_counts_;             // +0x1c
    std::map<int, int> dungeon_counts_;            // +0x34
    std::map<int, int> play_status_counts_;        // +0x4c
};

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(StatisticsToObserveCrash) == 0x64u,
              "StatisticsToObserveCrash old-server ABI size must stay 0x64");
#endif


#pragma pack(push, 1)

// IDA unit: 99_misc_global/Packet_Reason_Crash_Down_Info.
struct Packet_Reason_Crash_Down_Info {
    PacketHeader header;       // +0x00, packet type 0xC48, size 0x1A
    uint32_t occ_time;         // +0x0a
    uint32_t channel_no;       // +0x0e
    uint32_t reason;           // +0x12
    uint32_t count;            // +0x16

    Packet_Reason_Crash_Down_Info();
};

#pragma pack(pop)

static_assert(sizeof(Packet_Reason_Crash_Down_Info) == 0x1au,
              "Packet_Reason_Crash_Down_Info old-server ABI size must stay 0x1A");
static_assert(offsetof(Packet_Reason_Crash_Down_Info, occ_time) == 0x0au,
              "Packet_Reason_Crash_Down_Info occ_time offset changed");
static_assert(offsetof(Packet_Reason_Crash_Down_Info, count) == 0x16u,
              "Packet_Reason_Crash_Down_Info count offset changed");


// IDA unit: 99_misc_global/CClientCrashDownLogStatistic.
class CClientCrashDownLogStatistic {
public:
    struct stClientCrashDownLog_t {
        stClientCrashDownLog_t();
        stClientCrashDownLog_t(const stClientCrashDownLog_t& rhs);
        ~stClientCrashDownLog_t();
        stClientCrashDownLog_t& operator=(const stClientCrashDownLog_t& rhs);

        int32_t occ_time_;                    // +0x00 on i386
        std::map<int, int> reason_counts_;    // +0x04 on i386

    private:
        friend class CClientCrashDownLogStatistic;
    };

    CClientCrashDownLogStatistic();
    ~CClientCrashDownLogStatistic();

    int RegisterClientCrashDown(int reason);
    int CarryClientCrashDown();
    void MakeClientCrashDownQuery(std::vector<std::string>& queries,
                                  int prefix_length);
    int UpdateClientCrashDown();
    void Init();

private:
    std::map<int, int> current_reason_counts_;             // +0x00
    std::vector<stClientCrashDownLog_t> carried_records_;  // +0x18 on i386
};

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(CClientCrashDownLogStatistic::stClientCrashDownLog_t) ==
                  0x1cu,
              "stClientCrashDownLog_t old-server ABI size must stay 0x1C");
static_assert(sizeof(CClientCrashDownLogStatistic) == 0x24u,
              "CClientCrashDownLogStatistic old-server ABI size must stay 0x24");
#endif

// IDA_EA: 0x860CFA5
// IDA_NAME: _Z38GetInstanceClientCrashDownLogStatisticv
// DEMANGLED: GetInstanceClientCrashDownLogStatistic(void)
CClientCrashDownLogStatistic* GetInstanceClientCrashDownLogStatistic();


#endif  // OLDSRV_GLOBAL_CRASHDOWN_ABI_H_
