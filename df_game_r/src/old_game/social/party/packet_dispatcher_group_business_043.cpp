#include "old_game/social/party/packet_dispatcher_group_business_043.h"

#include <cstring>
#include <string>

#include "common/manager/community_server_proxy_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/gm_accounts_abi.h"
#include "old_game/social/guild/guild_power_parameter_script_abi.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/economy/inventory/item.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/security/policy/restrict_inputting_name.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/slang_filter.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/guild_parameter_script.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/channel/packet_dispatch/channel_packets.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/social/party/party.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kCallGuildMembersSignature[] =
    "virtual int Dispatcher_CallGuildMembers::dispatch_sig(CUser*, PacketBuf&)";
const char kCallGuildInfoSignature[] =
    "virtual int Dispatcher_CallGuildInfo::dispatch_sig(CUser*, PacketBuf&)";
const char kSelectCardSignature[] =
    "virtual int Dispatcher_Select_Card::dispatch_sig(CUser*, PacketBuf&)";

// The monitor payload layouts are derived from the four original C2 bodies
// plus their dispatch callers.  They are local transport records, not a new
// proxy or a substitute external packet provider.
#pragma pack(push, 1)
struct MonitorRequestMemberEnterPacket {
    MonitorRequestMemberEnterPacket()
        : header(0x04B7u, 0x0030u), uid(0), charac_no(0) {
        std::memset(target_name, 0, sizeof(target_name));
    }

    PacketHeader header;
    int uid;
    int charac_no;
    char target_name[30];
};

struct MonitorMemberEnterReplyPacket {
    MonitorMemberEnterReplyPacket()
        : header(0x04B9u, 0x0013u), uid(0), charac_no(0), accepted(0) {}

    PacketHeader header;
    int uid;
    int charac_no;
    uint8_t accepted;
};

struct MonitorMemberSecedePacket {
    MonitorMemberSecedePacket()
        : header(0x04BBu, 0x0031u), uid(0), charac_no(0) {
        // The old C2 clears only +0x13..+0x30.  raw_18 is intentionally left
        // untouched, exactly like Packet_Monitor_Member_Secede::C2.
        std::memset(target_name, 0, sizeof(target_name));
    }

    PacketHeader header;
    int uid;
    int charac_no;
    uint8_t raw_18;
    char target_name[30];
};

struct MonitorCallMemberListPacket {
    MonitorCallMemberListPacket()
        : header(0x04BEu, 0x0012u), uid(0), charac_no(0) {}

    PacketHeader header;
    int uid;
    int charac_no;
};
#pragma pack(pop)

static_assert(sizeof(MonitorRequestMemberEnterPacket) == 0x30u,
              "monitor request-member-enter wire size must stay 0x30");
static_assert(sizeof(MonitorMemberEnterReplyPacket) == 0x13u,
              "monitor member-enter-reply wire size must stay 0x13");
static_assert(sizeof(MonitorMemberSecedePacket) == 0x31u,
              "monitor member-secede wire size must stay 0x31");
static_assert(sizeof(MonitorCallMemberListPacket) == 0x12u,
              "monitor call-member-list wire size must stay 0x12");

CMonitorServerProxy* monitor_proxy_for(CUser* user) {
    return GlobalData::s_monitor_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
}

GuildParameterScript& guild_parameter_script(CDataManager* data_manager) {
    // IDA_EA: 0x8355B30 / 0x8358332
    // CDataManager owns the complete typed GuildParameterScript at +0xA50C.
    // The offset and 0x11c footprint are asserted by the public PVF ABI.
    return *data_manager->GetGuildParameterScriptForBuild();
}

void BuildOtherChannelChat(Packet_Monitor_Other_Channel_Chat& packet,
                           unsigned char chat_type,
                           uint32_t account_id,
                           unsigned char sender_marker,
                           uint32_t charac_no,
                           uint32_t item_position,
                           const char* target_name,
                           int target_name_length,
                           const char* message,
                           int message_length) {
    packet.chat_type = chat_type;
    packet.account_id = account_id;
    packet.sender_marker = sender_marker;
    packet.charac_no = charac_no;
    packet.item_position = item_position;
    packet.target_name_length = static_cast<unsigned char>(target_name_length);
    std::memcpy(packet.target_name, target_name, target_name_length);
    packet.message_length = static_cast<unsigned char>(message_length);
    std::memcpy(packet.message, message, message_length);
}

int OtherChannelChatWireSize(int message_length) {
    return static_cast<int>(static_cast<unsigned char>(message_length)) +
           static_cast<int>(offsetof(Packet_Monitor_Other_Channel_Chat,
                                     message));
}

bool IsChatUseItemId(int item_id) {
    return item_id == 36 || item_id == 898 || item_id == 2675024 ||
           item_id == 690000093 || item_id == 690000184;
}

}  // namespace


// IDA_EA: 0x8230FB0
// IDA_NAME: _ZN25stRequestMemberEnterParamC2Ev
// DEMANGLED: stRequestMemberEnterParam::stRequestMemberEnterParam(void)
stRequestMemberEnterParam::stRequestMemberEnterParam()
    : ParamBase(), result(0) {}

// IDA_EA: 0x8247BD0
// IDA_NAME: _ZN25stRequestMemberEnterParamD2Ev
// DEMANGLED: stRequestMemberEnterParam::~stRequestMemberEnterParam()
// IDA_EA: 0x8247C00
// IDA_NAME: _ZN25stRequestMemberEnterParamD0Ev
// DEMANGLED: stRequestMemberEnterParam::~stRequestMemberEnterParam()
stRequestMemberEnterParam::~stRequestMemberEnterParam() {}

// IDA_EA: 0x8230FD6
// IDA_NAME: _ZN23stMemberEnterReplyParamC2Ev
// DEMANGLED: stMemberEnterReplyParam::stMemberEnterReplyParam(void)
stMemberEnterReplyParam::stMemberEnterReplyParam()
    : ParamBase(), result(0) {}

// IDA_EA: 0x8247B82
// IDA_NAME: _ZN23stMemberEnterReplyParamD2Ev
// DEMANGLED: stMemberEnterReplyParam::~stMemberEnterReplyParam()
// IDA_EA: 0x8247BB2
// IDA_NAME: _ZN23stMemberEnterReplyParamD0Ev
// DEMANGLED: stMemberEnterReplyParam::~stMemberEnterReplyParam()
stMemberEnterReplyParam::~stMemberEnterReplyParam() {}

// IDA_EA: 0x8230FFC
// IDA_NAME: _ZN19stMemberSecedeParamC2Ev
// DEMANGLED: stMemberSecedeParam::stMemberSecedeParam(void)
stMemberSecedeParam::stMemberSecedeParam()
    : ParamBase(), result(0) {}

// IDA_EA: 0x8247B34
// IDA_NAME: _ZN19stMemberSecedeParamD2Ev
// DEMANGLED: stMemberSecedeParam::~stMemberSecedeParam()
// IDA_EA: 0x8247B64
// IDA_NAME: _ZN19stMemberSecedeParamD0Ev
// DEMANGLED: stMemberSecedeParam::~stMemberSecedeParam()
stMemberSecedeParam::~stMemberSecedeParam() {}

// IDA_EA: 0x8231022
// IDA_NAME: _ZN21stCallMemberListParamC2Ev
// DEMANGLED: stCallMemberListParam::stCallMemberListParam(void)
stCallMemberListParam::stCallMemberListParam()
    : ParamBase(), result(0) {}

