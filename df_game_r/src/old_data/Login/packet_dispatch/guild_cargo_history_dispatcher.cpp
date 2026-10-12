#include "old_data/Login/packet_dispatch/guild_cargo_history_dispatcher.h"

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "common/manager/user_history_log_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kGuildCargoHistoryProcessSignature[] =
    "virtual int Dispatcher_GuildCargoHistory::process(CUser*, MSG_BASE&, ParamBase&)";
const char kGuildCargoProcessSignature[] =
    "virtual int Dispatcher_GuildCargo::process(CUser*, MSG_BASE&, ParamBase&)";
const char kGuildCargoPushItemReadSignature[] =
    "virtual int Dispatcher_GuildCargoPushItem::read(PacketBuf&, MSG_BASE&)";
const char kGuildCargoPushItemProcessSignature[] =
    "virtual int Dispatcher_GuildCargoPushItem::process(CUser*, MSG_BASE&, ParamBase&)";
const char kGuildCargoPopItemReadSignature[] =
    "virtual int Dispatcher_GuildCargoPopItem::read(PacketBuf&, MSG_BASE&)";
const char kGuildCargoPopItemProcessSignature[] =
    "virtual int Dispatcher_GuildCargoPopItem::process(CUser*, MSG_BASE&, ParamBase&)";
const char kGuildCargoMoveItemReadSignature[] =
    "virtual int Dispatcher_GuildCargoMoveItem::read(PacketBuf&, MSG_BASE&)";
const char kGuildCargoMoveItemProcessSignature[] =
    "virtual int Dispatcher_GuildCargoMoveItem::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x8686544
// IDA_NAME: _ZN15cUserHistoryLog17GuildCargoHistoryE31ENUM_GUILD_CARGO_HISTORY_REASONi
// DEMANGLED: cUserHistoryLog::GuildCargoHistory(
//             ENUM_GUILD_CARGO_HISTORY_REASON,int)
int cUserHistoryLog::GuildCargoHistory(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int charac_guildkey) {
    return owner_->LogHistory("GuildCargo,%d,%d", static_cast<int>(reason),
                              charac_guildkey);
}

// IDA_EA: 0x86864E6
// IDA_NAME: _ZN15cUserHistoryLog10GuildCargoE31ENUM_GUILD_CARGO_HISTORY_REASONi
// DEMANGLED: cUserHistoryLog::GuildCargo(
//             ENUM_GUILD_CARGO_HISTORY_REASON,int)
int cUserHistoryLog::GuildCargo(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                                 int charac_guildkey) {
    return owner_->LogHistory("GuildCargo,%d,%d", static_cast<int>(reason),
                              charac_guildkey);
}

// IDA_EA: 0x8686512
// IDA_NAME: _ZN15cUserHistoryLog16GuildCargoResultE31ENUM_GUILD_CARGO_HISTORY_REASONii
// DEMANGLED: cUserHistoryLog::GuildCargoResult(ENUM_GUILD_CARGO_HISTORY_REASON,int,int)
int cUserHistoryLog::GuildCargoResult(ENUM_GUILD_CARGO_HISTORY_REASON reason,
                                      int result,
                                      int capacity) {
    return owner_->LogHistory("GuildCargo,%d,%d,%d", static_cast<int>(reason),
                              result, capacity);
}

// IDA_EA: 0x8686570
// IDA_NAME: _ZN15cUserHistoryLog23GuildCargoHistoryResultE31ENUM_GUILD_CARGO_HISTORY_REASONi
// DEMANGLED: cUserHistoryLog::GuildCargoHistoryResult(ENUM_GUILD_CARGO_HISTORY_REASON,int)
int cUserHistoryLog::GuildCargoHistoryResult(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int item_count) {
    return owner_->LogHistory("GuildCargo,%d,%d", static_cast<int>(reason),
                              item_count);
}

// IDA_EA: 0x868659C
// IDA_NAME: _ZN15cUserHistoryLog23GuildCargoCheckPushItemE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiii
int cUserHistoryLog::GuildCargoCheckPushItem(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int item_count,
    int item_id,
    int add_info,
    int item_space,
    int slot,
    int is_stackable) {
    return owner_->LogHistory("GuildCargo,%d,%d,%d,%d,%d,%d,%d",
                              static_cast<int>(reason), item_count, item_id,
                              add_info, item_space, slot, is_stackable);
}

