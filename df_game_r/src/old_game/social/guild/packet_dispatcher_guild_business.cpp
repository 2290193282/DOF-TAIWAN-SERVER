#include "old_game/social/guild/packet_dispatcher_guild_business.h"

#include "common/config.h"
#include <algorithm>
#include <cstring>

#include "core/config/env/environment.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/global_tools.h"
#include "common/manager/mannerless_user_accusation_packet.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kReportMannerlessSignature[] =
    "virtual int Dispatcher_ReportMannerlessUser::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x85961DC
// IDA_NAME: _ZN31Dispatcher_ReportMannerlessUserC2Ev
// DEMANGLED: Dispatcher_ReportMannerlessUser::Dispatcher_ReportMannerlessUser(void)
Dispatcher_ReportMannerlessUser::Dispatcher_ReportMannerlessUser() {}

// IDA_EA: 0x82428D4
// IDA_NAME: _ZN31Dispatcher_ReportMannerlessUserD2Ev
// DEMANGLED: Dispatcher_ReportMannerlessUser::~Dispatcher_ReportMannerlessUser()
// IDA_EA: 0x8242904
// IDA_NAME: _ZN31Dispatcher_ReportMannerlessUserD0Ev
// DEMANGLED: Dispatcher_ReportMannerlessUser::~Dispatcher_ReportMannerlessUser()
Dispatcher_ReportMannerlessUser::~Dispatcher_ReportMannerlessUser() {}

// IDA_EA: 0x8206124
// IDA_NAME: _ZN31Dispatcher_ReportMannerlessUser12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ReportMannerlessUser::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ReportMannerlessUser::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(40836, kReportMannerlessSignature, 0, 0);
    }

    Packet_Mannerless_User_Accusation report;
    report.charac_no = user->getCurCharacNo();
    report.uid = user->GetUID();

    int accusation_type = 4;
    if (packet.get_int(accusation_type) != 1 || accusation_type > 3) {
        return LineFunc(40846, kReportMannerlessSignature, 0, 0);
    }
    report.accusation_type = accusation_type;
    report.reporter_acc_id = user->get_acc_id();

    const char* const reporter_name = user->getCurCharacName();
    const size_t reporter_name_length = std::strlen(reporter_name);
    const size_t reporter_copy_length =
        std::min(reporter_name_length, static_cast<size_t>(29));
    std::strncpy(report.reporter_name,
                 user->getCurCharacName(),
                 reporter_copy_length);

    uint8_t accused_server_group = 0;
    // The old body ignores get_byte's return value here.
    packet.get_byte(accused_server_group);
    report.accused_server_group = accused_server_group;
    if (accused_server_group > 100u) {
        return LineFunc(40864, kReportMannerlessSignature, 0, 0);
    }
    if (report.accused_server_group != 0) {
        report.reporter_server_group = user->GetServerGroup();
    } else {
        report.accused_server_group = G_CEnvironment()->get_server_group();
        report.reporter_server_group = report.accused_server_group;
    }

    unsigned int accused_name_length = 0;
    if (packet.get_int(accused_name_length) != 1) {
        return 0;
    }
    if (packet.get_str(report.accused_name,
                       sizeof(report.accused_name),
                       static_cast<int>(accused_name_length)) != 1) {
        return 0;
    }

    if (Packet_Mannerless_User_Accusation::isNeedTypeForChattingMsg(
            accusation_type)) {
        unsigned int chat_length = 0;
        if (packet.get_int(chat_length) != 1) {
            return 0;
        }
        if (packet.get_str(report.chat_message,
                           sizeof(report.chat_message),
                           static_cast<int>(chat_length)) != 1) {
            return 0;
        }
    }

    const Inven_Item head_equipment =
        user->getCurCharacInvenR()->GetInvenSlot(
            0, oldsrv::config::kEquipmentTypeTitle);
    if (head_equipment.getKey() == 26494) {
        report.result_or_status = 112;
    }

    if (report.reporter_server_group == report.accused_server_group &&
        accused_name_length == reporter_name_length &&
        std::strncmp(report.reporter_name,
                     report.accused_name,
                     accused_name_length) == 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REPORT_MANNERLESS_USER, 3);
        return 0;
    }

    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendTcpPacket(reinterpret_cast<char*>(&report), sizeof(report));
    return 0;
}

