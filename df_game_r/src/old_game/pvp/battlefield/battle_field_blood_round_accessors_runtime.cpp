#include "old_game/pvp/battlefield/battle_field.h"

// IDA_EA: 0x830EB72
// IDA_NAME: _ZN13CBattle_Field11CBloodRound18setBloodRoundOrderEv
// DEMANGLED: CBattle_Field::CBloodRound::setBloodRoundOrder(void)
CBattle_Field::CBloodRound*
CBattle_Field::CBloodRound::setBloodRoundOrder() {
    blood_round_order = pre_map_blood_round_order;
    return this;
}

// IDA_EA: 0x830EB84
// IDA_NAME: _ZN13CBattle_Field11CBloodRound24setPreMapBloodRoundOrderEv
// DEMANGLED: CBattle_Field::CBloodRound::setPreMapBloodRoundOrder(void)
CBattle_Field::CBloodRound*
CBattle_Field::CBloodRound::setPreMapBloodRoundOrder() {
    pre_map_blood_round_order = blood_round_order;
    return this;
}

// IDA_EA: 0x830EB96
// IDA_NAME: _ZN13CBattle_Field11CBloodRound27setBloodLastSmallRoundOrderEi
// DEMANGLED: CBattle_Field::CBloodRound::setBloodLastSmallRoundOrder(int)
CBattle_Field::CBloodRound*
CBattle_Field::CBloodRound::setBloodLastSmallRoundOrder(int small_round) {
    blood_last_small_round_order = static_cast<uint16_t>(small_round);
    return this;
}

// IDA_EA: 0x830EBA8
// IDA_NAME: _ZN13CBattle_Field11CBloodRound23setBloodSmallRoundOrderEs
// DEMANGLED: CBattle_Field::CBloodRound::setBloodSmallRoundOrder(short)
CBattle_Field::CBloodRound*
CBattle_Field::CBloodRound::setBloodSmallRoundOrder(short small_round) {
    blood_small_round_order = static_cast<uint16_t>(small_round);
    return this;
}

// IDA_EA: 0x830EBC2
// IDA_NAME: _ZN13CBattle_Field11CBloodRound18getBloodRoundOrderEv
// DEMANGLED: CBattle_Field::CBloodRound::getBloodRoundOrder(void)
int CBattle_Field::CBloodRound::getBloodRoundOrder() {
    return blood_round_order;
}

// IDA_EA: 0x830EBCE
// IDA_NAME: _ZN13CBattle_Field11CBloodRound23getBloodSmallRoundOrderEv
// DEMANGLED: CBattle_Field::CBloodRound::getBloodSmallRoundOrder(void)
int CBattle_Field::CBloodRound::getBloodSmallRoundOrder() {
    return blood_small_round_order;
}

// IDA_EA: 0x830EBDA
// IDA_NAME: _ZN13CBattle_Field11CBloodRound27getBloodLastSmallRoundOrderEv
// DEMANGLED: CBattle_Field::CBloodRound::getBloodLastSmallRoundOrder(void)
int CBattle_Field::CBloodRound::getBloodLastSmallRoundOrder() {
    return blood_last_small_round_order;
}

// IDA_EA: 0x830EBE6
// IDA_NAME: _ZN13CBattle_Field11CBloodRound18incBloodRoundOrderEv
// DEMANGLED: CBattle_Field::CBloodRound::incBloodRoundOrder(void)
CBattle_Field::CBloodRound*
CBattle_Field::CBloodRound::incBloodRoundOrder() {
    ++blood_round_order;
    return this;
}

// IDA_EA: 0x830EBFC
// IDA_NAME: _ZN13CBattle_Field11CBloodRound23incBloodSmallRoundOrderEv
// DEMANGLED: CBattle_Field::CBloodRound::incBloodSmallRoundOrder(void)
CBattle_Field::CBloodRound*
CBattle_Field::CBloodRound::incBloodSmallRoundOrder() {
    ++blood_small_round_order;
    return this;
}
