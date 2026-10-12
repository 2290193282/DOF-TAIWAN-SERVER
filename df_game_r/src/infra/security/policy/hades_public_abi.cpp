#include "infra/security/policy/hades_public_abi.h"

#include "infra/security/policy/hades_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/user.h"
#include "core/config/env/environment.h"
#include "old_data/db/anti_cheat/db_hades_punish.h"
#include "old_data/Login/packets/hades_login_packet.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/transport/network_endpoint.h"

#include <map>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
struct Packet_Hades_Gold_Minus : public PacketHeader {
    Packet_Hades_Gold_Minus();

    int32_t acc_id;
    uint32_t money;
    int16_t reason;
};

struct Packet_Hades_Gold_Plus : public PacketHeader {
    Packet_Hades_Gold_Plus();

    int32_t acc_id;
    uint32_t money;
    int16_t reason;
};

struct Packet_Hades_GoldTracking : public PacketHeader {
    Packet_Hades_GoldTracking();

    int32_t owner_acc_id;
    uint32_t account_id;
    char ip[16];
    uint32_t money;
    uint16_t used_fatigue;
    uint8_t tracking_type;
};

struct Packet_Hades_StdUpdate : public PacketHeader {
    Packet_Hades_StdUpdate();

    int32_t acc_id;
    uint16_t category;
    uint16_t value;
};

struct Packet_Hades_UserStatus : public PacketHeader {
    Packet_Hades_UserStatus();

    int32_t acc_id;
    int32_t status;
};

// IDA_EA: 0x84B996A.  Packet_Hades_Logout is the ten-byte server header
// followed by the owning account id; its header advertises the exact 0x0e
// byte wire extent.
struct Packet_Hades_Logout : public PacketHeader {
    Packet_Hades_Logout();

    int32_t acc_id;
};
#pragma pack(pop)

static_assert(sizeof(Packet_Hades_Gold_Minus) == 0x14u,
              "Packet_Hades_Gold_Minus IDA ABI size must stay 0x14");
static_assert(sizeof(Packet_Hades_Gold_Plus) == 0x14u,
              "Packet_Hades_Gold_Plus IDA ABI size must stay 0x14");
static_assert(sizeof(Packet_Hades_GoldTracking) == 0x29u,
              "Packet_Hades_GoldTracking IDA ABI size must stay 0x29");
static_assert(sizeof(Packet_Hades_StdUpdate) == 0x12u,
              "Packet_Hades_StdUpdate IDA ABI size must stay 0x12");
static_assert(sizeof(Packet_Hades_Login) == 0x64u,
              "Packet_Hades_Login IDA ABI size must stay 0x64");
static_assert(sizeof(Packet_Hades_UserStatus) == 0x12u,
              "Packet_Hades_UserStatus IDA ABI size must stay 0x12");
static_assert(sizeof(Packet_Hades_Logout) == 0x0eu,
              "Packet_Hades_Logout IDA ABI size must stay 0x0e");

// IDA_EA: 0x84B91E8
// IDA_NAME: _ZN8XNuclear6CHades4InitEP5CUser
// DEMANGLED: XNuclear::CHades::Init(CUser *)
XNuclear::CHades* XNuclear::CHades::Init(CUser* user) {
    owner_ = user;
    security_state_.reward_enabled = 0u;
    security_state_.security_type = 0u;
    security_state_.reward_type = 0u;
    security_state_.security_grade = 0;
    return this;
}

// IDA_EA: 0x84B9386
// IDA_NAME: _ZN8XNuclear6CHades20Send_ReturnToVillageEv
// DEMANGLED: XNuclear::CHades::Send_ReturnToVillage(void)
void XNuclear::CHades::Send_ReturnToVillage() {
    // Old body is exactly prologue/epilogue: 55 89 E5 5D C3.
}

