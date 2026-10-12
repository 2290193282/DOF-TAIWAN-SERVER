#ifndef OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_DB_PAYLOADS_H_
#define OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_DB_PAYLOADS_H_

#include <stddef.h>
#include <stdint.h>

// These two signals already have canonical owners.  Keep a single physical
// ABI declaration for each rather than shadowing them in the DB handlers.
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/activity/schedule/event_messages.h"

namespace Taiwan {

// IDA_EA: init/set/setData @0x815DE9C/0x815DECC/0x815DF06.
// One packet carries at most 45 16-byte country codes.
struct SigLoadGeolocation {
    uint32_t count;                 // +0x00
    char country[45][16];           // +0x04

    void* init();
    SigLoadGeolocation* set(char* source);
    void* setData(SigLoadGeolocation& source);
};

// IDA_EA: init/set @0x815DF38/0x815DF50.
struct SigUpdateExitTime {
    uint32_t account_id;            // +0x00
    uint32_t expire_time;           // +0x04

    SigUpdateExitTime* init();
    SigUpdateExitTime* set(uint32_t account, uint32_t timestamp);
};

// IDA only proves an allocated four-byte DB-queue payload: the producer
// reserves it and writes no field before queueing ResetStayTimeEvent type 7.
// Name the storage rather than exposing a raw byte facade.
struct SigResetStayTimeEvent {
    uint32_t reserved;              // +0x00, observed unwritten
};

// IDA_EA: C2/init @0x81639F0/0x8163A04.
struct SigLoadHeroMissionData {
    uint32_t charac_no;             // +0x00
    char mission[0x100];            // +0x04

    SigLoadHeroMissionData();
    void* init();
};

// IDA_EA: C2/init @0x81639AC/0x81639C0.
struct SigSaveHeroMissionData {
    uint32_t charac_no;             // +0x00
    char mission[0x100];            // +0x04

    SigSaveHeroMissionData();
    void* init();
};

}  // namespace Taiwan

static_assert(sizeof(Taiwan::SigLoadRestrictPolicy) == 0x1200u,
              "SigLoadRestrictPolicy must retain the old 0x1200-byte ABI");
static_assert(offsetof(Taiwan::SigLoadRestrictPolicy, values) == 0x119Cu,
              "SigLoadRestrictPolicy value bytes must begin at +0x119C");
static_assert(sizeof(Taiwan::SigStayTimeEvent) == 0x04u,
              "SigStayTimeEvent must retain the old 4-byte ABI");
static_assert(sizeof(Taiwan::SigLoadGeolocation) == 0x2D4u,
              "SigLoadGeolocation must retain the old 0x2D4-byte ABI");
static_assert(offsetof(Taiwan::SigLoadGeolocation, country) == 0x04u,
              "SigLoadGeolocation country table must begin at +0x04");
static_assert(sizeof(Taiwan::SigUpdateExitTime) == 0x08u,
              "SigUpdateExitTime must retain the old 8-byte ABI");
static_assert(sizeof(Taiwan::SigResetStayTimeEvent) == 0x04u,
              "SigResetStayTimeEvent must retain the observed 4-byte ABI");
static_assert(sizeof(Taiwan::SigLoadHeroMissionData) == 0x104u,
              "SigLoadHeroMissionData must retain the old 0x104-byte ABI");
static_assert(sizeof(Taiwan::SigSaveHeroMissionData) == 0x104u,
              "SigSaveHeroMissionData must retain the old 0x104-byte ABI");

#endif  // OLDSRV_OLD_DATA_DB_TAIWAN_TAIWAN_DB_PAYLOADS_H_
