#include "infra/security/policy/check_security_protection_dispatcher.h"
#include "infra/network/packet/PacketEnum.h"

#include "common/manager/global_data.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8265424
// IDA_NAME: _ZN23CheckSecurityProtectionC2Ev
// DEMANGLED: CheckSecurityProtection::CheckSecurityProtection(void)
CheckSecurityProtection::CheckSecurityProtection() : ParamBase(), result(0) {}

// IDA_EA: 0x82658E8
// IDA_NAME: _ZN23CheckSecurityProtectionD2Ev
// DEMANGLED: CheckSecurityProtection::~CheckSecurityProtection()
// IDA_EA: 0x8265918
// IDA_NAME: _ZN23CheckSecurityProtectionD0Ev
// DEMANGLED: CheckSecurityProtection::~CheckSecurityProtection()
CheckSecurityProtection::~CheckSecurityProtection() {}

#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x8266000
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23CheckSecurityProtectionL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,CheckSecurityProtection,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8266150
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23CheckSecurityProtectionL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,CheckSecurityProtection,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x826615A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23CheckSecurityProtectionL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,CheckSecurityProtection,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x826713C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23CheckSecurityProtectionL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,CheckSecurityProtection,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x8266E88
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23CheckSecurityProtectionL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,CheckSecurityProtection,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8266EB8
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23CheckSecurityProtectionL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,CheckSecurityProtection,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8598BAA
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE23CheckSecurityProtectionL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,CheckSecurityProtection,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_BASE,
                                 CheckSecurityProtection,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8597656
// IDA_NAME: _ZN34DisPatcher_CheckSecurityProtectionC2Ev
// DEMANGLED: DisPatcher_CheckSecurityProtection::DisPatcher_CheckSecurityProtection(void)
DisPatcher_CheckSecurityProtection::DisPatcher_CheckSecurityProtection() {}

// IDA_EA: 0x826562A
// IDA_NAME: _ZN34DisPatcher_CheckSecurityProtectionD2Ev
// DEMANGLED: DisPatcher_CheckSecurityProtection::~DisPatcher_CheckSecurityProtection()
// IDA_EA: 0x826565A
// IDA_NAME: _ZN34DisPatcher_CheckSecurityProtectionD0Ev
// DEMANGLED: DisPatcher_CheckSecurityProtection::~DisPatcher_CheckSecurityProtection()
DisPatcher_CheckSecurityProtection::~DisPatcher_CheckSecurityProtection() {}

// IDA_EA: 0x8264810
// IDA_NAME: _ZN34DisPatcher_CheckSecurityProtection4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_CheckSecurityProtection::read(PacketBuf &,MSG_BASE &)
int DisPatcher_CheckSecurityProtection::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x826481A
// IDA_NAME: _ZN34DisPatcher_CheckSecurityProtection7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_CheckSecurityProtection::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_CheckSecurityProtection::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)message;

    CheckSecurityProtection& output =
        static_cast<CheckSecurityProtection&>(param);
    output.result = static_cast<uint8_t>(
        GlobalData::s_pSecuProtectionField->Check(
            user, static_cast<SECURITY_PROTCTION>(56)));
    return 0;
}

// IDA_EA: 0x8264852
// IDA_NAME: _ZN34DisPatcher_CheckSecurityProtection4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_CheckSecurityProtection::send(CUser *,ParamBase &)
void DisPatcher_CheckSecurityProtection::send(CUser* user, ParamBase& param) {
    CheckSecurityProtection& output =
        static_cast<CheckSecurityProtection&>(param);
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, ENUM_CMDPACKET_CHECK_SECURITY_PROTECTION);
    packet.put_byte(0);
    packet.put_byte(output.result);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8264912
// IDA_NAME: _ZN34DisPatcher_CheckSecurityProtection11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_CheckSecurityProtection::check_error(CUser *,MSG_BASE &)
int DisPatcher_CheckSecurityProtection::check_error(CUser* user,
                                                    MSG_BASE& message) {
    (void)user;
    (void)message;
    return 0;
}
