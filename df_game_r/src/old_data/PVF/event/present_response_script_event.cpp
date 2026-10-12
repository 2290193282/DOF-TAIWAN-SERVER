#include "old_data/PVF/event/present_response_script_event.h"

// IDA_EA: 0x8A00B8A
// IDA_NAME: _ZN26PresentResponceScriptEventC2Ev
// DEMANGLED: PresentResponceScriptEvent::PresentResponceScriptEvent(void)
PresentResponceScriptEvent::PresentResponceScriptEvent()
    : value_00(0), text() {
    clear();
}

// IDA_EA: 0x8A00B6A
// IDA_NAME: _ZN26PresentResponceScriptEvent5clearEv
// DEMANGLED: PresentResponceScriptEvent::clear(void)
PresentResponceScriptEvent* PresentResponceScriptEvent::clear() {
    value_00 = 0;
    text.clear();
    return this;
}

// IDA_EA: 0x8A036F0
// IDA_NAME: _ZN26PresentResponceScriptEventC2ERKS_
// DEMANGLED: PresentResponceScriptEvent::PresentResponceScriptEvent(PresentResponceScriptEvent const&)
PresentResponceScriptEvent::PresentResponceScriptEvent(
    const PresentResponceScriptEvent& other)
    : value_00(other.value_00), text(other.text) {}

// IDA_EA: 0x8585564
// IDA_NAME: _ZN26PresentResponceScriptEventD2Ev
// DEMANGLED: PresentResponceScriptEvent::~PresentResponceScriptEvent()
PresentResponceScriptEvent::~PresentResponceScriptEvent() {}

// IDA_EA: 0x8A0379E
// IDA_NAME: _ZN26PresentResponceScriptEventaSERKS_
// DEMANGLED: PresentResponceScriptEvent::operator=(PresentResponceScriptEvent const&)
PresentResponceScriptEvent& PresentResponceScriptEvent::operator=(
    const PresentResponceScriptEvent& other) {
    value_00 = other.value_00;
    text = other.text;
    return *this;
}
