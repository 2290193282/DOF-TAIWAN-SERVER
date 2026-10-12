#include "old_data/Login/packet_dispatch/teleport_dispatcher.h"

#include "common/manager/global_tools.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const ENUM_CMDPACKET kTeleportCommand = ENUM_CMDPACKET_TELEPORT;
const int kTeleportPotionItemId = 2600014;
const int kSuppressReply = 0x7fffffff;
const char kTeleportReadSignature[] =
    "virtual int Dispatcher_Teleport::read(PacketBuf&, MSG_BASE&)";
const char kTeleportProcessSignature[] =
    "virtual int Dispatcher_Teleport::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x8231362
// IDA_NAME: _ZN15stTeleportParamC2Ev
// DEMANGLED: stTeleportParam::stTeleportParam(void)
stTeleportParam::stTeleportParam()
    : ParamBase(),
      result(0),
      stackable_slot(0),
      padding_0a(0),
      request_item_token(0),
      item_space(0),
      target_village(0),
      padding_12(0),
      consumed_item_id(0) {}

// IDA_EA: 0x8247480 / 0x82474B0
// IDA_NAME: _ZN15stTeleportParamD2Ev / _ZN15stTeleportParamD0Ev
// DEMANGLED: stTeleportParam::~stTeleportParam()
stTeleportParam::~stTeleportParam() {}

// IDA_EA: 0x8253E2E
// IDA_NAME: _ZN17IPacketDispatcherI12MSG_TELEPORT15stTeleportParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_TELEPORT,stTeleportParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// The old body allocates the 0x16-byte request directly on its stack and
// passes it to read(); it never calls a MSG_TELEPORT constructor.  Retaining
// that uninitialized protocol storage is required because raw_0d is not read
// by the TELEPORT decoder.
template <>
int IPacketDispatcher<MSG_TELEPORT,
                      stTeleportParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    char message_storage[sizeof(MSG_TELEPORT)];
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(message_storage);
    int result = read(packet, message);
    if (result != 0) {
        return result;
    }

    stTeleportParam param;
    result = process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, param);
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// The exact template specialization has a single source/object owner here.
// IDA_EA: 0x8253E2E / 0x8253F7E / 0x8253F88 / 0x825CB26 /
//         0x825CB56 / 0x825D8C4 / 0x859880E
template class IPacketDispatcher<MSG_TELEPORT,
                                 stTeleportParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8596B98
// IDA_NAME: _ZN19Dispatcher_TeleportC2Ev
// DEMANGLED: Dispatcher_Teleport::Dispatcher_Teleport(void)
Dispatcher_Teleport::Dispatcher_Teleport()
    : IPacketDispatcher<MSG_TELEPORT,
                        stTeleportParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x82458F8
// IDA_NAME: _ZN19Dispatcher_TeleportD2Ev
// DEMANGLED: Dispatcher_Teleport::~Dispatcher_Teleport()
// IDA_EA: 0x8245928
// IDA_NAME: _ZN19Dispatcher_TeleportD0Ev
// DEMANGLED: Dispatcher_Teleport::~Dispatcher_Teleport()
Dispatcher_Teleport::~Dispatcher_Teleport() {}

// IDA_EA: 0x81D044A
// IDA_NAME: _ZN19Dispatcher_Teleport4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Teleport::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Teleport::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_TELEPORT& request = static_cast<MSG_TELEPORT&>(message);
    if (packet.get_short(request.stackable_slot) != 1) {
        return LineFunc(11060, kTeleportReadSignature, 0, 0);
    }
    if (packet.get_int(request.request_item_token) != 1) {
        return LineFunc(11064, kTeleportReadSignature, 0, 0);
    }
    if (packet.get_byte(request.item_space) != 1) {
        return LineFunc(11068, kTeleportReadSignature, 0, 0);
    }
    if (packet.get_byte(request.target_village) == 1) {
        return 0;
    }
    return LineFunc(11071, kTeleportReadSignature, 0, 0);
}

// IDA_EA: 0x81D056C
// IDA_NAME: _ZN19Dispatcher_Teleport11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Teleport::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Teleport::check_error(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    stTeleportParam& result = static_cast<stTeleportParam&>(param);
    const MSG_TELEPORT& request = static_cast<const MSG_TELEPORT&>(message);
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    if (user->get_state() > 4) {
        user->SendCmdErrorPacket(kTeleportCommand,
                                 19);
        return -1;
    }

    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(request.item_space));
    const Inven_Item* const item = user->getCurCharacInvenR()->GetInvenRef(
        inventory_type, static_cast<int>(request.stackable_slot));
    if (!item || item->item_id_ != kTeleportPotionItemId) {
        return -2;
    }
    if (request.target_village == 0) {
        return kSuppressReply;
    }
    if (user->CheckMoveTown(request.target_village)) {
        return kSuppressReply;
    }

    CParty* const party = user->GetParty();
    if (party) {
        const int teleport_state = party->GetPartyTelePort()->get_teleport_state();
        if ((static_cast<unsigned char>(teleport_state) & 0x80u) != 0u ||
            static_cast<signed char>(teleport_state) <= 3) {
            return 158;
        }
    }

    result.consumed_item_id = static_cast<uint32_t>(item->item_id_);
    if (user->use_stackable(static_cast<short>(request.stackable_slot),
                            static_cast<int>(request.item_space),
                            -1,
                            -1) != 1) {
        return 17;
    }
    if (expert_job::CAlchemist::OnTeleportCharacter(
            user, request.target_village, 0, 0, 0, true) != 1) {
        return kSuppressReply;
    }
    return 0;
}

// IDA_EA: 0x81D07BC
// IDA_NAME: _ZN19Dispatcher_Teleport7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Teleport::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Teleport::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    stTeleportParam& result = static_cast<stTeleportParam&>(param);
    result.result = check_error(user, message, param);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(11212,
                        kTeleportProcessSignature,
                        result.result,
                        user->get_acc_id());
    }

    const MSG_TELEPORT& request = static_cast<const MSG_TELEPORT&>(message);
    result.item_space = request.item_space;
    result.request_item_token = request.request_item_token;
    result.stackable_slot = request.stackable_slot;
    result.target_village = request.target_village;
    return 0;
}

// IDA_EA: 0x81D087C
// IDA_NAME: _ZN19Dispatcher_Teleport4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Teleport::send(CUser *,ParamBase &)
void Dispatcher_Teleport::send(CUser* user, ParamBase& param) {
    const stTeleportParam& result = static_cast<const stTeleportParam&>(param);
    if (result.result == kSuppressReply) {
        return;
    }
    if (result.result != 0) {
        user->SendCmdErrorPacket(kTeleportCommand,
                                 static_cast<unsigned char>(result.result));
        return;
    }

    PacketGuard inventory_packet;
    inventory_packet.put_header(0, 14);
    inventory_packet.put_byte(result.item_space);
    inventory_packet.put_short(1);
    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(result.item_space));
    user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(inventory_type), result.stackable_slot, inventory_packet);
    inventory_packet.finalize(true);
    user->Send(inventory_packet);

    PacketGuard response;
    response.put_header(1, kTeleportCommand);
    response.put_byte(1);
    response.put_short(result.stackable_slot);
    response.put_int(result.consumed_item_id);
    response.finalize(true);
    user->Send(response);
}
