#include "old_game/gm/core/game_master_npc_shop_commands.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/shop/npc_limit_packets.h"
#include "old_game/economy/mall/secret_shop_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace game_master {

namespace {

// 0x84AACB8 and 0x84AADD8 share this exact two-integer parser.  In
// particular, only literal spaces are trimmed/split: the old command parser
// did not normalize tabs or validate a missing second token.
void ParseNpcAndValue(CParser* parser, int* npc_id, int* value) {
    std::string& command_tail = parser->command_tail();
    trim(command_tail);
    const std::string::size_type separator = command_tail.find(" ");
    *npc_id = std::atoi(command_tail.substr(0, separator).c_str());
    *value = std::atoi(command_tail.substr(separator + 1).c_str());
}

// IDA rodata `format__3` at 0x8C7FC78.  Keep the original CP949 byte
// sequence, rather than allowing the source-file encoding to change the
// client-visible chat message.
const char kSecretShopNpcChangedFormat[] =
    "\xBA\xF1\xB9\xD0\xBB\xF3\xC1\xA1 npc\xB0\xA1 %d\xB7\xCE "
    "\xB1\xB3\xC3\xBC\xB5\xC7\xBE\xFA\xBD\xC0\xB4\xCF\xB4\xD9";

}  // namespace

// IDA_EA: 0x82A8300
// IDA_NAME: _ZN11game_master19CChangeNPCMaxDayCmdC2Ev
// DEMANGLED: game_master::CChangeNPCMaxDayCmd::CChangeNPCMaxDayCmd(void)
CChangeNPCMaxDayCmd::CChangeNPCMaxDayCmd()
    : CCommand(), npc_id_(0), max_day_(0) {}

// IDA_EA: 0x84AACB8
// IDA_NAME: _ZN11game_master19CChangeNPCMaxDayCmd5parseEv
// DEMANGLED: game_master::CChangeNPCMaxDayCmd::parse(void)
void CChangeNPCMaxDayCmd::parse() {
    ParseNpcAndValue(parser_, &npc_id_, &max_day_);
}

// IDA_EA: 0x84AADD2
// IDA_NAME: _ZN11game_master19CChangeNPCMaxDayCmd7executeEv
// DEMANGLED: game_master::CChangeNPCMaxDayCmd::execute(void)
void CChangeNPCMaxDayCmd::execute() {
    // The original provider is intentionally empty after parsing its two
    // fields.  Preserve that behavior rather than inventing a state mutation.
}

// IDA_EA: 0x82A831C
// IDA_NAME: _ZN11game_master18CChangeNPCFavorCmdC2Ev
// DEMANGLED: game_master::CChangeNPCFavorCmd::CChangeNPCFavorCmd(void)
CChangeNPCFavorCmd::CChangeNPCFavorCmd()
    : CCommand(), npc_id_(0), favor_(0) {}

// IDA_EA: 0x84AADD8
// IDA_NAME: _ZN11game_master18CChangeNPCFavorCmd5parseEv
// DEMANGLED: game_master::CChangeNPCFavorCmd::parse(void)
void CChangeNPCFavorCmd::parse() {
    ParseNpcAndValue(parser_, &npc_id_, &favor_);
}

// IDA_EA: 0x84AAEF2
// IDA_NAME: _ZN11game_master18CChangeNPCFavorCmd7executeEv
// DEMANGLED: game_master::CChangeNPCFavorCmd::execute(void)
void CChangeNPCFavorCmd::execute() {
    // 0x84AAEF2 is intentionally empty in the original binary.
}

// IDA_EA: 0x82A868E
// IDA_NAME: _ZN11game_master16COneADayItemShopC2Ev
// DEMANGLED: game_master::COneADayItemShop::COneADayItemShop(void)
COneADayItemShop::COneADayItemShop() : CCommand() {}

// IDA_EA: 0x84B0610
// IDA_NAME: _ZN11game_master16COneADayItemShop7executeEv
// DEMANGLED: game_master::COneADayItemShop::execute(void)
void COneADayItemShop::execute() {
    G_GameWorld()->testResetOneADayItemShop();
}

// IDA_EA: 0x82A885A
// IDA_NAME: _ZN11game_master15NpcLimitBuyItemC2Ev
// DEMANGLED: game_master::NpcLimitBuyItem::NpcLimitBuyItem(void)
NpcLimitBuyItem::NpcLimitBuyItem() : CCommand() {}