// IDA_EA: 0x8247AE6
// IDA_NAME: _ZN21stCallMemberListParamD2Ev
// DEMANGLED: stCallMemberListParam::~stCallMemberListParam()
// IDA_EA: 0x8247B16
// IDA_NAME: _ZN21stCallMemberListParamD0Ev
// DEMANGLED: stCallMemberListParam::~stCallMemberListParam()
stCallMemberListParam::~stCallMemberListParam() {}


// IDA_EA: 0x8595BBC
// IDA_NAME: _ZN29Dispatcher_Score_Scroll_StateC2Ev
// DEMANGLED: Dispatcher_Score_Scroll_State::Dispatcher_Score_Scroll_State(void)
Dispatcher_Score_Scroll_State::Dispatcher_Score_Scroll_State() {}

// IDA_EA: 0x8243504
// IDA_NAME: _ZN29Dispatcher_Score_Scroll_StateD2Ev
// DEMANGLED: Dispatcher_Score_Scroll_State::~Dispatcher_Score_Scroll_State()
// IDA_EA: 0x8243534
// IDA_NAME: _ZN29Dispatcher_Score_Scroll_StateD0Ev
// DEMANGLED: Dispatcher_Score_Scroll_State::~Dispatcher_Score_Scroll_State()
Dispatcher_Score_Scroll_State::~Dispatcher_Score_Scroll_State() {}

// IDA_EA: 0x81FF344
// IDA_NAME: _ZN29Dispatcher_Score_Scroll_State12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Score_Scroll_State::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Score_Scroll_State::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    (void)packet;

    LogManager::logFormat(
        1,
        "card_flow_diag.cpp",
        "int Dispatcher_Score_Scroll_State::dispatch_sig(CUser*,PacketBuf&)",
        0,
        "[CARD_FLOW][CMD72_ENTER] uid=%d user_state=%d party=%d",
        user->GetUID(),
        user->get_state(),
        user->GetPartyIndex());

    if (user->get_state() != 5) {
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Score_Scroll_State::dispatch_sig(CUser*,PacketBuf&)",
            0, "[CARD_FLOW][CMD72_USER_STATE_REJECT] uid=%d state=%d",
            user->GetUID(), user->get_state());
        return 0;
    }

    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    if (!party) {
        return LineFunc(
            37626,
            "virtual int Dispatcher_Score_Scroll_State::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    const int recv_all = party->CheckRecvFromAllMember(user);
    if (static_cast<unsigned char>(recv_all) != 1u) {
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Score_Scroll_State::dispatch_sig(CUser*,PacketBuf&)",
            0,
            "[CARD_FLOW][CMD72_RECV_WAIT] uid=%d party=%d state=%d recv_all=%d",
            user->GetUID(), user->GetPartyIndex(),
            party->get_dungeon_clear_state(), recv_all);
        return 0;
    }

    if (party->get_dungeon_clear_state() <= 2) {
        PacketGuard out_packet;
        out_packet.clear();
        out_packet.put_header(1, ENUM_CMDPACKET_SCORE_SCROLL_STATE);
        out_packet.put_byte(1);
        out_packet.finalize(true);
        party->send_to_party(out_packet);
        party->set_dungeon_clear_state(3);
        party->insert_time_dungeon_score_scroll_msg(30);
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Score_Scroll_State::dispatch_sig(CUser*,PacketBuf&)",
            0, "[CARD_FLOW][CMD72_SENT] uid=%d party=%d state=%d",
            user->GetUID(), user->GetPartyIndex(),
            party->get_dungeon_clear_state());
        return 0;
    }

    user->getHackAnalyzer()->addServerHackCnt(
        user,
        static_cast<WongWork::ENUM_HACKTYPE>(0x197u),
        1u,
        0u,
        0u);
    return 0;
}

// IDA_EA: 0x823351C
// IDA_NAME: _ZN29Dispatcher_Score_Scroll_State4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Score_Scroll_State::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Score_Scroll_State::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233526
// IDA_NAME: _ZN29Dispatcher_Score_Scroll_State7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Score_Scroll_State::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Score_Scroll_State::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}


// IDA_EA: 0x81FFCC2
// IDA_NAME: _ZN31Dispatcher_CallGuildCreateRight12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CallGuildCreateRight::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CallGuildCreateRight::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    (void)packet;

    static const char kSignature[] =
        "virtual int Dispatcher_CallGuildCreateRight::dispatch_sig(CUser*, PacketBuf&)";

    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(37876, kSignature, 0, 0);
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 28)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT,
                                 209);
        return 0;
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(6));
    if (security_error != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT,
                                 static_cast<unsigned char>(security_error));
        return 0;
    }

    const std::string guild_name(user->getCreateGuildName());
    CSyncSlangFilter* const slang_filter = CSyncSlangFilter::GetInstance();
    if (restrict_inputting_name::isUtf8StrInUnicodeRange(
            guild_name.c_str()) != 1 ||
        slang_filter->HasSlangName(guild_name) != 0 ||
        (slang_filter->HasSlang(guild_name) == 0 &&
         G_CDataManager()->hasSpecialCharacter(guild_name.c_str()) != 0)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT,
                                 159);
        return 0;
    }

    if (user->get_charac_guildkey() != 0 ||
        user->get_guild_create_right() == 2) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT, 32);
        return 0;
    }
    if (user->isSetGuildCreateFlag(2) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT,
                                 109);
        return 0;
    }
    if (user->isSetGuildCreateFlag(4) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT,
                                 113);
        return 0;
    }

    const int guild_create_cost =
        guild_parameter_script(G_CDataManager()).guild_create_cost;
    if (user->getCurCharacInvenR()->get_money() < guild_create_cost) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT, 31);
        return 0;
    }

    user->getCurCharacInvenW()->use_money(
        guild_create_cost, static_cast<eMoneySubReason>(11), true);
    send_packet_guildcreateright(user, 0);
    user->set_guild_create_right(1);
    user->UpdateData();

    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendGuildCreate(
        static_cast<unsigned int>(user->getCurCharacNo()),
        user->getCreateGuildName(),
        user->getCreateGuildAddr());
    user->resetGuildCreateFlag();
    user->resetTempGuildNameAddr();
    return 0;
}


// IDA_EA: 0x8595C80
// IDA_NAME: _ZN29Dispatcher_RequestMemberEnterC2Ev
// DEMANGLED: Dispatcher_RequestMemberEnter::Dispatcher_RequestMemberEnter(void)
Dispatcher_RequestMemberEnter::Dispatcher_RequestMemberEnter() {}

// IDA_EA: 0x82461CE
// IDA_NAME: _ZN29Dispatcher_RequestMemberEnterD2Ev
// DEMANGLED: Dispatcher_RequestMemberEnter::~Dispatcher_RequestMemberEnter()
// IDA_EA: 0x82461FE
// IDA_NAME: _ZN29Dispatcher_RequestMemberEnterD0Ev
// DEMANGLED: Dispatcher_RequestMemberEnter::~Dispatcher_RequestMemberEnter()
Dispatcher_RequestMemberEnter::~Dispatcher_RequestMemberEnter() {}