// IDA_EA: 0x82338B2
// IDA_NAME: _ZN31Dispatcher_ReportMannerlessUser4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ReportMannerlessUser::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ReportMannerlessUser::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82338BC
// IDA_NAME: _ZN31Dispatcher_ReportMannerlessUser7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ReportMannerlessUser::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ReportMannerlessUser::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

namespace {

const char kCallGuildAllMembersSignature[] =
    "virtual int Dispatcher_CallGuildAllMembers::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8596214
// IDA_NAME: _ZN30Dispatcher_CallGuildAllMembersC2Ev
// DEMANGLED: Dispatcher_CallGuildAllMembers::Dispatcher_CallGuildAllMembers(void)
Dispatcher_CallGuildAllMembers::Dispatcher_CallGuildAllMembers() {}

// IDA_EA: 0x82433CC
// IDA_NAME: _ZN30Dispatcher_CallGuildAllMembersD2Ev
// DEMANGLED: Dispatcher_CallGuildAllMembers::~Dispatcher_CallGuildAllMembers()
// IDA_EA: 0x82433FC
// IDA_NAME: _ZN30Dispatcher_CallGuildAllMembersD0Ev
// DEMANGLED: Dispatcher_CallGuildAllMembers::~Dispatcher_CallGuildAllMembers()
Dispatcher_CallGuildAllMembers::~Dispatcher_CallGuildAllMembers() {}

// IDA_EA: 0x81FFB1A
// IDA_NAME: _ZN30Dispatcher_CallGuildAllMembers12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CallGuildAllMembers::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CallGuildAllMembers::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    (void)packet;

    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(37837, kCallGuildAllMembersSignature, 0, 0);
    }

    const unsigned int charac_guildkey =
        static_cast<unsigned int>(user->get_charac_guildkey());
    if (!charac_guildkey) {
        return 0;
    }

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendPacketCallGuildAllMembers(
        static_cast<unsigned int>(user->getCurCharacNo()),
        charac_guildkey);
    return 0;
}

// IDA_EA: 0x823356C
// IDA_NAME: _ZN30Dispatcher_CallGuildAllMembers4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CallGuildAllMembers::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CallGuildAllMembers::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233576
// IDA_NAME: _ZN30Dispatcher_CallGuildAllMembers7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CallGuildAllMembers::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CallGuildAllMembers::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

namespace {

const char kCallGuildInviteSignature[] =
    "virtual int Dispatcher_CallGuildInvite::dispatch_sig(CUser*, PacketBuf&)";
const char kReplyGuildInviteSignature[] =
    "virtual int Dispatcher_ReplyGuildInvite::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x859632C
// IDA_NAME: _ZN26Dispatcher_CallGuildInviteC2Ev
// DEMANGLED: Dispatcher_CallGuildInvite::Dispatcher_CallGuildInvite(void)
Dispatcher_CallGuildInvite::Dispatcher_CallGuildInvite() {}

// IDA_EA: 0x82425C8
// IDA_NAME: _ZN26Dispatcher_CallGuildInviteD2Ev
// DEMANGLED: Dispatcher_CallGuildInvite::~Dispatcher_CallGuildInvite()
// IDA_EA: 0x82425F8
// IDA_NAME: _ZN26Dispatcher_CallGuildInviteD0Ev
// DEMANGLED: Dispatcher_CallGuildInvite::~Dispatcher_CallGuildInvite()
Dispatcher_CallGuildInvite::~Dispatcher_CallGuildInvite() {}

// IDA_EA: 0x8207852
// IDA_NAME: _ZN26Dispatcher_CallGuildInvite24send_packet_guild_inviteEP5CUserh
// DEMANGLED: Dispatcher_CallGuildInvite::send_packet_guild_invite(CUser *,unsigned char)
void Dispatcher_CallGuildInvite::send_packet_guild_invite(CUser* user,
                                                          unsigned char error) {
    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_CALL_GUILD_INVITE);
    if (error) {
        response.put_byte(0);
        response.put_byte(error);
    } else {
        response.put_byte(1);
    }
    response.finalize(true);
    user->Send(response);
}