// IDA_EA: 0x84B1F22
// IDA_NAME: _ZN11game_master15NpcLimitBuyItem5parseEv
// DEMANGLED: game_master::NpcLimitBuyItem::parse(void)
void NpcLimitBuyItem::parse() {
    // 0x84B1F22 is intentionally empty.
}

// IDA_EA: 0x84B1F28
// IDA_NAME: _ZN11game_master15NpcLimitBuyItem7executeEv
// DEMANGLED: game_master::NpcLimitBuyItem::execute(void)
void NpcLimitBuyItem::execute() {
    CUser* const user = GetUser();
    if (!user) {
        return;
    }

    LimitNpcBuyItemRequestInfo request;
    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendPacket(reinterpret_cast<char*>(&request),
                      static_cast<int>(sizeof(request)));
}

// IDA_EA: 0x82A8496
// IDA_NAME: _ZN11game_master14CSecretShopNpcC2Ev
// DEMANGLED: game_master::CSecretShopNpc::CSecretShopNpc(void)
CSecretShopNpc::CSecretShopNpc() : CCommand(), npc_id_(0) {}

// IDA_EA: 0x84AB9DA
// IDA_NAME: _ZN11game_master14CSecretShopNpc5parseEv
// DEMANGLED: game_master::CSecretShopNpc::parse(void)
void CSecretShopNpc::parse() {
    std::string& command_tail = parser_->command_tail();
    trim(command_tail);
    npc_id_ = std::atoi(command_tail.c_str());
}

// IDA_EA: 0x84ABA12
// IDA_NAME: _ZN11game_master14CSecretShopNpc7executeEv
// DEMANGLED: game_master::CSecretShopNpc::execute(void)
void CSecretShopNpc::execute() {
    CUser* const user = GetUser();
    if (user->get_state() != 5) {
        return;
    }

    CParty* const party = user->GetParty();
    secretshop::SECRET_SHOP_DATA* const secret_shop_data =
        user->GetSecretShopData();
    if (!secret_shop_data) {
        return;
    }

    if (npc_id_ != 1000 && npc_id_ != 1002 && npc_id_ != 1003 &&
        npc_id_ != 1004) {
        return;
    }

    // The original reads CParty +0xCAC, which is exactly
    // CParty::battle_field_ + CBattle_Field::dungeon_.  Do not allow NPC
    // swapping while that selected dungeon is a blood dungeon.
    CDungeon* const dungeon = party ? party->battle_field().GetDungeon() : 0;
    if (dungeon && dungeon->get_blood_dungeon_type() > 0) {
        return;
    }

    secret_shop_data->state = npc_id_;

    char message[256] = {};
    std::sprintf(message, kSecretShopNpcChangedFormat, secret_shop_data->state);

    cMyTrace trace("virtual void game_master::CSecretShopNpc::execute()",
                   1855,
                   0);
    trace("[SECRETSHOP] change npc(%s, %d)", user->getCurCharacName(),
          secret_shop_data->state);

    PacketGuard chat_packet;
    G_GameWorld()->make_packet_chat_msg(
        chat_packet,
        user,
        message,
        static_cast<int>(std::strlen(message)),
        2,
        static_cast<ENUM_NOTIPACKET>(12));

    PacketGuard npc_packet;
    npc_packet.put_header(0, ENUM_NOTIPACKET_SECRET_SHOP_NPC);
    npc_packet.put_int(secret_shop_data->state);
    npc_packet.finalize(true);

    if (party) {
        party->send_to_party(chat_packet);
        party->send_to_party(npc_packet);
    } else {
        user->Send(chat_packet);
        user->Send(npc_packet);
    }
}

// IDA_EA: 0x82A847A
// IDA_NAME: _ZN11game_master23CSecretShopCheckLotteryC2Ev
// DEMANGLED: game_master::CSecretShopCheckLottery::CSecretShopCheckLottery(void)
CSecretShopCheckLottery::CSecretShopCheckLottery() : CCommand() {}

// IDA_EA: 0x84AB9C4
// IDA_NAME: _ZN11game_master23CSecretShopCheckLottery7executeEv
// DEMANGLED: game_master::CSecretShopCheckLottery::execute(void)
void CSecretShopCheckLottery::execute() {
    GlobalData::s_secret_shop->CheckLottery();
}

}  // namespace game_master