// IDA_EA: 0x86865EA
// IDA_NAME: _ZN15cUserHistoryLog29GuildCargoCheckPushItemResultE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiii
// DEMANGLED: cUserHistoryLog::GuildCargoCheckPushItemResult(ENUM_GUILD_CARGO_HISTORY_REASON,int,int,int,int,int,int)
int cUserHistoryLog::GuildCargoCheckPushItemResult(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int result,
    int item_count,
    int item_id,
    int add_info,
    int item_space,
    int slot) {
    return owner_->LogHistory("GuildCargo,%d,%d,%d,%d,%d,%d,%d",
                              static_cast<int>(reason), result, item_count,
                              item_id, add_info, item_space, slot);
}

// IDA_EA: 0x8686638
// IDA_NAME: _ZN15cUserHistoryLog18GuildCargoPushItemE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiiih
// DEMANGLED: cUserHistoryLog::GuildCargoPushItem(ENUM_GUILD_CARGO_HISTORY_REASON,int,int,int,int,int,int,unsigned char)
int cUserHistoryLog::GuildCargoPushItem(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int item_count,
    int item_id,
    int add_info,
    int item_space,
    int slot,
    int is_stackable,
    unsigned char result) {
    return owner_->LogHistory("GuildCargo,%d,%d,%d,%d,%d,%d,%d,%d",
                              static_cast<int>(reason), item_count, item_id,
                              add_info, item_space, slot, is_stackable,
                              result);
}

// IDA_EA: 0x8686694
// IDA_NAME: _ZN15cUserHistoryLog22GuildCargoItemRecoveryE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiih
// DEMANGLED: cUserHistoryLog::GuildCargoItemRecovery(ENUM_GUILD_CARGO_HISTORY_REASON,int,int,int,int,int,unsigned char)
int cUserHistoryLog::GuildCargoItemRecovery(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int item_id,
    int add_info,
    int avatar_select,
    int extend_info,
    int item_attr,
    unsigned char upgrade_separate) {
    return owner_->LogHistory("GuildCargo,%d,%d,%d,%d,%d,%d,%d",
                              static_cast<int>(reason), item_id, add_info,
                              avatar_select, extend_info, item_attr,
                              upgrade_separate);
}

// IDA_EA: 0x86866EA
// IDA_NAME: _ZN15cUserHistoryLog17GuildCargoPopItemE31ENUM_GUILD_CARGO_HISTORY_REASONiiiii
int cUserHistoryLog::GuildCargoPopItem(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int cargo_slot,
    int item_id,
    int add_info,
    int is_stackable,
    int item_count) {
    return owner_->LogHistory("GuildCargo,%d,%d,%d,%d,%d,%d",
                              static_cast<int>(reason), cargo_slot, item_id,
                              add_info, is_stackable, item_count);
}

// IDA_EA: 0x8686732
// IDA_NAME: _ZN15cUserHistoryLog24GuildCargoPushItemResultE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiii
// DEMANGLED: cUserHistoryLog::GuildCargoPushItemResult(ENUM_GUILD_CARGO_HISTORY_REASON,int,int,int,int,int,int)
int cUserHistoryLog::GuildCargoPushItemResult(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int result,
    int cargo_slot,
    int item_id,
    int add_info,
    int item_space,
    int slot) {
    return owner_->LogHistory("GuildCargo,%d,%d,%d,%d,%d,%d,%d",
                              static_cast<int>(reason), result, cargo_slot,
                              item_id, add_info, item_space, slot);
}

// IDA_EA: 0x8686780
// IDA_NAME: _ZN15cUserHistoryLog23GuildCargoPopItemResultE31ENUM_GUILD_CARGO_HISTORY_REASONiiiiii
// DEMANGLED: cUserHistoryLog::GuildCargoPopItemResult(ENUM_GUILD_CARGO_HISTORY_REASON,int,int,int,int,int,int)
int cUserHistoryLog::GuildCargoPopItemResult(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int result,
    int cargo_slot,
    int item_id,
    int add_info,
    int avatar_select,
    int extend_info) {
    return owner_->LogHistory("GuildCargo,%d,%d,%d,%d,%d,%d,%d",
                              static_cast<int>(reason), result, cargo_slot,
                              item_id, add_info, avatar_select, extend_info);
}

// IDA_EA: 0x86867CE
// IDA_NAME: _ZN15cUserHistoryLog18GuildCargoMoveItemE31ENUM_GUILD_CARGO_HISTORY_REASONii
int cUserHistoryLog::GuildCargoMoveItem(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int from_slot,
    int to_slot) {
    return owner_->LogHistory("GuildCargo,%d,%d,%d", static_cast<int>(reason),
                              from_slot, to_slot);
}