// IDA_EA: 0x84B93EE
// IDA_NAME: _ZN8XNuclear6CHades15Send_ChangeGoldEv
// DEMANGLED: XNuclear::CHades::Send_ChangeGold(void)
void XNuclear::CHades::Send_ChangeGold() {
    // Old body is exactly prologue/epilogue: 55 89 E5 5D C3.
}

// IDA_EA: 0x84B93FA
// IDA_NAME: _ZN8XNuclear6CHades8GoldPlusEjNS_24ENUM_GOLD_CONTROL_REASONE
// DEMANGLED: XNuclear::CHades::GoldPlus(unsigned int,XNuclear::ENUM_GOLD_CONTROL_REASON)
int XNuclear::CHades::GoldPlus(unsigned int money,
                               ENUM_GOLD_CONTROL_REASON reason) {
    needLogin();
    Packet_Hades_Gold_Plus packet;
    packet.acc_id = owner_->get_acc_id();
    packet.money = money;
    packet.reason = static_cast<int16_t>(reason);
    return GlobalData::s_hades_proxy->SendHadesTcpPacket(
        reinterpret_cast<char*>(&packet));
}

// IDA_EA: 0x84B944A
// IDA_NAME: _ZN8XNuclear6CHades9GoldMinusEjNS_24ENUM_GOLD_CONTROL_REASONE
// DEMANGLED: XNuclear::CHades::GoldMinus(unsigned int,XNuclear::ENUM_GOLD_CONTROL_REASON)
int XNuclear::CHades::GoldMinus(unsigned int money,
                                ENUM_GOLD_CONTROL_REASON reason) {
    needLogin();
    Packet_Hades_Gold_Minus packet;
    packet.acc_id = owner_->get_acc_id();
    packet.money = money;
    packet.reason = static_cast<int16_t>(reason);
    return GlobalData::s_hades_proxy->SendHadesTcpPacket(
        reinterpret_cast<char*>(&packet));
}

// IDA_EA: 0x84B949A
// IDA_NAME: _ZN8XNuclear6CHades12GoldTrackingEhjPcjj
// DEMANGLED: XNuclear::CHades::GoldTracking(unsigned char,unsigned int,char *,unsigned int,unsigned int)
int XNuclear::CHades::GoldTracking(unsigned char tracking_type,
                                   unsigned int account_id,
                                   char* ip,
                                   unsigned int money,
                                   unsigned int used_fatigue) {
    needLogin();
    Packet_Hades_GoldTracking packet;
    packet.owner_acc_id = owner_->get_acc_id();
    packet.account_id = account_id;
    memset(packet.ip, 0, sizeof(packet.ip));
    if (ip) {
        strncpy(packet.ip, ip, sizeof(packet.ip));
    }
    packet.money = money;
    packet.used_fatigue = static_cast<uint16_t>(used_fatigue);
    packet.tracking_type = tracking_type;
    return GlobalData::s_hades_proxy->SendHadesTcpPacket(
        reinterpret_cast<char*>(&packet));
}

// IDA_EA: 0x84B95A8
// IDA_NAME: _ZN8XNuclear6CHades14ChangeSecurityENS_19ENUM_HADES_SECURITYEj
// DEMANGLED: XNuclear::CHades::ChangeSecurity(XNuclear::ENUM_HADES_SECURITY,unsigned int)
int XNuclear::CHades::ChangeSecurity(ENUM_HADES_SECURITY, unsigned int) {
    return needLogin();
}

