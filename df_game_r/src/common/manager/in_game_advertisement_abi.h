#ifndef OLDSRV_GLOBAL_IN_GAME_ADVERTISEMENT_ABI_H_
#define OLDSRV_GLOBAL_IN_GAME_ADVERTISEMENT_ABI_H_

#include <cstddef>
#include <map>
#include <stdint.h>
#include <utility>
#include <vector>

#include "old_game/economy/inventory/inventory.h"

class CMTRand;
class CAdvertisementExposeInfo;
class CUser;

template <typename T>
class MemPool;


// IDA payload layout reconstructed from
// CInGameAdvertisementManager::queryADExposeInfo @0x80FD19E.  The names keep
// the database-column origin where the original business meaning is not yet
// proven; their offsets are the ABI contract.
#pragma pack(push, 1)

struct stNotifyIngameADInfo {
    uint32_t banner_id;            // +0x000
    char title[50];                // +0x004, DB column 1
    char raw_col6[255];            // +0x036, DB column 6
    char raw_col8[255];            // +0x135, DB column 8
    uint32_t raw_col7;             // +0x234, DB column 7
    char raw_col20[255];           // +0x238, DB column 20
    uint8_t raw_col15;             // +0x337, DB column 15
    uint8_t raw_col21;             // +0x338, DB column 21
    uint8_t condition_match;       // +0x339, written by checkCondition
    char raw_col9[255];            // +0x33a, DB column 9
    char raw_col10[255];           // +0x439, DB column 10
    uint32_t raw_col11;            // +0x538, DB column 11
    char raw_col12[255];           // +0x53c, DB column 12
    uint8_t raw_tail;              // +0x63b
};

struct stInGameADExposeCondition {
    char condition_type;           // +0x000, DB column 2
    char condition_value;          // +0x001, DB column 3
    char condition_flag;           // +0x002, DB column 4
    char level_filter[512];        // +0x003, DB column 5
    uint8_t probability_divisor;   // +0x203, DB column 17
    int16_t advertisement_kind;    // +0x204, DB column 18
    uint8_t raw_206[2];            // +0x206
    uint32_t min_cera;             // +0x208, DB column 13
    uint32_t max_cera;             // +0x20c, DB column 14
    stNotifyIngameADInfo notify;   // +0x210
};

struct SIG_RELOAD_IN_GAME_ADVERTISEMENT {
    stInGameADExposeCondition entries[200];
};

struct SIG_IN_GAME_ADVERTISEMENT_LOG {
    uint32_t m_id;                 // +0x00
    uint32_t charac_no;            // +0x04
    char state;                    // +0x08
    uint8_t pad_9;                 // +0x09
    int16_t view_code;             // +0x0a
    char type_code;                // +0x0c
    uint8_t pad_13[3];             // +0x0d
    uint32_t banner_id;            // +0x10
    int32_t dungeon_index;         // +0x14
    int32_t village_index;         // +0x18
    int32_t area_index;            // +0x1c
};

#pragma pack(pop)

static_assert(sizeof(stNotifyIngameADInfo) == 0x63cu,
              "stNotifyIngameADInfo old-server ABI size must stay 0x63C");
static_assert(offsetof(stNotifyIngameADInfo, raw_col6) == 0x36u,
              "stNotifyIngameADInfo column-6 offset changed");
static_assert(offsetof(stNotifyIngameADInfo, raw_col7) == 0x234u,
              "stNotifyIngameADInfo column-7 offset changed");
static_assert(offsetof(stNotifyIngameADInfo, raw_col20) == 0x238u,
              "stNotifyIngameADInfo column-20 offset changed");
static_assert(offsetof(stNotifyIngameADInfo, condition_match) == 0x339u,
              "stNotifyIngameADInfo condition-match offset changed");
static_assert(sizeof(stInGameADExposeCondition) == 0x84cu,
              "stInGameADExposeCondition old-server ABI size must stay 0x84C");
