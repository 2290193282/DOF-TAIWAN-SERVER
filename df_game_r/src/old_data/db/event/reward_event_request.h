#ifndef OLDSRV_DB_REWARD_EVENT_REQUEST_H_
#define OLDSRV_DB_REWARD_EVENT_REQUEST_H_

#include <stddef.h>
#include <stdint.h>

// DB dispatcher payloads for the ARAD event-reward save/load flow.  These are
// packed wire/database records, not event-runtime state objects.
#pragma pack(push, 1)

namespace arad {

struct SigSaveRewardEventItem {
    SigSaveRewardEventItem();

    SigSaveRewardEventItem* init();
    SigSaveRewardEventItem* set(int event_code_value,
                                uint32_t dword_04_value,
                                uint32_t dword_08_value,
                                unsigned long dword_10_value,
                                int16_t word_14_value,
                                uint32_t dword_0c_value);

    int32_t event_code;
    uint32_t dword_04;
    uint32_t dword_08;
    uint32_t dword_0c;
    uint32_t dword_10;
    int16_t word_14;
};

struct SigLoadRewardEventItem {
    SigLoadRewardEventItem();

    SigLoadRewardEventItem* init();
    SigLoadRewardEventItem* set(uint32_t account_id_value);

    uint32_t account_id;
    int16_t reward_count;
    int32_t event_codes[10];
    uint32_t charac_nos[10];
};

struct SigAradEventItemLog {
    SigAradEventItemLog();

    SigAradEventItemLog* init();

    uint32_t item_id;      // +0x00
    int32_t item_count;    // +0x04
    uint32_t account_id;   // +0x08
    uint32_t charac_no;    // +0x0c
};

}  // namespace arad

#pragma pack(pop)

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(arad::SigSaveRewardEventItem) == 0x16u,
              "SigSaveRewardEventItem IDA ABI size must stay 0x16");
static_assert(sizeof(arad::SigLoadRewardEventItem) == 0x56u,
              "SigLoadRewardEventItem IDA ABI size must stay 0x56");
static_assert(sizeof(arad::SigAradEventItemLog) == 0x10u,
              "SigAradEventItemLog IDA ABI size must stay 0x10");
static_assert(offsetof(arad::SigAradEventItemLog, item_id) == 0x00u &&
                  offsetof(arad::SigAradEventItemLog, item_count) == 0x04u &&
                  offsetof(arad::SigAradEventItemLog, account_id) == 0x08u &&
                  offsetof(arad::SigAradEventItemLog, charac_no) == 0x0cu,
              "SigAradEventItemLog field offsets must stay exact");
#endif

#endif  // OLDSRV_DB_REWARD_EVENT_REQUEST_H_