// IDA_EA: 0x81CB780
// IDA_NAME: _ZN29Dispatcher_RequestMemberEnter4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestMemberEnter::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestMemberEnter::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_REQUEST_MEMBER_ENTER& input =
        static_cast<MSG_REQUEST_MEMBER_ENTER&>(message);
    if (packet.get_int(input.target_name_length) != 1) {
        return LineFunc(
            8553,
            "virtual int Dispatcher_RequestMemberEnter::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_str(input.target_name,
                       static_cast<int>(sizeof(input.target_name)),
                       input.target_name_length) == 1) {
        return 0;
    }
    return LineFunc(
        8556,
        "virtual int Dispatcher_RequestMemberEnter::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81CB82A
// IDA_NAME: _ZN29Dispatcher_RequestMemberEnter11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_RequestMemberEnter::check_error(CUser *,MSG_BASE &)
int Dispatcher_RequestMemberEnter::check_error(CUser* user,
                                                MSG_BASE& message) {
    (void)message;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    return 0;
}

// IDA_EA: 0x81CB874
// IDA_NAME: _ZN29Dispatcher_RequestMemberEnter7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestMemberEnter::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestMemberEnter::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param_base) {
    MSG_REQUEST_MEMBER_ENTER& input =
        static_cast<MSG_REQUEST_MEMBER_ENTER&>(message);
    stRequestMemberEnterParam& param =
        static_cast<stRequestMemberEnterParam&>(param_base);

    param.result = check_error(user, message);
    if (param.result > 0) {
        return 0;
    }
    if (param.result >= 0) {
        MonitorRequestMemberEnterPacket out_packet;
        out_packet.uid = user->GetUID();
        out_packet.charac_no = user->getCurCharacNo();
        std::strncpy(out_packet.target_name,
                     input.target_name,
                     input.target_name_length);
        monitor_proxy_for(user)->SendTcpPacket(
            reinterpret_cast<char*>(&out_packet),
            static_cast<int>(sizeof(out_packet)));
        return 0;
    }

    return LineFunc(
        8594,
        "virtual int Dispatcher_RequestMemberEnter::process(CUser*, MSG_BASE&, ParamBase&)",
        param.result,
        static_cast<unsigned int>(user->get_acc_id()));
}

// IDA_EA: 0x81CB980
// IDA_NAME: _ZN29Dispatcher_RequestMemberEnter4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_RequestMemberEnter::send(CUser *,ParamBase &)
void Dispatcher_RequestMemberEnter::send(CUser* user, ParamBase& param) {
    (void)user;
    (void)param;
}


// IDA_EA: 0x8595C9C
// IDA_NAME: _ZN27Dispatcher_MemberEnterReplyC2Ev
// DEMANGLED: Dispatcher_MemberEnterReply::Dispatcher_MemberEnterReply(void)
Dispatcher_MemberEnterReply::Dispatcher_MemberEnterReply() {}

// IDA_EA: 0x8246180
// IDA_NAME: _ZN27Dispatcher_MemberEnterReplyD2Ev
// DEMANGLED: Dispatcher_MemberEnterReply::~Dispatcher_MemberEnterReply()
// IDA_EA: 0x82461B0
// IDA_NAME: _ZN27Dispatcher_MemberEnterReplyD0Ev
// DEMANGLED: Dispatcher_MemberEnterReply::~Dispatcher_MemberEnterReply()
Dispatcher_MemberEnterReply::~Dispatcher_MemberEnterReply() {}

// IDA_EA: 0x81CB99A
// IDA_NAME: _ZN27Dispatcher_MemberEnterReply4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_MemberEnterReply::read(PacketBuf &,MSG_BASE &)
int Dispatcher_MemberEnterReply::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_MEMBER_ENTER_REPLY& input =
        static_cast<MSG_MEMBER_ENTER_REPLY&>(message);
    if (packet.get_byte(input.accepted) == 1) {
        return 0;
    }
    return LineFunc(
        8660,
        "virtual int Dispatcher_MemberEnterReply::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81CB9F0
// IDA_NAME: _ZN27Dispatcher_MemberEnterReply11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_MemberEnterReply::check_error(CUser *,MSG_BASE &)
int Dispatcher_MemberEnterReply::check_error(CUser* user,
                                              MSG_BASE& message) {
    (void)message;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    return 0;
}

// IDA_EA: 0x81CBA34
// IDA_NAME: _ZN27Dispatcher_MemberEnterReply7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_MemberEnterReply::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_MemberEnterReply::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param_base) {
    MSG_MEMBER_ENTER_REPLY& input =
        static_cast<MSG_MEMBER_ENTER_REPLY&>(message);
    stMemberEnterReplyParam& param =
        static_cast<stMemberEnterReplyParam&>(param_base);

    param.result = check_error(user, message);
    if (param.result > 0) {
        return 0;
    }
    if (param.result >= 0) {
        MonitorMemberEnterReplyPacket out_packet;
        out_packet.uid = user->GetUID();
        out_packet.charac_no = user->getCurCharacNo();
        out_packet.accepted = input.accepted;
        monitor_proxy_for(user)->SendTcpPacket(
            reinterpret_cast<char*>(&out_packet),
            static_cast<int>(sizeof(out_packet)));
        return 0;
    }

    return LineFunc(
        8687,
        "virtual int Dispatcher_MemberEnterReply::process(CUser*, MSG_BASE&, ParamBase&)",
        param.result,
        static_cast<unsigned int>(user->get_acc_id()));
}


// IDA_EA: 0x8595CB8
// IDA_NAME: _ZN23Dispatcher_MemberSecedeC2Ev
// DEMANGLED: Dispatcher_MemberSecede::Dispatcher_MemberSecede(void)
Dispatcher_MemberSecede::Dispatcher_MemberSecede() {}

// IDA_EA: 0x8246132
// IDA_NAME: _ZN23Dispatcher_MemberSecedeD2Ev
// DEMANGLED: Dispatcher_MemberSecede::~Dispatcher_MemberSecede()
// IDA_EA: 0x8246162
// IDA_NAME: _ZN23Dispatcher_MemberSecedeD0Ev
// DEMANGLED: Dispatcher_MemberSecede::~Dispatcher_MemberSecede()
Dispatcher_MemberSecede::~Dispatcher_MemberSecede() {}

// IDA_EA: 0x81CBB26
// IDA_NAME: _ZN23Dispatcher_MemberSecede4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_MemberSecede::read(PacketBuf &,MSG_BASE &)
int Dispatcher_MemberSecede::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_MEMBER_SECEDE& input = static_cast<MSG_MEMBER_SECEDE&>(message);
    if (packet.get_int(input.target_name_length) != 1) {
        return LineFunc(
            8713,
            "virtual int Dispatcher_MemberSecede::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_str(input.target_name,
                       static_cast<int>(sizeof(input.target_name)),
                       input.target_name_length) == 1) {
        return 0;
    }
    return LineFunc(
        8716,
        "virtual int Dispatcher_MemberSecede::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81CBBD0
// IDA_NAME: _ZN23Dispatcher_MemberSecede11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_MemberSecede::check_error(CUser *,MSG_BASE &)
int Dispatcher_MemberSecede::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    if (user->get_charac_memberkey()) {
        return 0;
    }
    return 0x7fffffff;
}

// IDA_EA: 0x81CBC2E
// IDA_NAME: _ZN23Dispatcher_MemberSecede7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_MemberSecede::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_MemberSecede::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param_base) {
    MSG_MEMBER_SECEDE& input = static_cast<MSG_MEMBER_SECEDE&>(message);
    stMemberSecedeParam& param =
        static_cast<stMemberSecedeParam&>(param_base);

    param.result = check_error(user, message);
    if (param.result > 0) {
        return 0;
    }
    if (param.result >= 0) {
        MonitorMemberSecedePacket out_packet;
        out_packet.uid = user->GetUID();
        out_packet.charac_no = user->getCurCharacNo();
        std::memcpy(out_packet.target_name,
                    input.target_name,
                    input.target_name_length);
        monitor_proxy_for(user)->SendTcpPacket(
            reinterpret_cast<char*>(&out_packet),
            static_cast<int>(sizeof(out_packet)));
        return 0;
    }

    return LineFunc(
        8744,
        "virtual int Dispatcher_MemberSecede::process(CUser*, MSG_BASE&, ParamBase&)",
        param.result,
        static_cast<unsigned int>(user->get_acc_id()));
}


