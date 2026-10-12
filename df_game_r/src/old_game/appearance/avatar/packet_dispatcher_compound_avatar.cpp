#include "old_game/appearance/avatar/packet_dispatcher_compound_avatar.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_game/appearance/avatar/compound_avatar.h"
#include "old_game/appearance/avatar/compound_avatar_messages.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_Compound_Avatar::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x81EAD40
// IDA_NAME: _ZN26Dispatcher_Compound_Avatar12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Compound_Avatar::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Compound_Avatar::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(18));
    if (protection_error != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_COMPOUND_AVATAR,
                                 static_cast<unsigned char>(protection_error));
        return 0;
    }

    // The old stack object begins with the MSG_BASE-sized (13-byte) prefix;
    // its direct reads populate exactly the payload fields at +0x0d..+0x22.
    // The compound provider only consumes those fields, matching the original
    // uninitialised header bytes in this direct-dispatch path.
    MSG_COMPOUND_AVATAR request;
    if (packet.get_short(request.recipe_slot) != 1) {
        return LineFunc(27785, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(request.first_avatar_slot) != 1) {
        return LineFunc(27786, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(request.first_avatar_item_id) != 1) {
        return LineFunc(27787, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(request.second_avatar_slot) != 1) {
        return LineFunc(27788, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(request.second_avatar_item_id) != 1) {
        return LineFunc(27789, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(request.requested_result_item) != 1) {
        return LineFunc(27790, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(request.requested_avatar_option) != 1) {
        return LineFunc(27791, kDispatchSignature, 0, 0);
    }
    if (packet.get_byte(request.upper_compound) != 1) {
        return LineFunc(27792, kDispatchSignature, 0, 0);
    }
    if (packet.get_byte(request.compound_kind) != 1) {
        return LineFunc(27794, kDispatchSignature, 0, 0);
    }

    if (user->CheckItemLock(2, static_cast<int>(request.first_avatar_slot))) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_COMPOUND_AVATAR, 213);
        return 0;
    }
    if (user->CheckItemLock(2, static_cast<int>(request.second_avatar_slot))) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_COMPOUND_AVATAR, 213);
        return 0;
    }

    GlobalData::s_pCompoundAvatar->ProcCompound(user, &request);
    return 0;
}

// IDA_EA: 0x82336FC
// IDA_NAME: _ZN26Dispatcher_Compound_Avatar4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Compound_Avatar::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Compound_Avatar::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233706
// IDA_NAME: _ZN26Dispatcher_Compound_Avatar7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Compound_Avatar::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Compound_Avatar::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x82438AC / 0x82438DC
// IDA_NAME: _ZN26Dispatcher_Compound_AvatarD2Ev / _ZN26Dispatcher_Compound_AvatarD0Ev
// DEMANGLED: Dispatcher_Compound_Avatar::~Dispatcher_Compound_Avatar()
Dispatcher_Compound_Avatar::~Dispatcher_Compound_Avatar() {}

// IDA_EA: 0x8595F04
// IDA_NAME: _ZN26Dispatcher_Compound_AvatarC2Ev
// DEMANGLED: Dispatcher_Compound_Avatar::Dispatcher_Compound_Avatar(void)
Dispatcher_Compound_Avatar::Dispatcher_Compound_Avatar()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