// IDA_EA: 0x8686800
// IDA_NAME: _ZN15cUserHistoryLog24GuildCargoMoveItemResultE31ENUM_GUILD_CARGO_HISTORY_REASONiii
// DEMANGLED: cUserHistoryLog::GuildCargoMoveItemResult(ENUM_GUILD_CARGO_HISTORY_REASON,int,int,int)
int cUserHistoryLog::GuildCargoMoveItemResult(
    ENUM_GUILD_CARGO_HISTORY_REASON reason,
    int result,
    int from_slot,
    int to_slot) {
    return owner_->LogHistory("GuildCargo,%d,%d,%d,%d", static_cast<int>(reason),
                              result, from_slot, to_slot);
}

// IDA_EA: 0x822BE8A
// IDA_NAME: _ZN34Packet_Guild_Cargo_History_RequestC2Ev
// DEMANGLED: Packet_Guild_Cargo_History_Request::Packet_Guild_Cargo_History_Request(void)
Packet_Guild_Cargo_History_Request::Packet_Guild_Cargo_History_Request()
    : PacketHeader(0x070bu, 0x12u), charac_guildkey(0), charac_no(0) {}

// IDA_EA: 0x822BE52
// IDA_NAME: _ZN25Packet_Guild_Cargo_RequstC2Ev
// DEMANGLED: Packet_Guild_Cargo_Requst::Packet_Guild_Cargo_Requst(void)
Packet_Guild_Cargo_Requst::Packet_Guild_Cargo_Requst()
    : PacketHeader(0x070au, 0x12u), charac_guildkey(0), charac_no(0) {}

// IDA_EA: 0x822BEC2
// IDA_NAME: _ZN40Packet_Guild_Check_Guild_Cargo_Push_ItemC2Ev
Packet_Guild_Check_Guild_Cargo_Push_Item::
    Packet_Guild_Check_Guild_Cargo_Push_Item()
    : PacketHeader(0x070cu, 0x26u),
      charac_guildkey(0),
      charac_no(0),
      item_count(0),
      item_id(0),
      add_info(0),
      stackable_limit(0),
      is_stackable(0),
      item_space(0),
      slot(0) {}

// IDA_EA: 0x84E9434
// IDA_NAME: _ZN34Packet_Guild_Guild_Cargo_Push_ItemC2Ev
// DEMANGLED: Packet_Guild_Guild_Cargo_Push_Item::Packet_Guild_Guild_Cargo_Push_Item(void)
Packet_Guild_Guild_Cargo_Push_Item::Packet_Guild_Guild_Cargo_Push_Item()
    : PacketHeader(0x070du, 0x53u),
      charac_guildkey(0),
      charac_no(0),
      item_count(0),
      stackable_limit(0),
      is_stackable(0),
      item(),
      item_space(0),
      slot(0) {}

// IDA_EA: 0x822BF38
// IDA_NAME: _ZN33Packet_Guild_Guild_Cargo_Pop_ItemC2Ev
Packet_Guild_Guild_Cargo_Pop_Item::Packet_Guild_Guild_Cargo_Pop_Item()
    : PacketHeader(0x070eu, 0x23u),
      charac_guildkey(0),
      charac_no(0),
      cargo_slot(0),
      item_id(0),
      add_info(0),
      is_stackable(0),
      item_count(0) {}

// IDA_EA: 0x822BF9E
// IDA_NAME: _ZN34Packet_Guild_Guild_Cargo_Move_ItemC2Ev
Packet_Guild_Guild_Cargo_Move_Item::Packet_Guild_Guild_Cargo_Move_Item()
    : PacketHeader(0x070fu, 0x22u),
      charac_guildkey(0),
      charac_no(0),
      from_slot(0),
      to_slot(0),
      from_item_key(0),
      to_item_key(0) {}

// This source is the single concrete owner of the exact old dispatcher
// template/vtable family; root-table registration intentionally remains
// deferred with PacketDispatcher.
// IDA_EA: 0x8253A56
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stGuildCargoHistoryParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x8253BA6
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stGuildCargoHistoryParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x8253BB0
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stGuildCargoHistoryParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825CA8A / 0x825CABA
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stGuildCargoHistoryParamL8ch_state0EED2Ev /
//           _ZN17IPacketDispatcherI8MSG_BASE24stGuildCargoHistoryParamL8ch_state0EED0Ev
// IDA_EA: 0x825D8B8
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stGuildCargoHistoryParamL8ch_state0EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x859889A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stGuildCargoHistoryParamL8ch_state0EEC2Ev
template class IPacketDispatcher<MSG_BASE,
                                 stGuildCargoHistoryParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8596C78
// IDA_NAME: _ZN28Dispatcher_GuildCargoHistoryC2Ev
// DEMANGLED: Dispatcher_GuildCargoHistory::Dispatcher_GuildCargoHistory(void)
Dispatcher_GuildCargoHistory::Dispatcher_GuildCargoHistory()
    : IPacketDispatcher<MSG_BASE,
                        stGuildCargoHistoryParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824585C
