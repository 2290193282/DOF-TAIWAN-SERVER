#include "old_game/player/session/inter_change_charac_name_return_dispatcher.h"

#include <string>

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/db/character/db_req_change_charac_name.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

void SendRenameEventMail(CUser* user,
                         unsigned int charac_no,
                         int item_id,
                         int item_count) {
    Inven_Item item;
    item.item_id_ = item_id;
    item.item_category_ = 2;
    item.add_info_ = item_count;
    item.avatar_select_ = 0;
    item.ResetItemAttr();

    const char* const title =
        g_scriptStringManager_.findString(4, "game_server_msg_01", 0);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        charac_no,
        0,
        0,
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
}

}  // namespace

// IDA_EA: 0x84D1C8A
// IDA_NAME: _ZN25Inter_ReqChangeCharacName12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_ReqChangeCharacName::dispatch_sig(CUser *,char *,int)
int Inter_ReqChangeCharacName::dispatch_sig(CUser* user, char* payload, int) {
    if (!user || !payload || user->get_state() <= 1) {
        return 0;
    }

    const SIG_CHANGE_CHARAC_NAME& reply =
        *reinterpret_cast<const SIG_CHANGE_CHARAC_NAME*>(payload);
    const int charac_no = static_cast<int>(reply.charac_no);
    if (reply.status.result_code != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
            static_cast<unsigned char>(reply.status.result_code));
        user->SetCharacViewStateByCharacNo(charac_no, 255);
        user->UpdateCharacView();
        return 0;
    }

    if (!user->setCharacName(charac_no, reply.charac_name)) {
        return 12718;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_CHANGE_CHARAC_NAME);
    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);

    user->SetCharacViewStateByCharacNo(charac_no, 1);
    const std::string charac_name(reply.charac_name);
    user->UpdateCharacNameInMercenary(static_cast<unsigned int>(charac_no),
                                      charac_name);
    user->UpdateCharacView();

    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());
    const unsigned int guild_id = reply.server_or_guild.result_guild_id;
    const unsigned int server_group =
        static_cast<unsigned int>(user->GetServerGroup());

    // 0x84D1C8A sends these two notification pairs twice.  Do not merge them:
    // the old guild/monitor endpoints observe two separate change records.
    if (guild_id != 0) {
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(server_group))
            ->SendChangeCharName(account_id,
                                 static_cast<unsigned int>(charac_no),
                                 guild_id,
                                 const_cast<char*>(reply.charac_name));
    }
    GlobalData::s_monitor_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(server_group))
        ->SendChangeCharName(account_id,
                             static_cast<unsigned int>(charac_no),
                             guild_id,
                             const_cast<char*>(reply.charac_name));
    if (guild_id != 0) {
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(server_group))
            ->SendChangeCharName(account_id,
                                 static_cast<unsigned int>(charac_no),
                                 guild_id,
                                 const_cast<char*>(reply.charac_name));
    }
    GlobalData::s_monitor_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(server_group))
        ->SendChangeCharName(account_id,
                             static_cast<unsigned int>(charac_no),
                             guild_id,
                             const_cast<char*>(reply.charac_name));

    if (reply.guild_context.result_request_flag == 0) {
        if (GlobalData::s_systemTime_.getCurSec() > 1179262800) {
            return 0;
        }

        const int level = user->getCharacLevelByCharacNo(charac_no);
        if (level <= 0) {
            return 12727;
        }
        if (level > 49) {
            SendRenameEventMail(user,
                                static_cast<unsigned int>(charac_no),
                                36,
                                5);
        }
        if (level > 39) {
            SendRenameEventMail(user,
                                static_cast<unsigned int>(charac_no),
                                16,
                                1);
        }
        SendRenameEventMail(user,
                            static_cast<unsigned int>(charac_no),
                            1,
                            10);
    }

    GlobalData::s_event_manager->changeCharacName(
        user,
        charac_no,
        const_cast<char*>(reply.charac_name),
        reply.guild_context.result_request_flag);
    return 0;
}

// IDA_EA: 0x84ED888 / 0x84F9606 / 0x84F9636
Inter_ReqChangeCharacName::Inter_ReqChangeCharacName()
    : ISigDispatcher() {}
Inter_ReqChangeCharacName::~Inter_ReqChangeCharacName() {}