// IDA_EA: 0x820791C
// IDA_NAME: _ZN26Dispatcher_CallGuildInvite12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CallGuildInvite::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CallGuildInvite::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(41551, kCallGuildInviteSignature, 0, 0);
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(11));
    if (protection_error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CALL_GUILD_INVITE,
            static_cast<unsigned char>(protection_error));
        return 0;
    }

    int target_name_length;
    char target_name[30];
    if (packet.get_int(target_name_length) != 1) {
        return LineFunc(41565, kCallGuildInviteSignature, 0, 0);
    }
    if (packet.get_str(target_name,
                       sizeof(target_name),
                       target_name_length) != 1) {
        return LineFunc(41569, kCallGuildInviteSignature, 0, 0);
    }

    const unsigned int charac_guildkey =
        static_cast<unsigned int>(user->get_charac_guildkey());
    if (!charac_guildkey) {
        send_packet_guild_invite(user, 0x22u);
        return 0;
    }
    if (user->IsSubGuildMaster() != 1 && user->IsGuildMaster() != 1 &&
        user->getGuildMemberGrade() != 3) {
        send_packet_guild_invite(user, 0x24u);
        return 0;
    }

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendCallGuildInvite(
        static_cast<unsigned int>(user->getCurCharacNo()),
        charac_guildkey,
        target_name);
    return 0;
}

// IDA_EA: 0x823397A
// IDA_NAME: _ZN26Dispatcher_CallGuildInvite4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CallGuildInvite::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CallGuildInvite::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233984
// IDA_NAME: _ZN26Dispatcher_CallGuildInvite7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CallGuildInvite::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CallGuildInvite::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596348
// IDA_NAME: _ZN27Dispatcher_ReplyGuildInviteC2Ev
// DEMANGLED: Dispatcher_ReplyGuildInvite::Dispatcher_ReplyGuildInvite(void)
Dispatcher_ReplyGuildInvite::Dispatcher_ReplyGuildInvite() {}

// IDA_EA: 0x824257A
// IDA_NAME: _ZN27Dispatcher_ReplyGuildInviteD2Ev
// DEMANGLED: Dispatcher_ReplyGuildInvite::~Dispatcher_ReplyGuildInvite()
// IDA_EA: 0x82425AA
// IDA_NAME: _ZN27Dispatcher_ReplyGuildInviteD0Ev
// DEMANGLED: Dispatcher_ReplyGuildInvite::~Dispatcher_ReplyGuildInvite()
Dispatcher_ReplyGuildInvite::~Dispatcher_ReplyGuildInvite() {}

// IDA_EA: 0x8207B52
// IDA_NAME: _ZN27Dispatcher_ReplyGuildInvite12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ReplyGuildInvite::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ReplyGuildInvite::dispatch_sig(CUser* user,
                                              PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(41592, kReplyGuildInviteSignature, 0, 0);
    }

    unsigned char reply_code;
    if (packet.get_byte(reply_code) != 1) {
        return LineFunc(41596, kReplyGuildInviteSignature, 0, 0);
    }

    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        PacketGuard response;
        response.put_header(1, ENUM_CMDPACKET_REPLY_GUILD_INVITE);
        response.put_byte(0);
        response.put_int(99);
        response.finalize(true);
        user->Send(response);
        return 0;
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(12));
    if (protection_error) {
        reply_code = static_cast<unsigned char>(
            GlobalData::s_pSecuProtectionField->GetOppositeErr(
                protection_error));
    }

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendReplyGuildInvite(
        static_cast<unsigned int>(user->getCurCharacNo()), reply_code);
    return 0;
}

// IDA_EA: 0x823398E
// IDA_NAME: _ZN27Dispatcher_ReplyGuildInvite4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ReplyGuildInvite::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ReplyGuildInvite::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233998
// IDA_NAME: _ZN27Dispatcher_ReplyGuildInvite7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ReplyGuildInvite::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ReplyGuildInvite::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