// IDA_NAME: _ZN28Dispatcher_GuildCargoHistoryD2Ev
// DEMANGLED: Dispatcher_GuildCargoHistory::~Dispatcher_GuildCargoHistory()
// IDA_EA: 0x824588C
// IDA_NAME: _ZN28Dispatcher_GuildCargoHistoryD0Ev
// DEMANGLED: Dispatcher_GuildCargoHistory::~Dispatcher_GuildCargoHistory()
Dispatcher_GuildCargoHistory::~Dispatcher_GuildCargoHistory() {}

// IDA_EA: 0x81D0C32
// IDA_NAME: _ZN28Dispatcher_GuildCargoHistory4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GuildCargoHistory::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GuildCargoHistory::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81D0C3C
// IDA_NAME: _ZN28Dispatcher_GuildCargoHistory11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildCargoHistory::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildCargoHistory::check_error(CUser* user) const {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return -1;
    }
    if (!user->get_charac_guildkey()) {
        return 100;
    }
    return user->IsExistGuildAgit() ? 0 : 181;
}

// IDA_EA: 0x81D0CB4
// IDA_NAME: _ZN28Dispatcher_GuildCargoHistory7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildCargoHistory::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildCargoHistory::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)message;
    stGuildCargoHistoryParam& result =
        static_cast<stGuildCargoHistoryParam&>(param);
    result.result = check_error(user);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(11373, kGuildCargoHistoryProcessSignature,
                        result.result, 0);
    }

    Packet_Guild_Cargo_History_Request request;
    request.charac_guildkey =
        static_cast<uint32_t>(user->get_charac_guildkey());
    request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendTcpPacket(reinterpret_cast<char*>(&request), request.size);
    CUser_history_log(user)->GuildCargoHistory(
        static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(3),
        user->get_charac_guildkey());
    return 0;
}

// IDA_EA: 0x81D0DC6
// IDA_NAME: _ZN28Dispatcher_GuildCargoHistory4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_GuildCargoHistory::send(CUser *,ParamBase &)
void Dispatcher_GuildCargoHistory::send(CUser* user, ParamBase& param) {
    const stGuildCargoHistoryParam& result =
        static_cast<const stGuildCargoHistoryParam&>(param);
    if (result.result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO,
                                 static_cast<unsigned char>(result.result));
    }
}

// This source is also the single concrete owner of CMD265's distinct
// MSG_BASE/stGuildCargoParam state-0 template family.
// IDA_EA: 0x8253C42
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stGuildCargoParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x8253D92
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stGuildCargoParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x8253D9C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stGuildCargoParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825CAD8 / 0x825CB08
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stGuildCargoParamL8ch_state0EED2Ev /
//           _ZN17IPacketDispatcherI8MSG_BASE17stGuildCargoParamL8ch_state0EED0Ev
// IDA_EA: 0x825D8BE
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stGuildCargoParamL8ch_state0EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x859887E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stGuildCargoParamL8ch_state0EEC2Ev
template class IPacketDispatcher<MSG_BASE,
                                 stGuildCargoParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8596C5C
// IDA_NAME: _ZN21Dispatcher_GuildCargoC2Ev
// DEMANGLED: Dispatcher_GuildCargo::Dispatcher_GuildCargo(void)
Dispatcher_GuildCargo::Dispatcher_GuildCargo()
    : IPacketDispatcher<MSG_BASE, stGuildCargoParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82458AA
// IDA_NAME: _ZN21Dispatcher_GuildCargoD2Ev
// DEMANGLED: Dispatcher_GuildCargo::~Dispatcher_GuildCargo()
// IDA_EA: 0x82458DA
// IDA_NAME: _ZN21Dispatcher_GuildCargoD0Ev
// DEMANGLED: Dispatcher_GuildCargo::~Dispatcher_GuildCargo()
Dispatcher_GuildCargo::~Dispatcher_GuildCargo() {}

// IDA_EA: 0x81D0A60
// IDA_NAME: _ZN21Dispatcher_GuildCargo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GuildCargo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GuildCargo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81D0A6A
// IDA_NAME: _ZN21Dispatcher_GuildCargo11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildCargo::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildCargo::check_error(CUser* user) const {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return -1;
    }
    if (!user->get_charac_guildkey()) {
        return 100;
    }
    return user->IsExistGuildAgit() ? 0 : 181;
}