// IDA_EA: 0x8595CD4
// IDA_NAME: _ZN25Dispatcher_CallMemberListC2Ev
// DEMANGLED: Dispatcher_CallMemberList::Dispatcher_CallMemberList(void)
Dispatcher_CallMemberList::Dispatcher_CallMemberList() {}

// IDA_EA: 0x82460E4
// IDA_NAME: _ZN25Dispatcher_CallMemberListD2Ev
// DEMANGLED: Dispatcher_CallMemberList::~Dispatcher_CallMemberList()
// IDA_EA: 0x8246114
// IDA_NAME: _ZN25Dispatcher_CallMemberListD0Ev
// DEMANGLED: Dispatcher_CallMemberList::~Dispatcher_CallMemberList()
Dispatcher_CallMemberList::~Dispatcher_CallMemberList() {}

// IDA_EA: 0x81CBD3A
// IDA_NAME: _ZN25Dispatcher_CallMemberList4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CallMemberList::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CallMemberList::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81CBD44
// IDA_NAME: _ZN25Dispatcher_CallMemberList11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_CallMemberList::check_error(CUser *,MSG_BASE &)
int Dispatcher_CallMemberList::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }
    if (user->get_charac_memberkey()) {
        return 0;
    }
    return 0x7fffffff;
}

// IDA_EA: 0x81CBDA2
// IDA_NAME: _ZN25Dispatcher_CallMemberList7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CallMemberList::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CallMemberList::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param_base) {
    stCallMemberListParam& param =
        static_cast<stCallMemberListParam&>(param_base);

    param.result = check_error(user, message);
    if (param.result > 0) {
        return 0;
    }
    if (param.result >= 0) {
        MonitorCallMemberListPacket out_packet;
        out_packet.uid = user->GetUID();
        out_packet.charac_no = user->getCurCharacNo();
        monitor_proxy_for(user)->SendPacket(
            reinterpret_cast<char*>(&out_packet),
            static_cast<int>(sizeof(out_packet)));
        return 0;
    }

    return LineFunc(
        8795,
        "virtual int Dispatcher_CallMemberList::process(CUser*, MSG_BASE&, ParamBase&)",
        param.result,
        static_cast<unsigned int>(user->get_acc_id()));
}


// The following lifecycle/read/process bodies are deliberately kept as their
// exact old-IDB trivial bodies.  Their corresponding dispatch_sig methods are
// not emitted until the separately listed typed ABI boundaries exist.

// IDA_EA: 0x8595548
// IDA_NAME: _ZN19DisPatcher_SendMessC2Ev
// DEMANGLED: DisPatcher_SendMess::DisPatcher_SendMess(void)
DisPatcher_SendMess::DisPatcher_SendMess() {}

// IDA_EA: 0x8232F7A
// IDA_NAME: _ZN19DisPatcher_SendMessD2Ev
// DEMANGLED: DisPatcher_SendMess::~DisPatcher_SendMess()
// IDA_EA: 0x8232FAA
// IDA_NAME: _ZN19DisPatcher_SendMessD0Ev
// DEMANGLED: DisPatcher_SendMess::~DisPatcher_SendMess()
DisPatcher_SendMess::~DisPatcher_SendMess() {}

// IDA_EA: 0x8232F66
// IDA_NAME: _ZN19DisPatcher_SendMess4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SendMess::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SendMess::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8232F70
// IDA_NAME: _ZN19DisPatcher_SendMess7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SendMess::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SendMess::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// The B-owned database-dispatch provider is compiled exactly once from
// old_data/db/legacy_dispatch/packet_dispatcher_send_mess_runtime.cpp.  Keep
// this historical group copy out of the C object so the shared vtable binds
// to that canonical provider without a duplicate definition.
#if 0
// IDA_EA: 0x81F3540
// IDA_NAME: _ZN19DisPatcher_SendMess12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_SendMess::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_SendMess::dispatch_sig(CUser* user, PacketBuf& packet) {
    static const char kSignature[] =
        "virtual int DisPatcher_SendMess::dispatch_sig(CUser*, PacketBuf&)";

    if (user->get_state() <= 2) {
        return 0;
    }

    unsigned char chat_type = 0;
    uint16_t item_space = 0;
    uint32_t item_position = 0;
    int message_length = 0;
    char message[256];
    if (packet.get_byte(chat_type) != 1) {
        return LineFunc(32501, kSignature, 0, 0);
    }
    if (packet.get_short(item_space) != 1) {
        return LineFunc(32502, kSignature, 0, 0);
    }
    if (packet.get_int(item_position) != 1) {
        return LineFunc(32503, kSignature, 0, 0);
    }
    if (packet.get_int(message_length) != 1) {
        return LineFunc(32504, kSignature, 0, 0);
    }
    if (packet.get_str(message, sizeof(message), message_length) != 1) {
        return LineFunc(32505, kSignature, 0, 0);
    }

    G_CDataManager()->rewriteUsableEmoticon(*user, message);
    if (user->IsCurCharacGhost() &&
        (chat_type > 8 || ((1u << chat_type) & 0x144u) == 0)) {
        return 0;
    }

    int target_name_length = 0;
    char target_name[30];
    if (chat_type == 36 || chat_type == 37 || chat_type == 38) {
        unsigned char sender_marker = 0;
        if (packet.get_int(target_name_length) != 1) {
            return LineFunc(32587, kSignature, 0, 0);
        }
        if (packet.get_str(target_name, sizeof(target_name),
                           target_name_length) != 1) {
            return LineFunc(32589, kSignature, 0, 0);
        }
        if (packet.get_byte(sender_marker) != 1) {
            return LineFunc(32591, kSignature, 0, 0);
        }

        Packet_Monitor_Other_Channel_Chat out_packet;
        const unsigned char monitor_chat_type =
            chat_type == 36 ? 7 : (chat_type == 37 ? 33 : 1);
        BuildOtherChannelChat(out_packet,
                              monitor_chat_type,
                              static_cast<uint32_t>(user->get_acc_id()),
                              sender_marker,
                              static_cast<uint32_t>(user->getCurCharacNo()),
                              item_position,
                              target_name,
                              target_name_length,
                              message,
                              message_length);
        GlobalData::s_community_proxy->SendTcpPacket(
            reinterpret_cast<char*>(&out_packet),
            OtherChannelChatWireSize(message_length));
        return 0;
    }

    if (chat_type == 7 || chat_type == 1 || chat_type == 33) {
        unsigned char ignored_sender_marker = 0;
        if (packet.get_int(target_name_length) != 1) {
            return LineFunc(32625, kSignature, 0, 0);
        }
        if (packet.get_str(target_name, sizeof(target_name),
                           target_name_length) != 1) {
            return LineFunc(32627, kSignature, 0, 0);
        }
        if (packet.get_byte(ignored_sender_marker) != 1) {
            return LineFunc(32632, kSignature, 0, 0);
        }

        if (chat_type == 7) {
            Packet_Monitor_Other_Channel_Chat out_packet;
            out_packet.charac_no =
                static_cast<uint32_t>(user->getCurCharacNo());
            out_packet.target_name_length =
                static_cast<unsigned char>(target_name_length);
            std::memcpy(out_packet.target_name,
                        target_name,
                        target_name_length);
            out_packet.message_length =
                static_cast<unsigned char>(message_length);
            std::memcpy(out_packet.message, message, message_length);

            const std::string filtered =
                CSyncSlangFilter::GetInstance()->Filter(std::string(message));
            if (!filtered.empty()) {
                std::strncpy(out_packet.message,
                             filtered.c_str(),
                             filtered.size());
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
                    ->SendPacket(reinterpret_cast<char*>(&out_packet),
                                 OtherChannelChatWireSize(message_length));
            }
            return 0;
        }
    } else {
        const int inventory_type = GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(item_space));
        const Inven_Item* const inventory_item =
            user->getCurCharacInvenR()->GetInvenRef(
                inventory_type, static_cast<int>(item_position));

        switch (chat_type) {
        case 11:
        case 12:
        case 35:
            if (inventory_item &&
                IsChatUseItemId(inventory_item->getKey())) {
                const int security_error =
                    GlobalData::s_pSecuProtectionField->Check(
                        user, static_cast<SECURITY_PROTCTION>(36));
                if (security_error) {
                    user->SendCmdErrorPacket(
                        ENUM_CMDPACKET_SEND_MESSAGE,
                        static_cast<unsigned char>(security_error));
                    return 0;
                }
                if (user->CheckItemLock(inventory_type,
                                        static_cast<int>(item_position))) {
                    user->SendCmdErrorPacket(ENUM_CMDPACKET_SEND_MESSAGE, 213);
                    return 0;
                }
                if (user->use_stackable(static_cast<short>(item_position),
                                        static_cast<int>(item_space),
                                        -1,
                                        -1) == 1) {
                    goto send_chat;
                }
            }
            break;

        case 13:
        case 15:
            if (!inventory_item) {
                return LineFunc(32800, kSignature, 0, 0);
            }
            if ((inventory_item->getKey() != 37 ||
                 chat_type != 13) &&
                (inventory_item->getKey() != 2660655 ||
                 chat_type != 15)) {
                return LineFunc(32805, kSignature, 0, 0);
            }
            {
                const int security_error =
                    GlobalData::s_pSecuProtectionField->Check(
                        user, static_cast<SECURITY_PROTCTION>(36));
                if (security_error) {
                    user->SendCmdErrorPacket(
                        ENUM_CMDPACKET_SEND_MESSAGE,
                        static_cast<unsigned char>(security_error));
                    return 0;
                }
            }
            if (user->CheckItemLock(inventory_type,
                                    static_cast<int>(item_position))) {
                user->SendCmdErrorPacket(ENUM_CMDPACKET_SEND_MESSAGE, 213);
                return 0;
            }
            if (user->use_stackable(static_cast<short>(item_position),
                                    static_cast<int>(item_space),
                                    -1,
                                    -1) == 1) {
                goto send_chat;
            }
            break;

        case 14:
            if (!inventory_item || inventory_item->getKey() != 1120) {
                return LineFunc(32851, kSignature, 0, 0);
            }
            if (!GlobalData::s_pGMAccounts_->isGM(
                    static_cast<uint32_t>(user->get_acc_id()))) {
                return LineFunc(32855, kSignature, 0, 0);
            }
            if (user->use_stackable(static_cast<short>(item_position),
                                    static_cast<int>(item_space),
                                    -1,
                                    -1) == 1) {
                goto send_chat;
            }
            break;

        case 34:
            if (user->get_state() != 3) {
                return 0;
            }
            if (!inventory_item || inventory_item->getKey() != 7582) {
                return LineFunc(32874, kSignature, 0, 0);
            }
            {
                CItem* const item = G_CDataManager()->find_item(
                    inventory_item->getKey());
                if (item && item->is_stackable() == 1 &&
                    user->getCurCharacInvenW()->delete_item(
                        static_cast<INVEN_TYPE>(inventory_type),
                        static_cast<int>(item_position),
                        1,
                        static_cast<eItemDelReason>(3),
                        true) == 1) {
                    goto send_chat;
                }
            }
            break;

        default:
            goto send_chat;
        }

        user->SendCmdErrorPacket(ENUM_CMDPACKET_SEND_MESSAGE, 17);
        return 0;
    }

