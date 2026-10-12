#include "old_game/player/core/cuser.h"

#include "old_game/economy/mall/auction_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/arad_jpn_packet_header.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>

extern "C" {
int MD5_Init(void* context);
int MD5_Update(void* context, const void* data, unsigned long length);
int MD5_Final(unsigned char* output, void* context);
}

namespace {

const char kEmptyCharacName[] = "";

}  // namespace

// IDA_EA: 0x84FA298
// IDA_NAME: _ZN5CUser11getHashCodeEPh
// DEMANGLED: CUser::getHashCode(unsigned char *)
int CUser::getHashCode(unsigned char* output) {
    // IDA allocates 56 dwords for the legacy md5_starts/update/finish context.
    // Keep that caller-local extent rather than exposing a library-specific
    // MD5_CTX in CUser's ABI.
    uint32_t context[56];
    MD5_Init(context);
    MD5_Update(context,
               characters_.data(),
               static_cast<unsigned long>(sizeof(_Charac_info) *
                                          characters_.size()));
    return MD5_Final(output, context);
}

// IDA_EA: 0x80C8C96
// IDA_NAME: _ZNK5CUser6GetUIDEv
// DEMANGLED: CUser::GetUID(void)const
int CUser::GetUID() const {
    const unsigned short high = slot_index_;
    const unsigned short low = increment_id_;
    return static_cast<int>((static_cast<unsigned int>(high) << 16) | low);
}

// IDA_EA: 0x80DA36E
// IDA_NAME: _ZNK5CUser10get_acc_idEv
// DEMANGLED: CUser::get_acc_id(void)const
int CUser::get_acc_id() const {
    return account_id_;
}

// IDA_EA: 0x80DA37C
// IDA_NAME: _ZNK5CUser13get_unique_idEv
// DEMANGLED: CUser::get_unique_id(void)const
int CUser::get_unique_id() const {
    return unique_id_;
}

// IDA_EA: 0x84B9AA2
// IDA_NAME: _ZN5CUser11getSecuTypeEv
// DEMANGLED: CUser::getSecuType(void)
int CUser::getSecuType() {
    return hades_.security_state_.security_type;
}

// IDA_EA: 0x822FCE4
// IDA_NAME: _ZN5CUser13getAntibotKeyEv
// DEMANGLED: CUser::getAntibotKey(void)
int CUser::getAntibotKey() {
    return antibot_key_;
}

// IDA_EA: 0x84EC87A
// IDA_NAME: _ZN5CUser13setAntibotKeyEi
// DEMANGLED: CUser::setAntibotKey(int)
CUser* CUser::setAntibotKey(int key) {
    antibot_key_ = key;
    return this;
}

// IDA_EA: 0x8275676
// IDA_NAME: _ZN5CUser18GetPISenderManagerEv
// DEMANGLED: CUser::GetPISenderManager(void)
PISenderManager* CUser::GetPISenderManager() {
    return &pi_sender_;
}

// IDA_EA: 0x8120432
// IDA_NAME: _ZNK15CUserCharacInfo13getCurCharacREv
// DEMANGLED: CUserCharacInfo::getCurCharacR(void)const
const _Charac_info* CUserCharacInfo::getCurCharacR() const {
    return current_charac_;
}

// IDA_EA: 0x80CBC4E
// IDA_NAME: _ZNK15CUserCharacInfo14getCurCharacNoEv
// DEMANGLED: CUserCharacInfo::getCurCharacNo(void)const
int CUserCharacInfo::getCurCharacNo() const {
    const _Charac_info* cur_charac = getCurCharacR();
    if (cur_charac) {
        return cur_charac->fields_.character_no_;
    }
    return 0;
}

// IDA_EA: 0x8101028
// IDA_NAME: _ZNK15CUserCharacInfo16getCurCharacNameEv
// DEMANGLED: CUserCharacInfo::getCurCharacName(void)const
const char* CUserCharacInfo::getCurCharacName() const {
    const _Charac_info* cur_charac = getCurCharacR();
    if (cur_charac != 0) {
        return cur_charac->fields_.character_name_;
    }
    return kEmptyCharacName;
}

// IDA_EA: 0x80DA2B8
// IDA_NAME: _ZNK15CUserCharacInfo16get_charac_levelEv
// DEMANGLED: CUserCharacInfo::get_charac_level(void)const
int CUserCharacInfo::get_charac_level() const {
    const _Charac_info* cur_charac = getCurCharacR();
    if (cur_charac) {
        return cur_charac->fields_.level_;
    }
    return 0;
}

// IDA_EA: 0x867BEA0
// IDA_NAME: _ZN5CUser15SendCmdOkPacketE14ENUM_CMDPACKET
// DEMANGLED: CUser::SendCmdOkPacket(ENUM_CMDPACKET)
bool CUser::SendCmdOkPacket(ENUM_CMDPACKET cmd) {
    PacketGuard packet;
    packet.clear();
    packet.put_header(1, static_cast<int>(cmd));
    packet.put_byte(1);
    packet.finalize(true);
    return Send(packet);
}

