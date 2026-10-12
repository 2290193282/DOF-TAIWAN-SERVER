#ifndef OLDSRV_DATAMANAGER_IN_GAME_ADVERTISEMENT_SCRIPT_H_
#define OLDSRV_DATAMANAGER_IN_GAME_ADVERTISEMENT_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>


// IDA_EA: 0x83732F0, 0x8373304, 0x897E9C4.
// IDA unit: 12_pvf_datamanager_scripts/InGameAdvertisementScript.
struct InGameAdvertisementScript {
    InGameAdvertisementScript();

    void* clear();

    // +0x00: one condition byte for every age accepted by the original
    // importer (0..150).  clear() enables every age; the
    // [in game advertisement expose condition of teen] list disables the
    // explicitly listed ages.  CInGameAdvertisementManager::checkCondition
    // indexes this table directly with CUser::GetAge().
    uint8_t age_condition_flags[151];
};

static_assert(sizeof(InGameAdvertisementScript) == 0x97u,
              "InGameAdvertisementScript IDA ABI size must stay 0x97");
static_assert(offsetof(InGameAdvertisementScript, age_condition_flags) == 0x00u,
              "InGameAdvertisementScript age table must stay at +0x00");
static_assert(sizeof(((InGameAdvertisementScript*)0)->age_condition_flags) ==
                  0x97u,
              "InGameAdvertisementScript age table must contain 151 bytes");

int importInGameAdvertisementEtcScript(InGameAdvertisementScript* script,
                                       const char* filename);

#endif  // OLDSRV_DATAMANAGER_IN_GAME_ADVERTISEMENT_SCRIPT_H_