send_chat:
    if (GlobalData::s_pGMAccounts_->isGM(
            static_cast<uint32_t>(user->get_acc_id()))) {
        DB_InsertPoliceSaveChatting::makeRequest(
            static_cast<uint32_t>(user->getCurCharacNo()),
            static_cast<char>(chat_type),
            message);
    }
    G_GameWorld()->send_chat_msg(user,
                                 static_cast<char>(chat_type),
                                 item_space,
                                 item_position,
                                 message,
                                 message_length,
                                 target_name,
                                 target_name_length,
                                 static_cast<ENUM_NOTIPACKET>(12));
    return 0;
}

#endif  // canonical B DisPatcher_SendMess::dispatch_sig provider


// IDA_EA: 0x8595C10
// IDA_NAME: _ZN27Dispatcher_CallGuildMembersC2Ev
// DEMANGLED: Dispatcher_CallGuildMembers::Dispatcher_CallGuildMembers(void)
Dispatcher_CallGuildMembers::Dispatcher_CallGuildMembers() {}

// IDA_EA: 0x824341A
// IDA_NAME: _ZN27Dispatcher_CallGuildMembersD2Ev
// DEMANGLED: Dispatcher_CallGuildMembers::~Dispatcher_CallGuildMembers()
// IDA_EA: 0x824344A
// IDA_NAME: _ZN27Dispatcher_CallGuildMembersD0Ev
// DEMANGLED: Dispatcher_CallGuildMembers::~Dispatcher_CallGuildMembers()
Dispatcher_CallGuildMembers::~Dispatcher_CallGuildMembers() {}

// IDA_EA: 0x8233558
// IDA_NAME: _ZN27Dispatcher_CallGuildMembers4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CallGuildMembers::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CallGuildMembers::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233562
// IDA_NAME: _ZN27Dispatcher_CallGuildMembers7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CallGuildMembers::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CallGuildMembers::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x81FFA56
// IDA_NAME: _ZN27Dispatcher_CallGuildMembers12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CallGuildMembers::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CallGuildMembers::dispatch_sig(CUser* user,
                                              PacketBuf& packet) {
    (void)packet;

    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(37824, kCallGuildMembersSignature, 0, 0);
    }

    const unsigned int charac_guildkey =
        static_cast<unsigned int>(user->get_charac_guildkey());
    if (charac_guildkey) {
        CGuildServerProxy* const proxy =
            GlobalData::s_guild_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        proxy->SendPacketCallGuildMembers(
            static_cast<unsigned int>(user->getCurCharacNo()),
            charac_guildkey);
    }
    return 0;
}


// IDA_EA: 0x8595C2C
// IDA_NAME: _ZN31Dispatcher_CallGuildCreateRightC2Ev
// DEMANGLED: Dispatcher_CallGuildCreateRight::Dispatcher_CallGuildCreateRight(void)
Dispatcher_CallGuildCreateRight::Dispatcher_CallGuildCreateRight() {}

// IDA_EA: 0x824337E
// IDA_NAME: _ZN31Dispatcher_CallGuildCreateRightD2Ev
// DEMANGLED: Dispatcher_CallGuildCreateRight::~Dispatcher_CallGuildCreateRight()
// IDA_EA: 0x82433AE
// IDA_NAME: _ZN31Dispatcher_CallGuildCreateRightD0Ev
// DEMANGLED: Dispatcher_CallGuildCreateRight::~Dispatcher_CallGuildCreateRight()
Dispatcher_CallGuildCreateRight::~Dispatcher_CallGuildCreateRight() {}

