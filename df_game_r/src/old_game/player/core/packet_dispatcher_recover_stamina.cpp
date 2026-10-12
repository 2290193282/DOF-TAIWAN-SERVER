#include "old_game/player/core/packet_dispatcher_recover_stamina.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kProcessSignature[] =
    "virtual int DisPatcher_RecoverStamina::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x81C3EA4
// IDA_NAME: _ZN25DisPatcher_RecoverStamina4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_RecoverStamina::read(PacketBuf &,MSG_BASE &)
int DisPatcher_RecoverStamina::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81C3EAE
// IDA_NAME: _ZN25DisPatcher_RecoverStamina7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_RecoverStamina::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_RecoverStamina::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& base_param) {
    stOnRecoverStaminaParam& param =
        static_cast<stOnRecoverStaminaParam&>(base_param);
    const int check_result = check_error(user, message);
    if (check_result > 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_RECOVER_STAMINA,
                                 static_cast<unsigned char>(check_result));
        return -1;
    }
    if (check_result < 0) {
        return LineFunc(3997, kProcessSignature, 0, user->get_acc_id());
    }

    param.result = user->serviceOfShusia();
    return check_result;
}

// IDA_EA: 0x81C3F4A
// IDA_NAME: _ZN25DisPatcher_RecoverStamina11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_RecoverStamina::check_error(CUser *,MSG_BASE &)
int DisPatcher_RecoverStamina::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (user->get_state() != static_cast<ch_state>(3)) {
        return 99;
    }

    const int security_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(47));
    if (security_result != 0) {
        return security_result;
    }
    return user->getCurCharacR() ? 0 : -1;
}

// IDA_EA: 0x81C3FB8
// IDA_NAME: _ZN25DisPatcher_RecoverStamina4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_RecoverStamina::send(CUser *,ParamBase &)
void DisPatcher_RecoverStamina::send(CUser* user, ParamBase& base_param) {
    stOnRecoverStaminaParam& param =
        static_cast<stOnRecoverStaminaParam&>(base_param);
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_RECOVER_STAMINA);
    if (param.result != 0) {
        response.put_byte(0);
        response.put_byte(static_cast<unsigned char>(param.result));
    } else {
        response.put_byte(1);
        response.put_int(user->getCurCharacInvenR()->get_money());
    }
    response.finalize(true);
    user->Send(response);
}

// IDA_EA: 0x823237A
// IDA_EA: 0x82323AA
// IDA_NAME: _ZN25DisPatcher_RecoverStaminaD2Ev
// IDA_NAME: _ZN25DisPatcher_RecoverStaminaD0Ev
// DEMANGLED: DisPatcher_RecoverStamina::~DisPatcher_RecoverStamina()
DisPatcher_RecoverStamina::~DisPatcher_RecoverStamina() {}

// IDA_EA: 0x85951C0
// IDA_NAME: _ZN25DisPatcher_RecoverStaminaC2Ev
// DEMANGLED: DisPatcher_RecoverStamina::DisPatcher_RecoverStamina(void)
DisPatcher_RecoverStamina::DisPatcher_RecoverStamina()
    : IPacketDispatcher<MSG_USERINFO_REQ,
                        stOnRecoverStaminaParam,
                        static_cast<ch_state>(3)>() {}

// The complete state-3 template family for this exact message/parameter pair
// is already provided by protocol/packet_dispatcher_generic_state2_3.cpp:
//   dispatch_template @0x82597AA, dispatch_sig @0x82598FA,
//   check_state @0x8259904, D2/D0 @0x82365C4/0x82365F4,
//   send @0x825ADC2, C2 @0x85980BA.
// Do not instantiate it here: a second provider would change the original
// vtable ownership and violate the single-provider ABI rule.