namespace {

const char kNotifyMessageToGuildSignature[] =
    "virtual int Dispatcher_NotifyMessageToGuild::dispatch_sig(CUser*, PacketBuf&)";
const char kGuildMasterDelegateSignature[] =
    "virtual int Dispatcher_GuildMasterDelegate::dispatch_sig(CUser*, PacketBuf&)";
const char kWriteGuildMemberMemoSignature[] =
    "virtual int Dispatcher_WriteGuildMemberMemo::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8596380
// IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuildC2Ev
// DEMANGLED: Dispatcher_NotifyMessageToGuild::Dispatcher_NotifyMessageToGuild(void)
Dispatcher_NotifyMessageToGuild::Dispatcher_NotifyMessageToGuild() {}

// IDA_EA: 0x82424DE
// IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuildD2Ev
// DEMANGLED: Dispatcher_NotifyMessageToGuild::~Dispatcher_NotifyMessageToGuild()
// IDA_EA: 0x824250E
// IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuildD0Ev
// DEMANGLED: Dispatcher_NotifyMessageToGuild::~Dispatcher_NotifyMessageToGuild()
Dispatcher_NotifyMessageToGuild::~Dispatcher_NotifyMessageToGuild() {}

// IDA_EA: 0x8207FFE
// IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuild12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_NotifyMessageToGuild::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_NotifyMessageToGuild::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(41716, kNotifyMessageToGuildSignature, 0, 0);
    }

    const unsigned int charac_guildkey =
        static_cast<unsigned int>(user->get_charac_guildkey());
    if (!charac_guildkey) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_NOTIFY_MESSAGE_TO_GUILD,
                                 34);
        return 0;
    }
    if (user->IsSubGuildMaster() != 1 && user->IsGuildMaster() != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_NOTIFY_MESSAGE_TO_GUILD,
                                 36);
        return 0;
    }

    int message_length = 0;
    char message[101];
    if (packet.get_int(message_length) != 1) {
        return LineFunc(41737, kNotifyMessageToGuildSignature, 0, 0);
    }
    if (packet.get_str(message, sizeof(message), message_length) != 1) {
        return LineFunc(41740, kNotifyMessageToGuildSignature, 0, 0);
    }
    if (G_CDataManager()->hasPreventString(message) ||
        G_CDataManager()->hasSpecialCharacterExceptSpace(message)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_NOTIFY_MESSAGE_TO_GUILD,
                                 108);
        return 0;
    }

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendMessageToGuild(charac_guildkey, message_length, message);

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_NOTIFY_MESSAGE_TO_GUILD);
    response.put_byte(1);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x82339B6
// IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuild4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_NotifyMessageToGuild::read(PacketBuf &,MSG_BASE &)
int Dispatcher_NotifyMessageToGuild::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82339C0
// IDA_NAME: _ZN31Dispatcher_NotifyMessageToGuild7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_NotifyMessageToGuild::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_NotifyMessageToGuild::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x859639C
// IDA_NAME: _ZN30Dispatcher_GuildMasterDelegateC2Ev
// DEMANGLED: Dispatcher_GuildMasterDelegate::Dispatcher_GuildMasterDelegate(void)
Dispatcher_GuildMasterDelegate::Dispatcher_GuildMasterDelegate() {}

// IDA_EA: 0x8242490
// IDA_NAME: _ZN30Dispatcher_GuildMasterDelegateD2Ev
// DEMANGLED: Dispatcher_GuildMasterDelegate::~Dispatcher_GuildMasterDelegate()
// IDA_EA: 0x82424C0
// IDA_NAME: _ZN30Dispatcher_GuildMasterDelegateD0Ev
// DEMANGLED: Dispatcher_GuildMasterDelegate::~Dispatcher_GuildMasterDelegate()
Dispatcher_GuildMasterDelegate::~Dispatcher_GuildMasterDelegate() {}

// IDA_EA: 0x82082D6
// IDA_NAME: _ZN30Dispatcher_GuildMasterDelegate12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_GuildMasterDelegate::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_GuildMasterDelegate::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(41783, kGuildMasterDelegateSignature, 0, 0);
    }

    const unsigned int charac_guildkey =
        static_cast<unsigned int>(user->get_charac_guildkey());
    if (!charac_guildkey) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_MASTER_DELEGATE, 34);
        return 0;
    }
    if (user->IsGuildMaster() != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_MASTER_DELEGATE, 36);
        return 0;
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(14));
    if (protection_error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_GUILD_MASTER_DELEGATE,
            static_cast<unsigned char>(protection_error));
        return 0;
    }

    int target_name_length = 0;
    char target_name[30];
    if (packet.get_int(target_name_length) != 1) {
        return LineFunc(41819, kGuildMasterDelegateSignature, 0, 0);
    }
    if (packet.get_str(target_name,
                       sizeof(target_name),
                       target_name_length) != 1) {
        return LineFunc(41822, kGuildMasterDelegateSignature, 0, 0);
    }

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendGuildMasterDelegate(
        charac_guildkey,
        static_cast<unsigned int>(user->get_charac_no(-1)),
        target_name_length,
        target_name);
    return 0;
}

