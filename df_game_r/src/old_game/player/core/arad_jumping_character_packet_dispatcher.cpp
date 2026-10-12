#include "old_game/player/core/arad_jumping_character_packet_dispatcher.h"

#include "old_data/db/legacy_dispatch/arad_dispatcher.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace arad {

// IDA_EA: 0x81A0990
// IDA_NAME: _ZN4arad23SigAradJumpingCharacterC2Ev
// DEMANGLED: arad::SigAradJumpingCharacter::SigAradJumpingCharacter(void)
SigAradJumpingCharacter::SigAradJumpingCharacter() {
    init();
}

// IDA_EA: 0x81A09A4
// IDA_NAME: _ZN4arad23SigAradJumpingCharacter4initEv
// DEMANGLED: arad::SigAradJumpingCharacter::init(void)
SigAradJumpingCharacter* SigAradJumpingCharacter::init() {
    // The old instructions clear precisely +0x00, +0x01..+0x04 and
    // +0x05..+0x08.  Do not clear the following C2S suffix: it is populated
    // one field at a time by Dispatcher_AradJumpingCharacter::dispatch_sig.
    reserved_zero = 0;
    acc_id = 0;
    charac_no = 0;
    return this;
}

}  // namespace arad

namespace ARAD {
namespace DISPATCHER {

// IDA_EA: 0x8597BCE
// IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacterC2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradJumpingCharacter::Dispatcher_AradJumpingCharacter(void)
Dispatcher_AradJumpingCharacter::Dispatcher_AradJumpingCharacter()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81A0DA2
// IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacterD2Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradJumpingCharacter::~Dispatcher_AradJumpingCharacter()
// IDA_EA: 0x81A0DD2
// IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacterD0Ev
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradJumpingCharacter::~Dispatcher_AradJumpingCharacter()
Dispatcher_AradJumpingCharacter::~Dispatcher_AradJumpingCharacter() {}

// IDA_EA: 0x819F1EC
// IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacter12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradJumpingCharacter::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AradJumpingCharacter::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    if (user == 0) {
        return 1435;
    }
    if (user->getCurCharacR() == 0) {
        return 0;
    }

    arad::SigAradJumpingCharacter payload;
    if (packet.get_byte(payload.ticket_type) != 1) {
        return 1449;
    }
    if (payload.ticket_type == 0 || payload.ticket_type > 2u) {
        return 1452;
    }
    if (packet.get_byte(payload.grow_type) != 1) {
        return 1455;
    }
    if (payload.grow_type == 0 || payload.grow_type > 4u) {
        return 1457;
    }
    if (packet.get_short(payload.material_slot) != 1) {
        return 1460;
    }
    if (packet.get_int(payload.opaque_client_value) != 1) {
        return 1462;
    }
    if (user->get_state() <= 2 || user->get_state() > 4) {
        return 0;
    }
    if (checkWorkState(user, payload) != 1) {
        return 0;
    }

    payload.acc_id = static_cast<uint32_t>(user->get_acc_id());
    payload.charac_no = static_cast<uint32_t>(user->get_charac_no(-1));

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                           "localjapan/Arad_PacketDispatcher.cpp", 1480),
                       true);
    // The old stream carries the distinct DB outer signal 0x2dc followed by
    // uid and this inner code.  Code 45 is the jumping-character consumer.
    make_internal_stream_jpn(
        guard, static_cast<ENUM_INTERNALPACKET_JPN>(45), user->GetUID());
    guard->put_binary(&payload, static_cast<int>(sizeof(payload)));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DISPATCH_Q, guard);
    return 0;
}

// IDA_EA: 0x819F44C
// IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacter14checkWorkStateEP5CUserRN4arad23SigAradJumpingCharacterE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradJumpingCharacter::checkWorkState(CUser *,arad::SigAradJumpingCharacter &)
int Dispatcher_AradJumpingCharacter::checkWorkState(
    CUser* user,
    arad::SigAradJumpingCharacter& payload) {
    const int protection_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(42));
    if (protection_result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_USE_JUMPING_CHAR_TICKET,
            static_cast<unsigned char>(protection_result));
        return 0;
    }

    if (payload.ticket_type == 2 && !user->HasCharacterLevelAbove(59)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_JUMPING_CHAR_TICKET, 8);
        return 0;
    }
    if (user->get_charac_level() > 17) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_JUMPING_CHAR_TICKET, 18);
        return 0;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    const Inven_Item material = inventory->GetInvenSlot(
        1, static_cast<int>(payload.material_slot));
    const int expected_item = payload.ticket_type == 1 ? 2675816 : 2675817;
    if (material.isEmpty() ||
        (payload.ticket_type != 1 && payload.ticket_type != 2) ||
        material.getKey() != expected_item) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_JUMPING_CHAR_TICKET, 17);
        return 0;
    }
    return 1;
}

// IDA_EA: 0x81A0B22
// IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacter4readER9PacketBufR8MSG_BASE
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradJumpingCharacter::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AradJumpingCharacter::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81A0B2C
// IDA_NAME: _ZN4ARAD10DISPATCHER31Dispatcher_AradJumpingCharacter7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: ARAD::DISPATCHER::Dispatcher_AradJumpingCharacter::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AradJumpingCharacter::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

}  // namespace DISPATCHER
}  // namespace ARAD