// IDA_EA: 0x84B95BC
// IDA_NAME: _ZN8XNuclear6CHades14RunHadesActionEjjjjPc
// DEMANGLED: XNuclear::CHades::RunHadesAction(unsigned int,unsigned int,unsigned int,unsigned int,char *)
unsigned int XNuclear::CHades::RunHadesAction(unsigned int action_flags,
                                              unsigned int punish_days,
                                              unsigned int apply_switch,
                                              unsigned int reserved,
                                              char* reason) {
    CUser* const user = owner_;
    if (!user) {
        return 0;
    }

    if (action_flags != 0x40u) {
        user->LogHistory("Hades,%d,%d,%d,%d",
                         action_flags,
                         punish_days,
                         apply_switch,
                         reserved);
    }

    if ((action_flags & 0x02u) != 0u) {
        user->setHackUserFlag();
        user->SetTradePunishType(4);
        DB_HadesPunish::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()),
            2u,
            punish_days,
            apply_switch,
            reserved,
            reason);
    }

    if ((action_flags & 0x01u) != 0u && !user->isHackUserTimer()) {
        user->setHackUserFlag();
        user->setHackUserTimer();
        DB_HadesPunish::makeRequest(
            static_cast<unsigned int>(user->get_acc_id()),
            1u,
            punish_days,
            apply_switch,
            reserved,
            reason);
    }

    if ((action_flags & 0x04u) != 0u) {
        user->DisConnSig(static_cast<DISCONN_SIG>(57), true, 0);
    }
    return action_flags & 0x04u;
}

// IDA_EA: 0x84B953E
// IDA_NAME: _ZN8XNuclear6CHades5ExpUpEj
// DEMANGLED: XNuclear::CHades::ExpUp(unsigned int)
// The old body preserves the unsigned-int ABI parameter but does not read it.
int XNuclear::CHades::ExpUp(unsigned int) {
    return needLogin();
}

// IDA_EA: 0x84B9718
// IDA_NAME: _ZN8XNuclear6CHades9needLoginEv
// DEMANGLED: XNuclear::CHades::needLogin(void)
int XNuclear::CHades::needLogin() {
    const int previous_conn_sec = login_conn_sec_;
    if (previous_conn_sec == GlobalData::s_hades_proxy->GetHadesSocketConnSec()) {
        return 0;
    }
    Send_Login();
    StdCateUpdate(3u, 0u);
    return 1;
}

// IDA_EA: 0x84B938C
// IDA_NAME: _ZN8XNuclear6CHades13StdCateUpdateEtt
// DEMANGLED: XNuclear::CHades::StdCateUpdate(unsigned short,unsigned short)
int XNuclear::CHades::StdCateUpdate(uint16_t category, uint16_t value) {
    needLogin();
    Packet_Hades_StdUpdate packet;
    packet.acc_id = owner_->get_acc_id();
    packet.category = category;
    packet.value = value;
    return GlobalData::s_hades_proxy->SendHadesTcpPacket(
        reinterpret_cast<char*>(&packet));
}

// IDA_EA: 0x84B9200
// IDA_NAME: _ZN8XNuclear6CHades10Send_LoginEv
// DEMANGLED: XNuclear::CHades::Send_Login(void)
int XNuclear::CHades::Send_Login() {
    CUser* user = owner_;
    login_conn_sec_ = GlobalData::s_hades_proxy->GetHadesSocketConnSec();

    Packet_Hades_Login packet;
    packet.acc_id = user->get_acc_id();
    packet.channel_no = static_cast<uint8_t>(
        G_CEnvironment()->get_channel_no());

    char ip[16] = {};
    GetIP(ip);
    strcpy(packet.ip, ip);
    strncpy(packet.mac_addr, user->network_.get_mac_addr(), 0x41u);
    packet.uid = user->GetUID();

    GlobalData::s_hades_proxy->SendHadesTcpPacket(
        reinterpret_cast<char*>(&packet));
    return Send_UserStatus();
}

// IDA_EA: 0x84B92EA
// IDA_NAME: _ZN8XNuclear6CHades11Send_LogoutEv
// DEMANGLED: XNuclear::CHades::Send_Logout(void)
int XNuclear::CHades::Send_Logout() {
    Packet_Hades_Logout packet;
    packet.acc_id = owner_->get_acc_id();
    return GlobalData::s_hades_proxy->SendHadesTcpPacket(
        reinterpret_cast<char*>(&packet));
}