// IDA_EA: 0x81FFBDE
// IDA_NAME: _ZN31Dispatcher_CallGuildCreateRight28send_packet_guildcreaterightEP5CUserh
// DEMANGLED: Dispatcher_CallGuildCreateRight::send_packet_guildcreateright(CUser *,unsigned char)
void Dispatcher_CallGuildCreateRight::send_packet_guildcreateright(
    CUser* user,
    unsigned char result) {
    (void)result;
    PacketGuard out_packet;
    out_packet.clear();
    out_packet.put_header(0, 14);
    out_packet.put_byte(0);
    out_packet.put_short(1);
    user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(1), 0, out_packet);
    out_packet.finalize(true);
    user->Send(out_packet);
}

// IDA_EA: 0x8233580
// IDA_NAME: _ZN31Dispatcher_CallGuildCreateRight4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CallGuildCreateRight::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CallGuildCreateRight::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823358A
// IDA_NAME: _ZN31Dispatcher_CallGuildCreateRight7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CallGuildCreateRight::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CallGuildCreateRight::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}


// IDA_EA: 0x8595BD8
// IDA_NAME: _ZN34Dispatcher_Card_Select_Right_StateC2Ev
// DEMANGLED: Dispatcher_Card_Select_Right_State::Dispatcher_Card_Select_Right_State(void)
Dispatcher_Card_Select_Right_State::Dispatcher_Card_Select_Right_State() {}

// IDA_EA: 0x82434B6
// IDA_NAME: _ZN34Dispatcher_Card_Select_Right_StateD2Ev
// DEMANGLED: Dispatcher_Card_Select_Right_State::~Dispatcher_Card_Select_Right_State()
// IDA_EA: 0x82434E6
// IDA_NAME: _ZN34Dispatcher_Card_Select_Right_StateD0Ev
// DEMANGLED: Dispatcher_Card_Select_Right_State::~Dispatcher_Card_Select_Right_State()
Dispatcher_Card_Select_Right_State::~Dispatcher_Card_Select_Right_State() {}

// IDA_EA: 0x8233530
// IDA_NAME: _ZN34Dispatcher_Card_Select_Right_State4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Card_Select_Right_State::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Card_Select_Right_State::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823353A
// IDA_NAME: _ZN34Dispatcher_Card_Select_Right_State7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Card_Select_Right_State::process(CUser* user, MSG_BASE&, ParamBase&)
int Dispatcher_Card_Select_Right_State::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x81FF504
// IDA_NAME: _ZN34Dispatcher_Card_Select_Right_State12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Card_Select_Right_State::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Card_Select_Right_State::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    (void)packet;

    LogManager::logFormat(
        1,
        "card_flow_diag.cpp",
        "int Dispatcher_Card_Select_Right_State::dispatch_sig(CUser*,PacketBuf&)",
        0,
        "[CARD_FLOW][CMD73_ENTER] uid=%d user_state=%d move=%d party=%d",
        user->GetUID(), user->get_state(), user->getMoveSpace(),
        user->GetPartyIndex());

    if (user->get_state() != 5 || user->getMoveSpace() == 1) {
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Card_Select_Right_State::dispatch_sig(CUser*,PacketBuf&)",
            0,
            "[CARD_FLOW][CMD73_USER_REJECT] uid=%d state=%d move=%d",
            user->GetUID(), user->get_state(), user->getMoveSpace());
        return 0;
    }

    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    if (!party) {
        return LineFunc(
            37672,
            "virtual int Dispatcher_Card_Select_Right_State::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    CDungeon* const dungeon = party->battle_field().GetDungeon();
    if (dungeon && dungeon->isTowerOfDespairDungeon()) {
        return 0;
    }

    const int recv_all = party->CheckRecvFromAllMember(user);
    if (static_cast<unsigned char>(recv_all) != 1u) {
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Card_Select_Right_State::dispatch_sig(CUser*,PacketBuf&)",
            0,
            "[CARD_FLOW][CMD73_RECV_WAIT] uid=%d party=%d state=%d recv_all=%d",
            user->GetUID(), user->GetPartyIndex(),
            party->get_dungeon_clear_state(), recv_all);
        return 0;
    }

    if (party->get_dungeon_clear_state() <= 3) {
        PacketGuard out_packet;
        out_packet.put_header(1, ENUM_CMDPACKET_CARD_SELECT_RIGHT_STATE);
        out_packet.put_byte(1);
        champion_break::WriteCardSelectRights(party, out_packet);
        out_packet.finalize(true);
        party->send_to_party(out_packet);
        party->set_dungeon_clear_state(4);
        party->insert_time_card_select_right_msg(30);
        party->insert_time_card_select_msg(4);
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Card_Select_Right_State::dispatch_sig(CUser*,PacketBuf&)",
            0, "[CARD_FLOW][CMD73_SENT] uid=%d party=%d state=%d",
            user->GetUID(), user->GetPartyIndex(),
            party->get_dungeon_clear_state());
        return 0;
    }

    user->getHackAnalyzer()->addServerHackCnt(
        user,
        static_cast<WongWork::ENUM_HACKTYPE>(0x197u),
        1u,
        0u,
        0u);
    return 0;
}


// IDA_EA: 0x8595BF4
// IDA_NAME: _ZN22Dispatcher_Select_CardC2Ev
// DEMANGLED: Dispatcher_Select_Card::Dispatcher_Select_Card(void)
Dispatcher_Select_Card::Dispatcher_Select_Card() {}

// IDA_EA: 0x8243468
// IDA_NAME: _ZN22Dispatcher_Select_CardD2Ev
// DEMANGLED: Dispatcher_Select_Card::~Dispatcher_Select_Card()
// IDA_EA: 0x8243498
// IDA_NAME: _ZN22Dispatcher_Select_CardD0Ev
// DEMANGLED: Dispatcher_Select_Card::~Dispatcher_Select_Card()
Dispatcher_Select_Card::~Dispatcher_Select_Card() {}