// IDA_EA: 0x867BF42
// IDA_NAME: _ZN5CUser18SendCmdErrorPacketE14ENUM_CMDPACKETh
// DEMANGLED: CUser::SendCmdErrorPacket(ENUM_CMDPACKET,unsigned char)
bool CUser::SendCmdErrorPacket(ENUM_CMDPACKET cmd, unsigned char error) {
    PacketGuard packet;
    return SendCmdErrorPacket(cmd, error, packet);
}

// IDA_EA: 0x867BFAE
// IDA_NAME: _ZN5CUser18SendCmdErrorPacketE14ENUM_CMDPACKEThR11PacketGuard
// DEMANGLED: CUser::SendCmdErrorPacket(ENUM_CMDPACKET,unsigned char,PacketGuard &)
bool CUser::SendCmdErrorPacket(ENUM_CMDPACKET cmd,
                               unsigned char error,
                               PacketGuard& packet) {
    packet.put_header(1, static_cast<int>(cmd));
    packet.put_byte(0);
    packet.put_byte(static_cast<int>(error));
    packet.finalize(true);
    return Send(packet);
}

// IDA_EA: 0x867C022
// IDA_NAME: _ZN5CUser18SendCmdErrorPacketE14ENUM_CMDPACKETh8PAY_TYPE
// DEMANGLED: CUser::SendCmdErrorPacket(ENUM_CMDPACKET,unsigned char,PAY_TYPE)
bool CUser::SendCmdErrorPacket(ENUM_CMDPACKET cmd,
                               unsigned char error,
                               PAY_TYPE payment_type) {
    PacketGuard packet;
    packet.put_header(1, static_cast<int>(cmd));
    packet.put_byte(0);
    packet.put_byte(static_cast<int>(error));
    packet.put_byte(static_cast<int>(payment_type));
    packet.finalize(true);
    return Send(packet);
}

// IDA_EA: 0x8692824
// IDA_NAME: _ZN5CUser21SendCmdErrorPacketJpnE14ENUM_CMDPACKETt
// DEMANGLED: CUser::SendCmdErrorPacketJpn(ENUM_CMDPACKET,unsigned short)
int CUser::SendCmdErrorPacketJpn(ENUM_CMDPACKET cmd, unsigned short error) {
    PacketGuard packet;
    ARAD::DISPATCHER::make_cmd_packetheader_jpn(packet, cmd, false);
    packet.put_short(error);
    packet.finalize(true);
    return Send(packet);
}

// IDA_EA: 0x867C1D4
// IDA_NAME: _ZN5CUser13SendCmdPacketENS_11eSendTargetE14ENUM_CMDPACKETi
// DEMANGLED: CUser::SendCmdPacket(CUser::eSendTarget,ENUM_CMDPACKET,int)
int CUser::SendCmdPacket(eSendTarget target,
                         ENUM_CMDPACKET cmd,
                         int value) {
    PacketGuard packet;
    packet.put_header(1, static_cast<int>(cmd));
    if (cmd == ENUM_CMDPACKET_DEBUG_COMMAND) {
        packet.put_byte(1);
        packet.put_byte(value);
        packet.finalize(true);
        return SendPacket(target, packet);
    }

    LogManager::logFormat(
        1,
        "user.cpp",
        "bool CUser::SendCmdPacket(CUser::eSendTarget, ENUM_CMDPACKET, int)",
        0x7741,
        "User %s - CmdID %d invalid",
        getCurCharacName(),
        static_cast<int>(cmd));
    return 0;
}

// IDA_EA: 0x8680A96
// IDA_NAME: _ZN5CUser15AuctionPreCheckE14ENUM_CMDPACKET8PAY_TYPE
// DEMANGLED: CUser::AuctionPreCheck(ENUM_CMDPACKET,PAY_TYPE)
int CUser::AuctionPreCheck(ENUM_CMDPACKET cmd, PAY_TYPE payment_type) {
    const int payment_value = static_cast<int>(payment_type);
    const bool disconnected =
        (payment_value == 1 &&
         GlobalData::s_cera_auction_proxy->IsRunning() != 1) ||
        (payment_value == 0 &&
         GlobalData::s_auction_proxy->IsRunning() != 1);
    if (disconnected) {
        SendCmdErrorPacket(cmd, 0x8C, payment_type);
        return 0;
    }
    if (get_state() <= 2 || !getCurCharacR()) {
        return 33105;
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        this, static_cast<SECURITY_PROTCTION>(53));
    if (security_error) {
        SendCmdErrorPacket(cmd,
                           static_cast<unsigned char>(security_error),
                           payment_type);
        return 0;
    }
    return -1;
}