// IDA_EA: 0x84B9774
// IDA_NAME: _ZN8XNuclear6CHades5GetIPEPc
// DEMANGLED: XNuclear::CHades::GetIP(char *)
char* XNuclear::CHades::GetIP(char* out) {
    CUser* user = owner_;
    user->network_.GetPeerIP2(out, 4);
    // This is the peer string populated by the login dispatch tail at
    // expanded CUser+0x8e768.  It is a login-session member, not a separate Hades
    // child, so Hades reads the same original storage through its owner.
    if (!out[0] && user->login_session_peer_ip_[0]) {
        memset(out, 0, 0x10u);
        return strncpy(out, user->login_session_peer_ip_, 0x10u);
    }
    return out;
}

// IDA_EA: 0x84B97FE
// IDA_NAME: _ZN8XNuclear6CHades15Send_UserStatusEv
// DEMANGLED: XNuclear::CHades::Send_UserStatus(void)
int XNuclear::CHades::Send_UserStatus() {
    CUser* user = owner_;
    Packet_Hades_UserStatus packet;
    packet.acc_id = user->get_acc_id();
    packet.status = 0;
    if ((user->getSecuType() & 1) != 0 ||
        (user->getSecuType() & 2) != 0 ||
        (user->getSecuType() & 0x20) != 0) {
        packet.status |= 1u;
    }
    if ((user->getSecuType() & 8) != 0) {
        packet.status |= 2u;
    }
    if ((user->getSecuType() & 0x10) != 0) {
        packet.status |= 4u;
    }
    return GlobalData::s_hades_proxy->SendHadesTcpPacket(
        reinterpret_cast<char*>(&packet));
}

// IDA_EA: 0x84B99D6
// IDA_NAME: _ZN23Packet_Hades_Gold_MinusC2Ev
// DEMANGLED: Packet_Hades_Gold_Minus::Packet_Hades_Gold_Minus(void)
Packet_Hades_Gold_Minus::Packet_Hades_Gold_Minus()
    : PacketHeader(0x0FDBu, 0x0014u) {}

// IDA_EA: 0x84B99B2
// IDA_NAME: _ZN22Packet_Hades_Gold_PlusC2Ev
// DEMANGLED: Packet_Hades_Gold_Plus::Packet_Hades_Gold_Plus(void)
Packet_Hades_Gold_Plus::Packet_Hades_Gold_Plus()
    : PacketHeader(0x0FDAu, 0x0014u) {}

// IDA_EA: 0x84B99FA
// IDA_NAME: _ZN25Packet_Hades_GoldTrackingC2Ev
// DEMANGLED: Packet_Hades_GoldTracking::Packet_Hades_GoldTracking(void)
Packet_Hades_GoldTracking::Packet_Hades_GoldTracking()
    : PacketHeader(0x0FDCu, 0x0029u) {}

// IDA_EA: 0x84B9A42
// IDA_NAME: _ZN22Packet_Hades_StdUpdateC2Ev
// DEMANGLED: Packet_Hades_StdUpdate::Packet_Hades_StdUpdate(void)
Packet_Hades_StdUpdate::Packet_Hades_StdUpdate()
    : PacketHeader(0x0FD9u, 0x0012u), acc_id(0), category(0),
      value(0) {}

// IDA_EA: 0x84B9A1E
// IDA_NAME: _ZN23Packet_Hades_UserStatusC2Ev
// DEMANGLED: Packet_Hades_UserStatus::Packet_Hades_UserStatus(void)
Packet_Hades_UserStatus::Packet_Hades_UserStatus()
    : PacketHeader(0x0FDDu, 0x0012u) {}

// IDA_EA: 0x84B996A
// IDA_NAME: _ZN19Packet_Hades_LogoutC2Ev
// DEMANGLED: Packet_Hades_Logout::Packet_Hades_Logout(void)
Packet_Hades_Logout::Packet_Hades_Logout()
    : PacketHeader(0x0FD3u, 0x000eu), acc_id(0) {}