static_assert(offsetof(stInGameADExposeCondition, notify) == 0x210u,
              "stInGameADExposeCondition notify offset changed");
static_assert(sizeof(SIG_RELOAD_IN_GAME_ADVERTISEMENT) == 0x67b60u,
              "SIG_RELOAD_IN_GAME_ADVERTISEMENT must stay 424800 bytes");
static_assert(sizeof(SIG_IN_GAME_ADVERTISEMENT_LOG) == 0x20u,
              "SIG_IN_GAME_ADVERTISEMENT_LOG old-server ABI size must stay 32");


// IDA unit: 99_misc_global/CAdvertisementExposeInfo.  The allocation link is
// stored at +0x84c, immediately after the condition copy, exactly as the raw
// MemPool free-list implementation does.
#pragma pack(push, 1)
class CAdvertisementExposeInfo {
public:
    static void* operator new(std::size_t bytes);
    static void operator delete(void* entry, std::size_t bytes);
    static void operator delete(void* entry);

    void* SetAdvertisementExposeInfo(stInGameADExposeCondition& condition);
    stInGameADExposeCondition* getExposeCondition();

private:
    template <typename T>
    friend class MemPool;

    stInGameADExposeCondition condition_;       // +0x000
    CAdvertisementExposeInfo* free_list_next_;  // +0x84c on i386

    static MemPool<CAdvertisementExposeInfo> advertisementExposeInfoPool_;
};
#pragma pack(pop)

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(CAdvertisementExposeInfo) == 0x850u,
              "CAdvertisementExposeInfo old-server ABI size must stay 0x850");
#endif


// IDA unit: 99_misc_global/MemPool<CAdvertisementExposeInfo>.  This explicit
// specialization is the actual 50-entry, 0x850-byte free-list allocator used
// by the advertisement manager; it is not a generic replacement facade.
template <>
class MemPool<CAdvertisementExposeInfo> {
public:
    explicit MemPool(unsigned int count);
    ~MemPool();

    CAdvertisementExposeInfo* alloc();
    void free(void* entry, unsigned int bytes);
    void free(void* entry);

private:
    unsigned int size_;
    unsigned int count_;
    std::vector<void*> allocated_blocks_;

    static CAdvertisementExposeInfo* headOfFreeList_;
};


// IDA unit: 99_misc_global/CInGameAdvertisementManager.  The condition
// selector owns no surrogate user state: it reads CUser accessors and the
// named CDataManager age-policy table, then copies one persisted exposure
// record into the packet parameter.
class CInGameAdvertisementManager {
public:
    CInGameAdvertisementManager();
    ~CInGameAdvertisementManager();

    int InitADExposeInfo();
    int queryADExposeInfo(stInGameADExposeCondition* conditions);
    int LoadADExposeInfoAtApp();
    int InsertADExposeInfo(stInGameADExposeCondition& condition);
    int ReloadADExposeInfo();
    int checkLevel(CUser* user, char* level_filter);
    void CheckFailMultiBoxLottery(
        CUser* user,
        std::vector<std::pair<Inven_Item, bool> >& results);
    int checkCondition(CUser* user, CAdvertisementExposeInfo* info);
    int CheckIngameAdvertisementCondition(CUser* user,
                                          stNotifyIngameADInfo& output,
                                          int request_id);
    void WriteLog(CUser* user,
                  char state,
                  short view_code,
                  char type_code,
                  unsigned int banner_id);
    unsigned int RandNum(int maximum);

private:
    std::multimap<short, CAdvertisementExposeInfo*> expose_infos_;
    CMTRand* random_;
};

// IDA_EA: 0x80FC9E9
// IDA_NAME: _Z23InGameADManagerInstancev
// DEMANGLED: InGameADManagerInstance(void)
CInGameAdvertisementManager* InGameADManagerInstance();


#endif  // OLDSRV_GLOBAL_IN_GAME_ADVERTISEMENT_ABI_H_