// IDA_EA: 0x82339CA
// IDA_NAME: _ZN30Dispatcher_GuildMasterDelegate4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GuildMasterDelegate::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GuildMasterDelegate::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82339D4
// IDA_NAME: _ZN30Dispatcher_GuildMasterDelegate7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildMasterDelegate::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildMasterDelegate::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596AF0
// IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemoC2Ev
// DEMANGLED: Dispatcher_WriteGuildMemberMemo::Dispatcher_WriteGuildMemberMemo(void)
Dispatcher_WriteGuildMemberMemo::Dispatcher_WriteGuildMemberMemo() {}

// IDA_EA: 0x8241248
// IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemoD2Ev
// DEMANGLED: Dispatcher_WriteGuildMemberMemo::~Dispatcher_WriteGuildMemberMemo()
// IDA_EA: 0x8241278
// IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemoD0Ev
// DEMANGLED: Dispatcher_WriteGuildMemberMemo::~Dispatcher_WriteGuildMemberMemo()
Dispatcher_WriteGuildMemberMemo::~Dispatcher_WriteGuildMemberMemo() {}

// IDA_EA: 0x821E748
// IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_WriteGuildMemberMemo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_WriteGuildMemberMemo::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(54620, kWriteGuildMemberMemoSignature, 0, 0);
    }

    const unsigned int charac_guildkey =
        static_cast<unsigned int>(user->get_charac_guildkey());
    if (!charac_guildkey) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_WRITE_GUILD_MEMBER_MEMO, 34);
        return 0;
    }

    char memo[21] = {};
    int memo_length = 0;
    if (packet.get_int(memo_length) != 1) {
        return LineFunc(54634, kWriteGuildMemberMemoSignature, 0, 0);
    }
    if (packet.get_str(memo, sizeof(memo), memo_length) != 1) {
        return LineFunc(54637, kWriteGuildMemberMemoSignature, 0, 0);
    }
    if (G_CDataManager()->hasPreventString(memo) ||
        G_CDataManager()->hasSpecialCharacterExceptSpace(memo)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_WRITE_GUILD_MEMBER_MEMO,
                                 108);
        return 0;
    }

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendMemoToGuild(charac_guildkey,
                           user->get_charac_no(-1),
                           memo_length,
                           memo);

    PacketGuard response;
    response.put_header(1, ENUM_CMDPACKET_WRITE_GUILD_MEMBER_MEMO);
    response.put_byte(1);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233EA2
// IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_WriteGuildMemberMemo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_WriteGuildMemberMemo::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233EAC
// IDA_NAME: _ZN31Dispatcher_WriteGuildMemberMemo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_WriteGuildMemberMemo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_WriteGuildMemberMemo::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

namespace {

const char kSetSubGuildMasterSignature[] =
    "virtual int Dispatcher_SetSubGuildMaster::dispatch_sig(CUser*, PacketBuf&)";
const char kChangeGuildNameSignature[] =
    "virtual int Dispatcher_ChangeGuildName::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8596134
// IDA_NAME: _ZN28Dispatcher_SetSubGuildMasterC2Ev
// DEMANGLED: Dispatcher_SetSubGuildMaster::Dispatcher_SetSubGuildMaster(void)
Dispatcher_SetSubGuildMaster::Dispatcher_SetSubGuildMaster() {}

// IDA_EA: 0x8242A0C
// IDA_NAME: _ZN28Dispatcher_SetSubGuildMasterD2Ev
// DEMANGLED: Dispatcher_SetSubGuildMaster::~Dispatcher_SetSubGuildMaster()
// IDA_EA: 0x8242A3C
// IDA_NAME: _ZN28Dispatcher_SetSubGuildMasterD0Ev
// DEMANGLED: Dispatcher_SetSubGuildMaster::~Dispatcher_SetSubGuildMaster()
Dispatcher_SetSubGuildMaster::~Dispatcher_SetSubGuildMaster() {}

// IDA_EA: 0x82053E6
// IDA_NAME: _ZN28Dispatcher_SetSubGuildMaster12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SetSubGuildMaster::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SetSubGuildMaster::dispatch_sig(CUser* user,
                                               PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(40343, kSetSubGuildMasterSignature, 0, 0);
    }

    Packet_Monitor_Set_Sub_Guild_Master request;
    request.charac_no =
        static_cast<unsigned int>(user->get_charac_no(-1));
    request.charac_guildkey =
        static_cast<unsigned int>(user->get_charac_guildkey());

    int target_name_length = 0;
    if (packet.get_int(target_name_length) != 1) {
        return LineFunc(40353, kSetSubGuildMasterSignature, 0, 0);
    }
    if (packet.get_str(request.target_name,
                       sizeof(request.target_name),
                       target_name_length) != 1) {
        return LineFunc(40356, kSetSubGuildMasterSignature, 0, 0);
    }
    if (packet.get_byte(request.request_flag) != 1) {
        return LineFunc(40359, kSetSubGuildMasterSignature, 0, 0);
    }

    if (request.request_flag != 1) {
        CGuildServerProxy* const proxy =
            GlobalData::s_guild_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        proxy->SendPacket(reinterpret_cast<char*>(&request), 0x31);
    }
    return 0;
}

