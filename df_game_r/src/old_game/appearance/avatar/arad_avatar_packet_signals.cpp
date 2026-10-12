#include "old_game/appearance/avatar/arad_avatar_packet_signals.h"

namespace arad {

// IDA_EA: 0x81A095C
// IDA_NAME: _ZN4arad27SigAvatarHiddenOptionChangeC2Ev
// DEMANGLED: arad::SigAvatarHiddenOptionChange::SigAvatarHiddenOptionChange(void)
SigAvatarHiddenOptionChange::SigAvatarHiddenOptionChange() {
    init();
}

// IDA_EA: 0x81A0970
// IDA_NAME: _ZN4arad27SigAvatarHiddenOptionChange4initEv
// DEMANGLED: arad::SigAvatarHiddenOptionChange::init(void)
SigAvatarHiddenOptionChange* SigAvatarHiddenOptionChange::init() {
    material_slot = 0;
    avatar_slot = 0;
    hidden_option = 0;
    return this;
}

// IDA_EA: 0x81A09C4
// IDA_NAME: _ZN4arad24SigAradUseAvatarRecharge4initEv
// DEMANGLED: arad::SigAradUseAvatarRecharge::init(void)
SigAradUseAvatarRecharge* SigAradUseAvatarRecharge::init() {
    avatar_slot = 0;
    recharge_stone_slot = 0;
    added_endurance = 0;
    endurance_limit = 0;
    current_endurance = 0;
    return this;
}

// IDA_EA: 0x81A09F8
// IDA_NAME: _ZN4arad24SigAradUseAvatarRechargeC2Ev
// DEMANGLED: arad::SigAradUseAvatarRecharge::SigAradUseAvatarRecharge(void)
SigAradUseAvatarRecharge::SigAradUseAvatarRecharge() {
    init();
}

// IDA_EA: 0x81A0A0C
// IDA_NAME: _ZN4arad21SigAradEmblemCompound12materialInfo4initEv
// DEMANGLED: arad::SigAradEmblemCompound::materialInfo::init(void)
SigAradEmblemCompound::materialInfo*
SigAradEmblemCompound::materialInfo::init() {
    slot = 0;
    grade = 0;
    item_index = 0;
    return this;
}

// IDA_EA: 0x81A0A2C
// IDA_NAME: _ZN4arad21SigAradEmblemCompoundC2Ev
// DEMANGLED: arad::SigAradEmblemCompound::SigAradEmblemCompound(void)
SigAradEmblemCompound::SigAradEmblemCompound() {
    init();
}

// IDA_EA: 0x81A0A40
// IDA_NAME: _ZN4arad21SigAradEmblemCompound4initEv
// DEMANGLED: arad::SigAradEmblemCompound::init(void)
SigAradEmblemCompound::materialInfo* SigAradEmblemCompound::init() {
    // The original only clears select_no and each material record.  The
    // service computes result_item_index later, so this member remains intact.
    select_no = 0;
    materials[0].init();
    return materials[1].init();
}

// IDA_EA: 0x81A0A6A
// IDA_NAME: _ZN4arad20SigAradAvatarConvertC2Ev
// DEMANGLED: arad::SigAradAvatarConvert::SigAradAvatarConvert(void)
SigAradAvatarConvert::SigAradAvatarConvert() {
    init();
}

// IDA_EA: 0x81A0A7E
// IDA_NAME: _ZN4arad20SigAradAvatarConvert4initEv
// DEMANGLED: arad::SigAradAvatarConvert::init(void)
SigAradAvatarConvert* SigAradAvatarConvert::init() {
    select_no = 0;
    avatar_slot = 0;
    input_item_index = 0;
    result_item_index = 0;
    result_count = 0;
    return this;
}

}  // namespace arad
