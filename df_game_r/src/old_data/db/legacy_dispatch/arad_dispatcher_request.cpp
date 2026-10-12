#include "old_data/db/legacy_dispatch/arad_dispatcher_request.h"

namespace arad {

// IDA_EA: 0x822EE3C
// IDA_NAME: _ZN4arad22SigSetCharacCreateTimeC2Ev
// DEMANGLED: arad::SigSetCharacCreateTime::SigSetCharacCreateTime(void)
SigSetCharacCreateTime::SigSetCharacCreateTime() {
    init();
}

// IDA_EA: 0x822EE50
// IDA_NAME: _ZN4arad22SigSetCharacCreateTime4initEv
// DEMANGLED: arad::SigSetCharacCreateTime::init(void)
int SigSetCharacCreateTime::init() {
    set(0, 0);
    return 0;
}

// IDA_EA: 0x822EE74
// IDA_NAME: _ZN4arad22SigSetCharacCreateTime3setEjj
// DEMANGLED: arad::SigSetCharacCreateTime::set(unsigned int,unsigned int)
SigSetCharacCreateTime* SigSetCharacCreateTime::set(
    uint32_t charac_no_value, uint32_t create_time_value) {
    charac_no = charac_no_value;
    create_time = create_time_value;
    return this;
}

// IDA_EA: 0x817EDFC
// IDA_NAME: _ZN4arad29SigAradAvatarHiddenOptionSaveC2Ev
// DEMANGLED: arad::SigAradAvatarHiddenOptionSave::SigAradAvatarHiddenOptionSave(void)
SigAradAvatarHiddenOptionSave::SigAradAvatarHiddenOptionSave() {
    init();
}

// IDA_EA: 0x817EE10
// IDA_NAME: _ZN4arad29SigAradAvatarHiddenOptionSave4initEv
// DEMANGLED: arad::SigAradAvatarHiddenOptionSave::init(void)
SigAradAvatarHiddenOptionSave* SigAradAvatarHiddenOptionSave::init() {
    account_id = 0;
    charac_no = 0;
    item_id = 0;
    slot = 0;
    hidden_option = 0;
    return this;
}

// IDA_EA: 0x81933DE
// IDA_NAME: _ZN4arad26SigAradAvatarEnduranceSave4initEv
// DEMANGLED: arad::SigAradAvatarEnduranceSave::init(void)
SigAradAvatarEnduranceSave* SigAradAvatarEnduranceSave::init() {
    ui_id = 0;
    endurance = 0;
    return this;
}

// IDA_EA: 0x81933F6
// IDA_NAME: _ZN4arad26SigAradAvatarEnduranceSaveC2Ev
// DEMANGLED: arad::SigAradAvatarEnduranceSave::SigAradAvatarEnduranceSave(void)
SigAradAvatarEnduranceSave::SigAradAvatarEnduranceSave() {
    init();
}

}  // namespace arad
