#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_ENCHANT_BY_BEAD_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_ENCHANT_BY_BEAD_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 99_misc_global/MSG_ENCHANT_BY_BEAD.  The concrete CMD275 reader
// writes these four payload fields immediately after MSG_BASE.  No separate
// message C2 exists in the old image: the template keeps this 20-byte stack
// storage unconstructed before read().
struct MSG_ENCHANT_BY_BEAD : public MSG_BASE {
    uint8_t source_item_space;  // +0x0d
    uint16_t bead_slot;         // +0x0e
    uint8_t target_item_space;  // +0x10
    uint16_t target_slot;       // +0x12
};

static_assert(sizeof(MSG_ENCHANT_BY_BEAD) == 0x14u,
              "CMD275 request must retain IDA's 20-byte stack extent");
static_assert(offsetof(MSG_ENCHANT_BY_BEAD, source_item_space) == 0x0du &&
                  offsetof(MSG_ENCHANT_BY_BEAD, bead_slot) == 0x0eu &&
                  offsetof(MSG_ENCHANT_BY_BEAD, target_item_space) == 0x10u &&
                  offsetof(MSG_ENCHANT_BY_BEAD, target_slot) == 0x12u,
              "CMD275 request field offsets changed");

// IDA unit: 99_misc_global/stEnchantByBeadParam.
class stEnchantByBeadParam : public ParamBase {
public:
    // IDA_EA: 0x823148E
    // IDA_NAME: _ZN20stEnchantByBeadParamC2Ev
    stEnchantByBeadParam();

    // IDA_EA: 0x824725E / 0x824728E
    // IDA_NAME: _ZN20stEnchantByBeadParamD2Ev / D0Ev
    virtual ~stEnchantByBeadParam();

    int result;  // +0x04
};

// IDA unit: 13_database_dispatch/Dispatcher_EnchantByBead (CMD275).
class Dispatcher_EnchantByBead
    : public IPacketDispatcher<MSG_ENCHANT_BY_BEAD,
                               stEnchantByBeadParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8596EE0
    // IDA_NAME: _ZN24Dispatcher_EnchantByBeadC2Ev
    Dispatcher_EnchantByBead();

    // IDA_EA: 0x8245688 / 0x82456B8
    // IDA_NAME: _ZN24Dispatcher_EnchantByBeadD2Ev / D0Ev
    virtual ~Dispatcher_EnchantByBead();

    // IDA_EA: 0x81D2218 / 0x81D24B8 / 0x81D25B0.
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81D233A.
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(stEnchantByBeadParam) == 0x08u,
              "CMD275 parameter must retain the old i386 ABI");
static_assert(offsetof(stEnchantByBeadParam, result) == 0x04u,
              "CMD275 parameter result must stay at +0x04");
static_assert(sizeof(Dispatcher_EnchantByBead) == 0x04u,
              "CMD275 dispatcher must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_ENCHANT_BY_BEAD_H_
