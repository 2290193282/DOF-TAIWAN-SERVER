#ifndef OLDSRV_OLD_DATA_PVF_EVENT_PRESENT_RESPONSE_SCRIPT_EVENT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_PRESENT_RESPONSE_SCRIPT_EVENT_H_

#include <stdint.h>

#include <string>

// IDA_EA: 0x8585564, 0x8A00B6A, 0x8A00B8A, 0x8A036F0, 0x8A0379E.
// This NPC/PVF dialog record is shared with event runtime consumers, but its
// parser and lifecycle are owned by DataManager.
struct PresentResponceScriptEvent {
    PresentResponceScriptEvent();
    PresentResponceScriptEvent(const PresentResponceScriptEvent& other);
    ~PresentResponceScriptEvent();

    PresentResponceScriptEvent* clear();
    PresentResponceScriptEvent& operator=(
        const PresentResponceScriptEvent& other);

    int32_t value_00;
    std::string text;
};

static_assert(sizeof(PresentResponceScriptEvent) == 0x08u,
              "PresentResponceScriptEvent IDA ABI size must stay 0x08");

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_PRESENT_RESPONSE_SCRIPT_EVENT_H_