// IDA_EA: 0x81D0AE2
// IDA_NAME: _ZN21Dispatcher_GuildCargo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildCargo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildCargo::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)message;
    stGuildCargoParam& result = static_cast<stGuildCargoParam&>(param);
    result.result = check_error(user);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(11302, kGuildCargoProcessSignature, result.result,
                        static_cast<unsigned int>(user->get_acc_id()));
    }

    Packet_Guild_Cargo_Requst request;
    request.charac_guildkey =
        static_cast<uint32_t>(user->get_charac_guildkey());
    request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendTcpPacket(reinterpret_cast<char*>(&request), request.size);
    CUser_history_log(user)->GuildCargo(
        static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(0),
        user->get_charac_guildkey());
    return 0;
}

// IDA_EA: 0x81D0BFA
// IDA_NAME: _ZN21Dispatcher_GuildCargo4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_GuildCargo::send(CUser *,ParamBase &)
void Dispatcher_GuildCargo::send(CUser* user, ParamBase& param) {
    const stGuildCargoParam& result = static_cast<const stGuildCargoParam&>(param);
    if (result.result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_GUILD_CARGO,
                                 static_cast<unsigned char>(result.result));
    }
}

// CMD251 template owner.
// IDA_EA: 0x825386A / 0x82539BA / 0x82539C4
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_GUILD_CARGO_PUSH_ITEM26stGuildCargoPushItemParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf /
//           _ZN17IPacketDispatcherI25MSG_GUILD_CARGO_PUSH_ITEM26stGuildCargoPushItemParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf /
//           _ZN17IPacketDispatcherI25MSG_GUILD_CARGO_PUSH_ITEM26stGuildCargoPushItemParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825CA3C / 0x825CA6C / 0x825D8B2 / 0x85988B6
// IDA_NAME: ...D2Ev / ...D0Ev / ...4sendEP5CUserR9ParamBase / ...C2Ev
template class IPacketDispatcher<MSG_GUILD_CARGO_PUSH_ITEM,
                                 stGuildCargoPushItemParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8596C94
// IDA_NAME: _ZN29Dispatcher_GuildCargoPushItemC2Ev
// DEMANGLED: Dispatcher_GuildCargoPushItem::Dispatcher_GuildCargoPushItem(void)
Dispatcher_GuildCargoPushItem::Dispatcher_GuildCargoPushItem()
    : IPacketDispatcher<MSG_GUILD_CARGO_PUSH_ITEM,
                        stGuildCargoPushItemParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824580E
// IDA_NAME: _ZN29Dispatcher_GuildCargoPushItemD2Ev
// DEMANGLED: Dispatcher_GuildCargoPushItem::~Dispatcher_GuildCargoPushItem()
// IDA_EA: 0x824583E
// IDA_NAME: _ZN29Dispatcher_GuildCargoPushItemD0Ev
// DEMANGLED: Dispatcher_GuildCargoPushItem::~Dispatcher_GuildCargoPushItem()
Dispatcher_GuildCargoPushItem::~Dispatcher_GuildCargoPushItem() {}

// IDA_EA: 0x81D0DFE
// IDA_NAME: _ZN29Dispatcher_GuildCargoPushItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GuildCargoPushItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GuildCargoPushItem::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_GUILD_CARGO_PUSH_ITEM& request =
        static_cast<MSG_GUILD_CARGO_PUSH_ITEM&>(message);
    if (packet.get_byte(request.item_space) != 1)
        return LineFunc(11409, kGuildCargoPushItemReadSignature, 0, 0);
    if (packet.get_short(request.slot) != 1)
        return LineFunc(11410, kGuildCargoPushItemReadSignature, 0, 0);
    if (packet.get_int(request.item_id) != 1)
        return LineFunc(11411, kGuildCargoPushItemReadSignature, 0, 0);
    if (packet.get_int(request.add_info) != 1)
        return LineFunc(11412, kGuildCargoPushItemReadSignature, 0, 0);
    if (packet.get_short(request.item_count) != 1)
        return LineFunc(11413, kGuildCargoPushItemReadSignature, 0, 0);
    return 0;
}

// IDA_EA: 0x81D0F64
// IDA_NAME: _ZN29Dispatcher_GuildCargoPushItem11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildCargoPushItem::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildCargoPushItem::check_error(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)param;
    const MSG_GUILD_CARGO_PUSH_ITEM& request =
        static_cast<const MSG_GUILD_CARGO_PUSH_ITEM&>(message);
    if (user->get_state() != 3 || !user->getCurCharacR()) return -1;
    if (!user->get_charac_guildkey()) return 100;
    if (!user->IsExistGuildAgit()) return 181;

    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(request.item_space));
    const Inven_Item item = user->getCurCharacInvenR()->GetInvenSlot(
        inventory_type, request.slot);
    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(8));
    if (security_error) return security_error;
    if (user->CheckItemLock(inventory_type, request.slot)) return 213;
    if (!request.item_id || item.item_id_ != request.item_id) return 197;
    return G_CDataManager()->find_item(request.item_id) ? 0 : 197;
}

