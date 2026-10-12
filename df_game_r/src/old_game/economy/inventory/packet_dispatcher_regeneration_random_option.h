#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_REGENERATION_RANDOM_OPTION_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_REGENERATION_RANDOM_OPTION_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 99_misc_global/MSG_REGENERATION_RANDOM_OPTION.  The raw reader
// consumes four adjacent shorts after the 13-byte C2S header; no named C2 was
// emitted for this POD stack message.
#pragma pack(push, 1)
struct MSG_REGENERATION_RANDOM_OPTION : public MSG_BASE {
    int16_t first_item_slot;   // +0x0d
    int16_t second_item_slot;  // +0x0f
    int16_t select_part;       // +0x11
    int16_t regeneration_mode; // +0x13
};
#pragma pack(pop)

static_assert(sizeof(MSG_REGENERATION_RANDOM_OPTION) == 0x15u,
              "CMD407 message must retain the original 21-byte stack extent");
static_assert(offsetof(MSG_REGENERATION_RANDOM_OPTION, first_item_slot) ==
                  0x0du,
              "CMD407 first inventory slot must stay at +0x0d");
static_assert(offsetof(MSG_REGENERATION_RANDOM_OPTION, regeneration_mode) ==
                  0x13u,
              "CMD407 regeneration mode must stay at +0x13");

// IDA unit: 99_misc_global/stRegenROIResult.
struct stRegenROIResult : public ParamBase {
    // IDA_EA: 0x82316D0
    // IDA_NAME: _ZN16stRegenROIResultC2Ev
    stRegenROIResult();

    // IDA_EA: 0x8246E1A / 0x8246E4A
    // IDA_NAME: _ZN16stRegenROIResultD2Ev / D0Ev
    virtual ~stRegenROIResult();

    int32_t result_code;  // +0x04
    uint16_t item_slot;   // +0x08
};

#if defined(__i386__)
static_assert(sizeof(stRegenROIResult) == 0x0cu,
              "CMD407 result parameter must retain its i386 ABI");
static_assert(offsetof(stRegenROIResult, result_code) == 0x04u,
              "CMD407 result code must stay at +0x04");
static_assert(offsetof(stRegenROIResult, item_slot) == 0x08u,
              "CMD407 result item slot must stay at +0x08");
#endif

// PacketDispatcher slot 407.  It is a recovered leaf provider; constructor
// table activation remains intentionally out of scope while the 606-slot root
// is still frozen.
class Dispather_RegenerationRandomOption
    : public IPacketDispatcher<MSG_REGENERATION_RANDOM_OPTION,
                               stRegenROIResult,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85977C2
    // IDA_NAME: _ZN34Dispather_RegenerationRandomOptionC2Ev
    Dispather_RegenerationRandomOption();

    // IDA_EA: 0x8244D64 / 0x8244D94
    // IDA_NAME: _ZN34Dispather_RegenerationRandomOptionD2Ev / D0Ev
    virtual ~Dispather_RegenerationRandomOption();

    // IDA_EA: 0x81D69D2
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    // IDA_EA: 0x81D6AF4
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    // IDA_EA: 0x81D7278
    virtual void send(CUser* user, ParamBase& param);
    // IDA_EA: 0x8232984
    virtual int check_state(CUser* user, PacketBuf& packet);
};

#if defined(__i386__)
static_assert(sizeof(Dispather_RegenerationRandomOption) == 0x04u,
              "CMD407 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_REGENERATION_RANDOM_OPTION_H_
