#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_AVATAR_PACKET_SIGNALS_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_AVATAR_PACKET_SIGNALS_H_

#include <stdint.h>

// These are queue-1 ARAD internal messages, not wire overlays.  Their first
// fields are populated by the C2S dispatchers and the service bodies fill the
// derived result fields before the internal dispatcher consumes them.
namespace arad {

#pragma pack(push, 1)

struct SigAvatarHiddenOptionChange {
    uint16_t material_slot;
    uint16_t avatar_slot;
    uint16_t hidden_option;

    // IDA_EA: 0x81A095C
    // IDA_NAME: _ZN4arad27SigAvatarHiddenOptionChangeC2Ev
    // DEMANGLED: arad::SigAvatarHiddenOptionChange::SigAvatarHiddenOptionChange(void)
    SigAvatarHiddenOptionChange();

    // IDA_EA: 0x81A0970
    // IDA_NAME: _ZN4arad27SigAvatarHiddenOptionChange4initEv
    // DEMANGLED: arad::SigAvatarHiddenOptionChange::init(void)
    SigAvatarHiddenOptionChange* init();
};

struct SigAradUseAvatarRecharge {
    uint16_t avatar_slot;
    uint16_t recharge_stone_slot;
    int32_t added_endurance;
    int32_t endurance_limit;
    int32_t current_endurance;

    // IDA_EA: 0x81A09F8
    // IDA_NAME: _ZN4arad24SigAradUseAvatarRechargeC2Ev
    // DEMANGLED: arad::SigAradUseAvatarRecharge::SigAradUseAvatarRecharge(void)
    SigAradUseAvatarRecharge();

    // IDA_EA: 0x81A09C4
    // IDA_NAME: _ZN4arad24SigAradUseAvatarRecharge4initEv
    // DEMANGLED: arad::SigAradUseAvatarRecharge::init(void)
    SigAradUseAvatarRecharge* init();
};

struct SigAradEmblemCompound {
    struct materialInfo {
        uint16_t slot;
        uint16_t grade;
        uint32_t item_index;

        // IDA_EA: 0x81A0A0C
        // IDA_NAME: _ZN4arad21SigAradEmblemCompound12materialInfo4initEv
        // DEMANGLED: arad::SigAradEmblemCompound::materialInfo::init(void)
        materialInfo* init();
    };

    materialInfo materials[2];
    uint32_t result_item_index;
    uint16_t select_no;

    // IDA_EA: 0x81A0A2C
    // IDA_NAME: _ZN4arad21SigAradEmblemCompoundC2Ev
    // DEMANGLED: arad::SigAradEmblemCompound::SigAradEmblemCompound(void)
    SigAradEmblemCompound();

    // IDA_EA: 0x81A0A40
    // IDA_NAME: _ZN4arad21SigAradEmblemCompound4initEv
    // DEMANGLED: arad::SigAradEmblemCompound::init(void)
    materialInfo* init();
};

struct SigAradAvatarConvert {
    uint16_t select_no;
    uint16_t avatar_slot;
    uint32_t input_item_index;
    uint32_t result_item_index;
    uint16_t result_count;

    // IDA_EA: 0x81A0A6A
    // IDA_NAME: _ZN4arad20SigAradAvatarConvertC2Ev
    // DEMANGLED: arad::SigAradAvatarConvert::SigAradAvatarConvert(void)
    SigAradAvatarConvert();

    // IDA_EA: 0x81A0A7E
    // IDA_NAME: _ZN4arad20SigAradAvatarConvert4initEv
    // DEMANGLED: arad::SigAradAvatarConvert::init(void)
    SigAradAvatarConvert* init();
};

#pragma pack(pop)

static_assert(sizeof(SigAvatarHiddenOptionChange) == 0x06u,
              "SigAvatarHiddenOptionChange IDA ABI size must stay 0x06");
static_assert(sizeof(SigAradUseAvatarRecharge) == 0x10u,
              "SigAradUseAvatarRecharge IDA ABI size must stay 0x10");
static_assert(sizeof(SigAradEmblemCompound::materialInfo) == 0x08u,
              "SigAradEmblemCompound::materialInfo IDA ABI size must stay 0x08");
static_assert(sizeof(SigAradEmblemCompound) == 0x16u,
              "SigAradEmblemCompound IDA ABI size must stay 0x16");
static_assert(sizeof(SigAradAvatarConvert) == 0x0eu,
              "SigAradAvatarConvert IDA ABI size must stay 0x0e");

}  // namespace arad

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_AVATAR_PACKET_SIGNALS_H_