// IDA_EA: 0x81D10D4
// IDA_NAME: _ZN29Dispatcher_GuildCargoPushItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildCargoPushItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildCargoPushItem::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    const MSG_GUILD_CARGO_PUSH_ITEM& request =
        static_cast<const MSG_GUILD_CARGO_PUSH_ITEM&>(message);
    stGuildCargoPushItemParam& result =
        static_cast<stGuildCargoPushItemParam&>(param);
    result.result = check_error(user, message, param);
    if (result.result > 0) return 0;
    if (result.result < 0)
        return LineFunc(11504, kGuildCargoPushItemProcessSignature,
                        result.result, 0);

    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(request.item_space));
    const Inven_Item item = user->getCurCharacInvenR()->GetInvenSlot(
        inventory_type, request.slot);
    const CItem* const item_prototype = G_CDataManager()->find_item(item.item_id_);

    Packet_Guild_Check_Guild_Cargo_Push_Item service_request;
    service_request.charac_guildkey =
        static_cast<uint32_t>(user->get_charac_guildkey());
    service_request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    service_request.item_count = request.item_count;
    service_request.item_id = request.item_id;
    service_request.add_info = request.add_info;
    service_request.is_stackable = item_prototype->is_stackable() ? 1u : 0u;
    service_request.stackable_limit = service_request.is_stackable
        ? static_cast<const CStackableItem*>(item_prototype)->getStackableLimit()
        : 1;
    service_request.item_space = request.item_space;
    service_request.slot = request.slot;

    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendTcpPacket(reinterpret_cast<char*>(&service_request),
                               service_request.size);
    CUser_history_log(user)->GuildCargoCheckPushItem(
        static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(5), request.item_count,
        request.item_id, request.add_info, request.item_space, request.slot,
        service_request.is_stackable);
    return 0;
}

// IDA_EA: 0x81D1316
// IDA_NAME: _ZN29Dispatcher_GuildCargoPushItem4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_GuildCargoPushItem::send(CUser *,ParamBase &)
void Dispatcher_GuildCargoPushItem::send(CUser* user, ParamBase& param) {
    const stGuildCargoPushItemParam& result =
        static_cast<const stGuildCargoPushItemParam&>(param);
    if (!result.result) return;
    const ENUM_CMDPACKET command =
        result.result == 100 || result.result == 181
            ? ENUM_CMDPACKET_GUILD_CARGO
            : ENUM_CMDPACKET_GUILD_CARGO_PUSH_ITEM;
    user->SendCmdErrorPacket(command,
                             static_cast<unsigned char>(result.result));
}

// CMD252 template owner.
// IDA_EA: 0x825367E / 0x82537CE / 0x82537D8
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_GUILD_CARGO_POP_ITEM25stGuildCargoPopItemParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf /
//           _ZN17IPacketDispatcherI24MSG_GUILD_CARGO_POP_ITEM25stGuildCargoPopItemParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf /
//           _ZN17IPacketDispatcherI24MSG_GUILD_CARGO_POP_ITEM25stGuildCargoPopItemParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825C9EE / 0x825CA1E / 0x825D8AC / 0x85988D2
// IDA_NAME: ...D2Ev / ...D0Ev / ...4sendEP5CUserR9ParamBase / ...C2Ev
template class IPacketDispatcher<MSG_GUILD_CARGO_POP_ITEM,
                                 stGuildCargoPopItemParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8596CB0
// IDA_NAME: _ZN28Dispatcher_GuildCargoPopItemC2Ev
// DEMANGLED: Dispatcher_GuildCargoPopItem::Dispatcher_GuildCargoPopItem(void)
Dispatcher_GuildCargoPopItem::Dispatcher_GuildCargoPopItem()
    : IPacketDispatcher<MSG_GUILD_CARGO_POP_ITEM,
                        stGuildCargoPopItemParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82457C0
// IDA_NAME: _ZN28Dispatcher_GuildCargoPopItemD2Ev
// DEMANGLED: Dispatcher_GuildCargoPopItem::~Dispatcher_GuildCargoPopItem()
// IDA_EA: 0x82457F0
// IDA_NAME: _ZN28Dispatcher_GuildCargoPopItemD0Ev
// DEMANGLED: Dispatcher_GuildCargoPopItem::~Dispatcher_GuildCargoPopItem()
Dispatcher_GuildCargoPopItem::~Dispatcher_GuildCargoPopItem() {}

