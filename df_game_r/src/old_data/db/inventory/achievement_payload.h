#ifndef OLDSRV_OLD_DATA_DB_INVENTORY_ACHIEVEMENT_PAYLOAD_H_
#define OLDSRV_OLD_DATA_DB_INVENTORY_ACHIEVEMENT_PAYLOAD_H_

#include <stddef.h>
#include <stdint.h>


// The achievement DB record is a fixed old-server wire image.  The fourth
// progress word is persisted but is not part of isComplete()'s three-trigger
// completion test.
struct stAchievement {
    // IDA_EA: 0x828D870
    // IDA_NAME: _ZN13stAchievementC2Ev
    // DEMANGLED: stAchievement::stAchievement(void)
    stAchievement();

    // IDA_EA: 0x828D8A2
    // IDA_NAME: _ZN13stAchievement10isCompleteEv
    // DEMANGLED: stAchievement::isComplete(void)
    bool isComplete();

    uint32_t achievement_id;  // +0x00
    uint16_t progress[4];      // +0x04
};

struct stAchievement_DBData {
    // IDA_EA: 0x828D8D8
    // IDA_NAME: _ZN20stAchievement_DBDataC2Ev
    // DEMANGLED: stAchievement_DBData::stAchievement_DBData(void)
    stAchievement_DBData();

    stAchievement achievements[512];  // +0x0000
    uint32_t count;                   // +0x1800
    uint32_t last_update_time;        // +0x1804
};

struct stAchievement_DBSaveData : public stAchievement_DBData {
    uint32_t charac_no;  // +0x1808
};

static_assert(sizeof(stAchievement) == 0x0Cu,
              "stAchievement old-server size must stay 0x0c");
static_assert(offsetof(stAchievement, progress) == 0x04u,
              "stAchievement progress offset must stay 0x04");
static_assert(sizeof(stAchievement_DBData) == 0x1808u,
              "stAchievement_DBData old-server size must stay 0x1808");
static_assert(offsetof(stAchievement_DBData, count) == 0x1800u,
              "achievement count offset must stay 0x1800");
static_assert(offsetof(stAchievement_DBData, last_update_time) == 0x1804u,
              "achievement update-time offset must stay 0x1804");
static_assert(sizeof(stAchievement_DBSaveData) == 0x180Cu,
              "stAchievement_DBSaveData old-server size must stay 0x180c");
static_assert(sizeof(stAchievement_DBSaveData) - sizeof(uint32_t) == 0x1808u,
              "achievement charac_no must follow the 0x1808-byte DB base");

#endif  // OLDSRV_OLD_DATA_DB_INVENTORY_ACHIEVEMENT_PAYLOAD_H_