// IDA_EA: 0x81FF7DE
// IDA_NAME: _ZN22Dispatcher_Select_Card12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Select_Card::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Select_Card::dispatch_sig(CUser* user, PacketBuf& packet) {
    LogManager::logFormat(
        1,
        "card_flow_diag.cpp",
        "int Dispatcher_Select_Card::dispatch_sig(CUser*,PacketBuf&)",
        0,
        "[CARD_FLOW][CMD71_ENTER] uid=%d user_state=%d move=%d party=%d index=%d",
        user->GetUID(), user->get_state(), user->getMoveSpace(),
        user->GetPartyIndex(), packet.get_index());
    if (user->get_state() != 5) {
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Select_Card::dispatch_sig(CUser*,PacketBuf&)", 0,
            "[CARD_FLOW][CMD71_USER_STATE_REJECT] uid=%d state=%d",
            user->GetUID(), user->get_state());
        return 0;
    }
    if (user->getMoveSpace() == 1) {
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Select_Card::dispatch_sig(CUser*,PacketBuf&)", 0,
            "[CARD_FLOW][CMD71_MOVE_REJECT] uid=%d", user->GetUID());
        return 0;
    }

    CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
    if (!party) {
        return LineFunc(37745, kSelectCardSignature, 0, 0);
    }
    CDungeon* const dungeon = party->battle_field().GetDungeon();
    if (dungeon &&
        static_cast<unsigned char>(
            dungeon->isTowerOfDespairDungeon()) != 0u) {
        return 0;
    }

    char reward_card_type = 0;
    if (static_cast<unsigned char>(packet.get_byte(reward_card_type)) != 1u) {
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Select_Card::dispatch_sig(CUser*,PacketBuf&)", 0,
            "[CARD_FLOW][CMD71_TYPE_READ_REJECT] uid=%d index=%d",
            user->GetUID(), packet.get_index());
        return LineFunc(37762, kSelectCardSignature, 0, 0);
    }
    if (static_cast<unsigned char>(reward_card_type) >= 2u) {
        return LineFunc(37764, kSelectCardSignature, 0, 0);
    }

    char card = 0;
    if (static_cast<unsigned char>(packet.get_byte(card)) != 1u) {
        return LineFunc(37778, kSelectCardSignature, 0, 0);
    }
    // CLIENT_IDA_EVIDENCE: state 9 retains the normal upper row as type 0 and
    // routes the ChampionBreak lower row through type 1.  The one-byte CMD71
    // producer belongs to the separately gated SAO clear path.
    if (static_cast<unsigned char>(reward_card_type) == 1u &&
        champion_break::IsCombinedCardStage(party) &&
        champion_break::TryHandleCardSelect(
            user, party, static_cast<unsigned char>(card))) {
        return 0;
    }
    if (static_cast<signed char>(party->get_dungeon_clear_state()) > 4) {
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Select_Card::dispatch_sig(CUser*,PacketBuf&)", 0,
            "[CARD_FLOW][CMD71_CLEAR_STATE_REJECT] uid=%d party=%d state=%d type=%d card=%d",
            user->GetUID(), user->GetPartyIndex(),
            party->get_dungeon_clear_state(),
            static_cast<int>(reward_card_type), static_cast<int>(card));
        return 0;
    }

    LogManager::logFormat(
        1, "card_flow_diag.cpp",
        "int Dispatcher_Select_Card::dispatch_sig(CUser*,PacketBuf&)", 0,
        "[CARD_FLOW][CMD71_PARSED] uid=%d party=%d state=%d type=%d card=%d",
        user->GetUID(), user->GetPartyIndex(),
        party->get_dungeon_clear_state(), static_cast<int>(reward_card_type),
        static_cast<int>(card));

    const eClearRewardCardType_t card_type =
        static_cast<eClearRewardCardType_t>(reward_card_type);
    if (static_cast<unsigned char>(
            party->IsCompleteCardSelectOfMembers(
                static_cast<eClearRewardCardType_t>(2))) != 1u) {
        const int set_result = party->SetCardNumber(user, card, card_type);
        if (static_cast<unsigned char>(set_result) != 1u) {
            LogManager::logFormat(
                1, "card_flow_diag.cpp",
                "int Dispatcher_Select_Card::dispatch_sig(CUser*,PacketBuf&)",
                0,
                "[CARD_FLOW][CMD71_SET_REJECT] uid=%d party=%d state=%d type=%d card=%d result=%d",
                user->GetUID(), user->GetPartyIndex(),
                party->get_dungeon_clear_state(),
                static_cast<int>(reward_card_type), static_cast<int>(card),
                set_result);
            return 0;
        }
        if (champion_break::IsCombinedCardStage(party)) {
            champion_break::SendCombinedCardInfo(party);
        } else {
            party->SendEachCardInfo();
        }
        LogManager::logFormat(
            1, "card_flow_diag.cpp",
            "int Dispatcher_Select_Card::dispatch_sig(CUser*,PacketBuf&)", 0,
            "[CARD_FLOW][CMD71_SENT] uid=%d party=%d state=%d type=%d card=%d",
            user->GetUID(), user->GetPartyIndex(),
            party->get_dungeon_clear_state(),
            static_cast<int>(reward_card_type), static_cast<int>(card));
    }

    if (champion_break::IsCombinedCardStage(party)) {
        champion_break::FinishCombinedCardSelectIfReady(party);
        return 0;
    }

    if (static_cast<unsigned char>(party->CheckRecvFromAllMember(user)) != 1u) {
        return 0;
    }
    if (static_cast<unsigned char>(
            party->IsCompleteCardSelectOfMembers(
                static_cast<eClearRewardCardType_t>(2))) == 1u) {
        party->set_dungeon_clear_state(5);
        party->dungeon_clear();
    }
    return 0;
}

// IDA_EA: 0x8233544
// IDA_NAME: _ZN22Dispatcher_Select_Card4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Select_Card::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Select_Card::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823354E
// IDA_NAME: _ZN22Dispatcher_Select_Card7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Select_Card::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Select_Card::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}


// IDA_EA: 0x8595C48
// IDA_NAME: _ZN27Dispatcher_CallGuildLevelUpC2Ev
// DEMANGLED: Dispatcher_CallGuildLevelUp::Dispatcher_CallGuildLevelUp(void)
Dispatcher_CallGuildLevelUp::Dispatcher_CallGuildLevelUp() {}

// IDA_EA: 0x8243330
// IDA_NAME: _ZN27Dispatcher_CallGuildLevelUpD2Ev
// DEMANGLED: Dispatcher_CallGuildLevelUp::~Dispatcher_CallGuildLevelUp()
// IDA_EA: 0x8243360
// IDA_NAME: _ZN27Dispatcher_CallGuildLevelUpD0Ev
// DEMANGLED: Dispatcher_CallGuildLevelUp::~Dispatcher_CallGuildLevelUp()
Dispatcher_CallGuildLevelUp::~Dispatcher_CallGuildLevelUp() {}

// IDA_EA: 0x8200126
// IDA_NAME: _ZN27Dispatcher_CallGuildLevelUp25send_packet_guild_levelupEP5CUserh
// DEMANGLED: Dispatcher_CallGuildLevelUp::send_packet_guild_levelup(CUser *,unsigned char)
void Dispatcher_CallGuildLevelUp::send_packet_guild_levelup(
    CUser* user,
    unsigned char error) {
    PacketGuard out_packet;
    out_packet.put_header(1, ENUM_CMDPACKET_CALL_GUILD_LEVEL_UP);
    out_packet.put_byte(error ? 0 : 1);
    out_packet.put_byte(error);
    out_packet.finalize(true);
    user->Send(out_packet);
}

// IDA_EA: 0x8233594
// IDA_NAME: _ZN27Dispatcher_CallGuildLevelUp4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CallGuildLevelUp::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CallGuildLevelUp::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823359E
// IDA_NAME: _ZN27Dispatcher_CallGuildLevelUp7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CallGuildLevelUp::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CallGuildLevelUp::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x82001F0
// IDA_NAME: _ZN27Dispatcher_CallGuildLevelUp12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CallGuildLevelUp::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CallGuildLevelUp::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;

    static const char kSignature[] =
        "virtual int Dispatcher_CallGuildLevelUp::dispatch_sig(CUser*, PacketBuf&)";

    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(38015, kSignature, 0, 0);
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 29)) {
        send_packet_guild_levelup(user, 0xd1u);
        return 0;
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(7));
    if (security_error != 0) {
        send_packet_guild_levelup(user,
                                  static_cast<unsigned char>(security_error));
        return 0;
    }

    unsigned int levelup_price = 0;
    const int check_result = user->CheckGuildLevelUp(levelup_price);
    if (check_result != 0) {
        send_packet_guild_levelup(user,
                                  static_cast<unsigned char>(check_result));
        return 0;
    }

    send_packet_guild_levelup(user, 0);

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    const unsigned int guild_key =
        static_cast<unsigned int>(user->get_charac_guildkey());
    proxy->SendGuildLevelup(user->GetUID(),
                            static_cast<unsigned int>(user->getCurCharacNo()),
                            guild_key,
                            levelup_price);

    STGuildDBInfoOnly* const guild_db_info = user->GetGuildDBInfo();
    STGuildProxy guild_proxy;
    std::memcpy(guild_proxy.guild_name,
                guild_db_info->guild_name,
                sizeof(guild_proxy.guild_name) - 1u);
    guild_proxy.guild_level =
        static_cast<uint8_t>(guild_db_info->guild_level + 1u);
    guild_proxy.power_side = guild_db_info->power_side;
    proxy->ModifyGuildInfoProxy(guild_key, guild_proxy);
    return 0;
}