// IDA_EA: 0x81D1388
// IDA_NAME: _ZN28Dispatcher_GuildCargoPopItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GuildCargoPopItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GuildCargoPopItem::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_GUILD_CARGO_POP_ITEM& request =
        static_cast<MSG_GUILD_CARGO_POP_ITEM&>(message);
    if (packet.get_short(request.cargo_slot) != 1)
        return LineFunc(11579, kGuildCargoPopItemReadSignature, 0, 0);
    if (packet.get_int(request.item_id) != 1)
        return LineFunc(11580, kGuildCargoPopItemReadSignature, 0, 0);
    if (packet.get_int(request.add_info) != 1)
        return LineFunc(11581, kGuildCargoPopItemReadSignature, 0, 0);
    if (packet.get_byte(request.item_count) != 1)
        return LineFunc(11582, kGuildCargoPopItemReadSignature, 0, 0);
    return 0;
}

// IDA_EA: 0x81D14AA
// IDA_NAME: _ZN28Dispatcher_GuildCargoPopItem11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildCargoPopItem::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildCargoPopItem::check_error(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)param;
    const MSG_GUILD_CARGO_POP_ITEM& request =
        static_cast<const MSG_GUILD_CARGO_POP_ITEM&>(message);
    if (user->get_state() != 3 || !user->getCurCharacR()) return -1;
    if (!user->get_charac_guildkey()) return 100;
    if (!user->IsExistGuildAgit()) return 181;
    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(9));
    if (security_error) return security_error;
    if (!user->getCurCharacInvenR()->possible_to_insert(request.item_id))
        return 203;
    return G_CDataManager()->find_item(request.item_id) ? 0 : 197;
}

// IDA_EA: 0x81D15B2
// IDA_NAME: _ZN28Dispatcher_GuildCargoPopItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildCargoPopItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildCargoPopItem::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    const MSG_GUILD_CARGO_POP_ITEM& request =
        static_cast<const MSG_GUILD_CARGO_POP_ITEM&>(message);
    stGuildCargoPopItemParam& result =
        static_cast<stGuildCargoPopItemParam&>(param);
    result.result = check_error(user, message, param);
    if (result.result > 0) return 0;
    if (result.result < 0)
        return LineFunc(11663, kGuildCargoPopItemProcessSignature,
                        result.result, 0);

    const CItem* const item_prototype = G_CDataManager()->find_item(request.item_id);
    Packet_Guild_Guild_Cargo_Pop_Item service_request;
    service_request.charac_guildkey =
        static_cast<uint32_t>(user->get_charac_guildkey());
    service_request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    service_request.cargo_slot = request.cargo_slot;
    service_request.item_id = request.item_id;
    service_request.add_info = request.add_info;
    service_request.is_stackable = item_prototype->is_stackable() ? 1u : 0u;
    service_request.item_count = request.item_count;

    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendTcpPacket(reinterpret_cast<char*>(&service_request),
                               service_request.size);
    CUser_history_log(user)->GuildCargoPopItem(
        static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(10), request.cargo_slot,
        request.item_id, request.add_info, service_request.is_stackable,
        request.item_count);
    return 0;
}

// IDA_EA: 0x81D1748
// IDA_NAME: _ZN28Dispatcher_GuildCargoPopItem4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_GuildCargoPopItem::send(CUser *,ParamBase &)
void Dispatcher_GuildCargoPopItem::send(CUser* user, ParamBase& param) {
    const stGuildCargoPopItemParam& result =
        static_cast<const stGuildCargoPopItemParam&>(param);
    if (!result.result) return;
    const ENUM_CMDPACKET command =
        result.result == 100 || result.result == 181
            ? ENUM_CMDPACKET_GUILD_CARGO
            : ENUM_CMDPACKET_GUILD_CARGO_POP_ITEM;
    user->SendCmdErrorPacket(command,
                             static_cast<unsigned char>(result.result));
}

// CMD253 template owner.
// IDA_EA: 0x8253492 / 0x82535E2 / 0x82535EC
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_GUILD_CARGO_MOVE_ITEM26stGuildCargoMoveItemParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf /
//           _ZN17IPacketDispatcherI25MSG_GUILD_CARGO_MOVE_ITEM26stGuildCargoMoveItemParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf /
//           _ZN17IPacketDispatcherI25MSG_GUILD_CARGO_MOVE_ITEM26stGuildCargoMoveItemParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825C9A0 / 0x825C9D0 / 0x825D8A6 / 0x85988EE
// IDA_NAME: ...D2Ev / ...D0Ev / ...4sendEP5CUserR9ParamBase / ...C2Ev
template class IPacketDispatcher<MSG_GUILD_CARGO_MOVE_ITEM,
                                 stGuildCargoMoveItemParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8596CCC