// IDA_EA: 0x8233862
// IDA_NAME: _ZN28Dispatcher_SetSubGuildMaster4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SetSubGuildMaster::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SetSubGuildMaster::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823386C
// IDA_NAME: _ZN28Dispatcher_SetSubGuildMaster7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SetSubGuildMaster::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SetSubGuildMaster::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596690
// IDA_NAME: _ZN26Dispatcher_ChangeGuildNameC2Ev
// DEMANGLED: Dispatcher_ChangeGuildName::Dispatcher_ChangeGuildName(void)
Dispatcher_ChangeGuildName::Dispatcher_ChangeGuildName() {}

// IDA_EA: 0x8241DDC
// IDA_NAME: _ZN26Dispatcher_ChangeGuildNameD2Ev
// DEMANGLED: Dispatcher_ChangeGuildName::~Dispatcher_ChangeGuildName()
// IDA_EA: 0x8241E0C
// IDA_NAME: _ZN26Dispatcher_ChangeGuildNameD0Ev
// DEMANGLED: Dispatcher_ChangeGuildName::~Dispatcher_ChangeGuildName()
Dispatcher_ChangeGuildName::~Dispatcher_ChangeGuildName() {}

// IDA_EA: 0x82134A8
// IDA_NAME: _ZN26Dispatcher_ChangeGuildName12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ChangeGuildName::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ChangeGuildName::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(49503, kChangeGuildNameSignature, 0, 0);
    }

    uint16_t slot = 0;
    if (packet.get_short(slot) != 1) {
        return LineFunc(49507, kChangeGuildNameSignature, 0, 0);
    }
    uint8_t item_space = 0;
    if (packet.get_byte(item_space) != 1) {
        return LineFunc(49510, kChangeGuildNameSignature, 0, 0);
    }

    const unsigned int charac_guildkey =
        static_cast<unsigned int>(user->get_charac_guildkey());
    if (!charac_guildkey) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_GUILD_NAME, 100);
        return 0;
    }
    if (user->isSetGuildCreateFlag(2) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_GUILD_NAME, 109);
        return 0;
    }
    if (user->IsGuildMaster() != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_GUILD_NAME, 86);
        return 0;
    }

    const int protection_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(15));
    if (protection_error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CHANGE_GUILD_NAME,
            static_cast<unsigned char>(protection_error));
        return 0;
    }

    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(item_space));
    if (user->CheckItemLock(inventory_type, static_cast<int>(slot))) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_GUILD_NAME, 213);
        return 0;
    }

    const Inven_Item guild_name_card = user->getCurCharacInvenR()->GetInvenSlot(
        static_cast<INVEN_TYPE>(1), static_cast<int>(slot));
    if (guild_name_card.getKey() != 745 ||
        user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1),
            static_cast<int>(slot),
            1,
            static_cast<eItemDelReason>(3),
            true) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_GUILD_NAME, 17);
        return 0;
    }

    PacketGuard response;
    response.put_header(0, 14);
    response.put_byte(0);
    response.put_short(1);
    user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(1), static_cast<int>(slot), response);
    response.finalize(true);
    user->Send(response);

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendGuildNameChange(
        static_cast<unsigned int>(user->getCurCharacNo()),
        charac_guildkey,
        user->getCreateGuildName());
    user->resetGuildCreateFlag();
    user->resetTempGuildNameAddr();
    return 0;
}

// IDA_EA: 0x8233B1E
// IDA_NAME: _ZN26Dispatcher_ChangeGuildName4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeGuildName::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ChangeGuildName::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233B28
// IDA_NAME: _ZN26Dispatcher_ChangeGuildName7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ChangeGuildName::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ChangeGuildName::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