// IDA_EA: 0x8595C64
// IDA_NAME: _ZN24Dispatcher_CallGuildInfoC2Ev
// DEMANGLED: Dispatcher_CallGuildInfo::Dispatcher_CallGuildInfo(void)
Dispatcher_CallGuildInfo::Dispatcher_CallGuildInfo() {}

// IDA_EA: 0x82432E2
// IDA_NAME: _ZN24Dispatcher_CallGuildInfoD2Ev
// DEMANGLED: Dispatcher_CallGuildInfo::~Dispatcher_CallGuildInfo()
// IDA_EA: 0x8243312
// IDA_NAME: _ZN24Dispatcher_CallGuildInfoD0Ev
// DEMANGLED: Dispatcher_CallGuildInfo::~Dispatcher_CallGuildInfo()
Dispatcher_CallGuildInfo::~Dispatcher_CallGuildInfo() {}

// IDA_EA: 0x82335A8
// IDA_NAME: _ZN24Dispatcher_CallGuildInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CallGuildInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CallGuildInfo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82335B2
// IDA_NAME: _ZN24Dispatcher_CallGuildInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CallGuildInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CallGuildInfo::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8200448
// IDA_NAME: _ZN24Dispatcher_CallGuildInfo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CallGuildInfo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CallGuildInfo::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(38119, kCallGuildInfoSignature, 0, 0);
    }

    unsigned char guild_count = 0;
    if (packet.get_byte(guild_count) != 1) {
        return LineFunc(38124, kCallGuildInfoSignature, 0, 0);
    }
    if (guild_count > 0x5Fu) {
        return LineFunc(38128, kCallGuildInfoSignature, 0, 0);
    }

    unsigned int guild_keys[95] = {};
    for (int index = 0; index < guild_count; ++index) {
        if (packet.get_int(guild_keys[index]) != 1) {
            return LineFunc(38135, kCallGuildInfoSignature, 0, 0);
        }
        if (!guild_keys[index]) {
            PacketGuard reply;
            reply.put_header(1, ENUM_CMDPACKET_GUILD_INFO);
            reply.put_byte(0);
            reply.put_byte(34);
            reply.finalize(true);
            user->Send(reply);
            return 0;
        }
    }

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendCallGuildInfo(static_cast<unsigned int>(user->getCurCharacNo()),
                             static_cast<unsigned int>(user->GetUID()),
                             guild_count,
                             guild_keys);
    return 0;
}


// IDA_EA: 0x8595FAC
// IDA_NAME: _ZN30DisPatcher_RequestGuildWarInfoC2Ev
// DEMANGLED: DisPatcher_RequestGuildWarInfo::DisPatcher_RequestGuildWarInfo(void)
DisPatcher_RequestGuildWarInfo::DisPatcher_RequestGuildWarInfo() {}

// IDA_EA: 0x8242CCA
// IDA_NAME: _ZN30DisPatcher_RequestGuildWarInfoD2Ev
// DEMANGLED: DisPatcher_RequestGuildWarInfo::~DisPatcher_RequestGuildWarInfo()
// IDA_EA: 0x8242CFA
// IDA_NAME: _ZN30DisPatcher_RequestGuildWarInfoD0Ev
// DEMANGLED: DisPatcher_RequestGuildWarInfo::~DisPatcher_RequestGuildWarInfo()
DisPatcher_RequestGuildWarInfo::~DisPatcher_RequestGuildWarInfo() {}

// IDA_EA: 0x823374C
// IDA_NAME: _ZN30DisPatcher_RequestGuildWarInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_RequestGuildWarInfo::read(PacketBuf &,MSG_BASE &)
int DisPatcher_RequestGuildWarInfo::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233756
// IDA_NAME: _ZN30DisPatcher_RequestGuildWarInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_RequestGuildWarInfo::process(CUser* user, MSG_BASE&, ParamBase&)
int DisPatcher_RequestGuildWarInfo::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x820439A
// IDA_NAME: _ZN30DisPatcher_RequestGuildWarInfo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_RequestGuildWarInfo::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_RequestGuildWarInfo::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    (void)packet;

    if (user->get_state() > 1 && user->getCurCharacR()) {
        Packet_Notice_Guild_War_Info notice;
        notice.uid = static_cast<unsigned int>(user->GetUID());
        notice.charac_no = static_cast<unsigned int>(user->get_charac_no(-1));

        CGuildServerProxy* const proxy =
            GlobalData::s_guild_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        char wire_bytes[sizeof(notice)];
        std::memcpy(wire_bytes, &notice, sizeof(wire_bytes));
        proxy->SendPacket(wire_bytes, sizeof(wire_bytes));
    }
    return 0;
}


// IDA_EA: 0x8596364
// IDA_NAME: _ZN29Dispatcher_RequestGuildSecedeC2Ev
// DEMANGLED: Dispatcher_RequestGuildSecede::Dispatcher_RequestGuildSecede(void)
Dispatcher_RequestGuildSecede::Dispatcher_RequestGuildSecede() {}

// IDA_EA: 0x824252C
// IDA_NAME: _ZN29Dispatcher_RequestGuildSecedeD2Ev
// DEMANGLED: Dispatcher_RequestGuildSecede::~Dispatcher_RequestGuildSecede()
// IDA_EA: 0x824255C
// IDA_NAME: _ZN29Dispatcher_RequestGuildSecedeD0Ev
// DEMANGLED: Dispatcher_RequestGuildSecede::~Dispatcher_RequestGuildSecede()
Dispatcher_RequestGuildSecede::~Dispatcher_RequestGuildSecede() {}

// IDA_EA: 0x8207D3A
// IDA_NAME: _ZN29Dispatcher_RequestGuildSecede12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RequestGuildSecede::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestGuildSecede::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    static const char kSignature[] =
        "virtual int Dispatcher_RequestGuildSecede::dispatch_sig(CUser*, PacketBuf&)";

    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(41631, kSignature, 0, 0);
    }

    const unsigned int guild_key =
        static_cast<unsigned int>(user->get_charac_guildkey());
    if (!guild_key) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQ_GUILD_SECEDE, 34);
        return 0;
    }
    if (user->getCurCharacVill() == 7) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQ_GUILD_SECEDE, 19);
        return 0;
    }

    int text_length = 0;
    char text[30];
    if (packet.get_int(text_length) != 1) {
        return LineFunc(41651, kSignature, 0, 0);
    }
    if (text_length != 0 && text_length <= 29) {
        if (packet.get_str(text, sizeof(text), text_length) != 1) {
            return LineFunc(41656, kSignature, 0, 0);
        }
        if (user->IsSubGuildMaster() != 1 && user->IsGuildMaster() != 1) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_REQ_GUILD_SECEDE, 36);
            return 0;
        }
        if (CPowerManager::IsPowerWarEventOn()) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_REQ_GUILD_SECEDE, 19);
            return 0;
        }
    }

    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(13));
    if (security_error != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQ_GUILD_SECEDE,
                                 static_cast<unsigned char>(security_error));
        return 0;
    }

    if (text_length == 0) {
        user->deleteGuildExpBook();
    }

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendRequestGuildSecede(guild_key,
                                  static_cast<unsigned int>(user->getCurCharacNo()),
                                  text_length,
                                  text);
    return 0;
}

// IDA_EA: 0x82339A2
// IDA_NAME: _ZN29Dispatcher_RequestGuildSecede4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestGuildSecede::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestGuildSecede::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82339AC
// IDA_NAME: _ZN29Dispatcher_RequestGuildSecede7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestGuildSecede::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestGuildSecede::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