// IDA_NAME: _ZN29Dispatcher_GuildCargoMoveItemC2Ev
// DEMANGLED: Dispatcher_GuildCargoMoveItem::Dispatcher_GuildCargoMoveItem(void)
Dispatcher_GuildCargoMoveItem::Dispatcher_GuildCargoMoveItem()
    : IPacketDispatcher<MSG_GUILD_CARGO_MOVE_ITEM,
                        stGuildCargoMoveItemParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8245772
// IDA_NAME: _ZN29Dispatcher_GuildCargoMoveItemD2Ev
// DEMANGLED: Dispatcher_GuildCargoMoveItem::~Dispatcher_GuildCargoMoveItem()
// IDA_EA: 0x82457A2
// IDA_NAME: _ZN29Dispatcher_GuildCargoMoveItemD0Ev
// DEMANGLED: Dispatcher_GuildCargoMoveItem::~Dispatcher_GuildCargoMoveItem()
Dispatcher_GuildCargoMoveItem::~Dispatcher_GuildCargoMoveItem() {}

// IDA_EA: 0x81D17BA
// IDA_NAME: _ZN29Dispatcher_GuildCargoMoveItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GuildCargoMoveItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GuildCargoMoveItem::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_GUILD_CARGO_MOVE_ITEM& request =
        static_cast<MSG_GUILD_CARGO_MOVE_ITEM&>(message);
    if (packet.get_short(request.from_slot) != 1)
        return LineFunc(11730, kGuildCargoMoveItemReadSignature, 0, 0);
    if (packet.get_int(request.from_item_key) != 1)
        return LineFunc(11732, kGuildCargoMoveItemReadSignature, 0, 0);
    if (packet.get_short(request.to_slot) != 1)
        return LineFunc(11734, kGuildCargoMoveItemReadSignature, 0, 0);
    if (packet.get_int(request.to_item_key) != 1)
        return LineFunc(11736, kGuildCargoMoveItemReadSignature, 0, 0);
    return 0;
}

// IDA_EA: 0x81D18DC
// IDA_NAME: _ZN29Dispatcher_GuildCargoMoveItem11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildCargoMoveItem::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildCargoMoveItem::check_error(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)message;
    (void)param;
    if (user->get_state() != 3 || !user->getCurCharacR()) return -1;
    if (!user->get_charac_guildkey()) return 100;
    return user->IsExistGuildAgit() ? 0 : 181;
}

// IDA_EA: 0x81D1954
// IDA_NAME: _ZN29Dispatcher_GuildCargoMoveItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GuildCargoMoveItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GuildCargoMoveItem::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    const MSG_GUILD_CARGO_MOVE_ITEM& request =
        static_cast<const MSG_GUILD_CARGO_MOVE_ITEM&>(message);
    stGuildCargoMoveItemParam& result =
        static_cast<stGuildCargoMoveItemParam&>(param);
    result.result = check_error(user, message, param);
    if (result.result > 0) return 0;
    if (result.result < 0)
        return LineFunc(11775, kGuildCargoMoveItemProcessSignature,
                        result.result, 0);

    Packet_Guild_Guild_Cargo_Move_Item service_request;
    service_request.charac_guildkey =
        static_cast<uint32_t>(user->get_charac_guildkey());
    service_request.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    service_request.from_slot = request.from_slot;
    service_request.to_slot = request.to_slot;
    service_request.from_item_key = request.from_item_key;
    service_request.to_item_key = request.to_item_key;

    CGuildServerProxy* const guild_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    guild_proxy->SendTcpPacket(reinterpret_cast<char*>(&service_request),
                               service_request.size);
    CUser_history_log(user)->GuildCargoMoveItem(
        static_cast<ENUM_GUILD_CARGO_HISTORY_REASON>(13), request.from_slot,
        request.to_slot);
    return 0;
}

// IDA_EA: 0x81D1A96
// IDA_NAME: _ZN29Dispatcher_GuildCargoMoveItem4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_GuildCargoMoveItem::send(CUser *,ParamBase &)
void Dispatcher_GuildCargoMoveItem::send(CUser* user, ParamBase& param) {
    const stGuildCargoMoveItemParam& result =
        static_cast<const stGuildCargoMoveItemParam&>(param);
    if (!result.result) return;
    const ENUM_CMDPACKET command =
        result.result == 100 || result.result == 181
            ? ENUM_CMDPACKET_GUILD_CARGO
            : ENUM_CMDPACKET_GUILD_CARGO_MOVE_ITEM;
    user->SendCmdErrorPacket(command,
                             static_cast<unsigned char>(result.result));
}
