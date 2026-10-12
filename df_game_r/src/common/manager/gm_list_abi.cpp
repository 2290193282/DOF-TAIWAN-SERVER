#include "common/manager/gm_list_abi.h"

#include "core/config/env/environment.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/gm_accounts_abi.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/PacketHeader.h"

#include <stdint.h>

#include <string>

namespace {

// IDA unit: 99_misc_global/Packet_Sync_GM_List.
// IDA_EA: C2 @0x82A40AC.  The old C2 sets the 10-byte PacketHeader to
// (0x1F4D, 0x70), clears sequence/count, then clears 20 account ids and 20
// GM levels.
#pragma pack(push, 1)
struct Packet_Sync_GM_List : PacketHeader {
    Packet_Sync_GM_List()
        : PacketHeader(0x1F4D, 0x70),
          sequence(0),
          count(0),
          account_ids(),
          gm_levels() {}

    uint8_t sequence;          // +0x0A
    uint8_t count;             // +0x0B
    uint32_t account_ids[20];  // +0x0C
    uint8_t gm_levels[20];     // +0x5C
};
#pragma pack(pop)

static_assert(sizeof(Packet_Sync_GM_List) == 0x70u,
              "Packet_Sync_GM_List IDA ABI size must stay 0x70");

void SendPacketToUpperServer(Packet_Sync_GM_List& packet,
                             int upper_server,
                             int server_group) {
    if (upper_server == 1) {
        CGuildServerProxy* const proxy =
            GlobalData::s_guild_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(server_group));
        proxy->SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
        return;
    }

    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(server_group));
    proxy->SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
}

}  // namespace

// IDA_EA: 0x8299912
// IDA_NAME: _ZN7Gm_ListC2Ev
// DEMANGLED: Gm_List::Gm_List(void)
Gm_List::Gm_List()
    : db_handle_(GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_LOG_REF,
                                                   SERVER_GROUP_DEFAULT)) {
    LoadGmListFromDb();
}

// IDA_EA: 0x829996C
// IDA_NAME: _ZN7Gm_List16LoadGmListFromDbEv
// DEMANGLED: Gm_List::LoadGmListFromDb(void)
int Gm_List::LoadGmListFromDb() {
    int result = 0;
    unsigned int account_id = 0;
    int gm_level = 0;

    db_handle_->set_query("seLect m_id, level from gm_manifest");
    result = db_handle_->exec(true);
    if (result != 1) {
        cMyTrace trace("bool Gm_List::LoadGmListFromDb()", 5588, 9, true,
                       true);
        trace("Failed to get GM list from DB.");
        return 0;
    }

    GlobalData::s_pGMAccounts_->clearGmList();
    std::string gm_account_ids;
    const int row_count = db_handle_->get_n_rows();
    for (int row = 0; row < row_count; ++row) {
        db_handle_->fetch();
        result = db_handle_->get_int(0, account_id);
        if (result != 1) {
            cMyTrace trace("bool Gm_List::LoadGmListFromDb()", 5605, 9,
                           true, true);
            trace("Failed to get GM's m_id from DB.");
            return 0;
        }

        gm_account_ids += NumberToString(account_id, 0);
        gm_account_ids += ", ";

        cMyTrace account_trace("bool Gm_List::LoadGmListFromDb()", 5612,
                               0);
        account_trace("GM M_ID : %s", NumberToString(account_id, 0));

        result = db_handle_->get_int(1, gm_level);
        if (result != 1) {
            cMyTrace trace("bool Gm_List::LoadGmListFromDb()", 5618, 9,
                           true, true);
            trace("Failed to get GM's lv from DB.");
            return 0;
        }

        GlobalData::s_pGMAccounts_->LoadGmList(account_id, gm_level);
    }

    cMyTrace list_trace("bool Gm_List::LoadGmListFromDb()", 5625, 9, true,
                        true);
    list_trace("\t- GM m_id: %s", gm_account_ids.c_str());
    if (G_CEnvironment()->get_channel_no() == 1) {
        SendGMList_To_UpperServer(0);
    }
    return result;
}

// IDA_EA: 0x8299D54
// IDA_NAME: _ZN7Gm_List25SendGMList_To_UpperServerEi
// DEMANGLED: Gm_List::SendGMList_To_UpperServer(int)
void Gm_List::SendGMList_To_UpperServer(int upper_server) {
    if (upper_server > 1) {
        return;
    }

    const int server_group = G_CEnvironment()->get_server_group();
    Packet_Sync_GM_List packet;
    const std::list<WongWork::CGMAccounts::stGMInfo_t>& gm_list =
        GlobalData::s_pGMAccounts_->GetGMList();
    int index = 0;
    for (std::list<WongWork::CGMAccounts::stGMInfo_t>::const_iterator it =
             gm_list.begin();
         it != gm_list.end();
         ++it) {
        packet.account_ids[index] = it->account_id;
        packet.gm_levels[index] = static_cast<uint8_t>(it->gm_level);
        ++index;
        if (index == 20) {
            packet.count = static_cast<uint8_t>(index);
            SendPacketToUpperServer(packet, upper_server, server_group);
            ++packet.sequence;
            index = 0;
        }
    }

    if (index) {
        packet.count = static_cast<uint8_t>(index);
        SendPacketToUpperServer(packet, upper_server, server_group);
    }
}
