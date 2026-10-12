#include "old_game/world/region/game_world.h"

#include "common/config.h"
#include "old_game/world/region/world_area_types.h"
#include "old_data/channel/channel_db_payloads.h"
#include "old_data/channel/channel_script_t.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/user.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/log/format/trace.h"

#include <map>
#include <stdint.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>


namespace {

typedef std::map<int, int> OccupationCounts;

Village* VillageAt(GameWorld* world, int village_index) {
    if (village_index < 0 || village_index >= MAX_VILLAGE_NUM) {
        return 0;
    }
    return world->villages_ ? &world->villages_[village_index] : 0;
}

int BeginBasicInfoPacket(PacketGuard& packet) {
    packet.clear();
    packet.put_header(0, 2);
    packet.put_byte(0);
    const int count_index = packet.get_index();
    packet.put_short(0);
    return count_index;
}

}  // namespace


// IDA_EA: 0x86C46BA
// IDA_NAME: _ZN9GameWorld15get_dispositionEiiPN11pvp_assault12CDispositionE
// DEMANGLED: GameWorld::get_disposition(int,int,pvp_assault::CDisposition *)
int GameWorld::get_disposition(int village_index,
                               int area_index,
                               pvp_assault::CDisposition* disposition) {
    if (!check_valid_area(village_index, area_index)) {
        return 0;
    }

    Village* village = VillageAt(this, village_index);
    Area* area = village ? village->getArea(area_index) : 0;
    return area ? disposition->SetVirtualMap(&area->map_areas) : 0;
}

// IDA_EA: 0x86C4B0A
// IDA_NAME: _ZN9GameWorld14make_unique_idEv
// DEMANGLED: GameWorld::make_unique_id(void)
int GameWorld::make_unique_id() {
    do {
        ++next_unique_id_;
        if (next_unique_id_ == 0xffffu) {
            next_unique_id_ = 0;
        }
    } while (users_in_world_.find(next_unique_id_) != users_in_world_.end());

    return next_unique_id_;
}

// IDA_EA: 0x86C4C24
// IDA_NAME: _ZN9GameWorld12find_sessionEj
// DEMANGLED: GameWorld::find_session(unsigned int)
uint16_t GameWorld::find_session(unsigned int account_id) {
    GameWorldSessionMap::const_iterator found =
        sessions_by_account_.find(account_id);
    return found == sessions_by_account_.end() ? 0 : found->second;
}

// IDA_EA: 0x86C4CAA
// IDA_NAME: _ZN9GameWorld33find_user_from_world_byaccid_loopEj
// DEMANGLED: GameWorld::find_user_from_world_byaccid_loop(unsigned int)
CUser* GameWorld::find_user_from_world_byaccid_loop(unsigned int account_id) {
    for (GameWorldUserMap::const_iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        if (it->second &&
            static_cast<unsigned int>(it->second->get_acc_id()) == account_id) {
            return it->second;
        }
    }
    return 0;
}

// IDA_EA: 0x86C4D40
// IDA_NAME: _ZN9GameWorld28find_user_from_world_byaccidEj
// DEMANGLED: GameWorld::find_user_from_world_byaccid(unsigned int)
CUser* GameWorld::find_user_from_world_byaccid(unsigned int account_id) {
    const uint16_t user_uid = find_session(account_id);
    return find_from_world(user_uid);
}

// IDA_EA: 0x86C4E0C
// IDA_NAME: _ZN9GameWorld14CheckUserCountEi
// DEMANGLED: GameWorld::CheckUserCount(int)
bool GameWorld::CheckUserCount(int additional_user_count) {
    return static_cast<int>(users_in_world_.size()) + additional_user_count <=
        G_CEnvironment()->get_max_user_num();
}

// IDA_EA: 0x86C4E50
// IDA_NAME: _ZN9GameWorld16reach_game_worldEP5CUser
// DEMANGLED: GameWorld::reach_game_world(CUser *)
int GameWorld::reach_game_world(CUser* user) {
    if (!user) {
        return 0;
    }

    const uint16_t unique_id = static_cast<uint16_t>(user->get_unique_id());
    if (users_in_world_.find(unique_id) != users_in_world_.end()) {
        return 0;
    }

    const uint32_t account_id = static_cast<uint32_t>(user->get_acc_id());
    if (sessions_by_account_.find(account_id) != sessions_by_account_.end() ||
        !CheckUserCount(1)) {
        return 0;
    }

    users_in_world_.insert(std::make_pair(unique_id, user));
    sessions_by_account_[account_id] = unique_id;

    Village* village = getUserVillage(user);
    if (village) {
        user->set_area(village->get_gate_area());
    }

    user->set_state(3);
    EraseLoginUser(user);
    send_AllBasicInfo(user);

    private_store::CPrivateStoreMgr* store_mgr =
        private_store::GetInstancePrivateStoreMgr();
    if (user->IsPermissionPrivateStore()) {
        store_mgr->Enter(user);
    } else {
        store_mgr->LoadPrivateStore(user);
    }

    if (user->isHangameUser()) {
        ++hangame_user_count_;
    } else {
        ++other_user_count_;
    }
    return 1;
}

// IDA_EA: 0x86C83FC
// IDA_NAME: _ZN9GameWorld17send_AllBasicInfoEP5CUser
// DEMANGLED: GameWorld::send_AllBasicInfo(CUser *)
void GameWorld::send_AllBasicInfo(CUser* user) {
    PacketGuard packet;
    int basic_info_header = BeginBasicInfoPacket(packet);
    int visible_count = 0;
    for (GameWorldUserMap::const_iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* world_user = it->second;
        if (!world_user) {
            cMyTrace trace("void GameWorld::send_AllBasicInfo(CUser*)",
                           3062,
                           5);
            trace("[%s][%d]",
                  "void GameWorld::send_AllBasicInfo(CUser*)",
                  3062);
            continue;
        }
        if (!world_user->IsCurCharacVisible()) {
            continue;
        }

        // A basic-info record is indivisible on the wire.  If appending it
        // would overflow the current packet, flush the previous records and
        // retry this same user in a fresh packet; otherwise the user would
        // disappear from the enter-world snapshot.
        bool retried_after_flush = false;
        for (;;) {
            const int record_start = packet.get_index();
            if (world_user->make_basic_info(packet, 0) != 1) {
                cMyTrace trace("void GameWorld::send_AllBasicInfo(CUser*)",
                               3043,
                               5);
                trace("[%s][%d]",
                      "void GameWorld::send_AllBasicInfo(CUser*)",
                      3043);
            }
            if (!packet.getLastError()) {
                ++visible_count;
                break;
            }

            // A single malformed/oversized record must not spin forever.  It
            // cannot be represented in one protocol packet, so leave it out
            // after the one normal split retry and continue with later users.
            if (retried_after_flush) {
                packet.set_index(record_start);
                cMyTrace trace("void GameWorld::send_AllBasicInfo(CUser*)",
                               3044,
                               5);
                trace("[%s][%d]",
                      "void GameWorld::send_AllBasicInfo(CUser*)",
                      3044);
                break;
            }

            packet.set_index(record_start);
            packet.put_short(basic_info_header, visible_count);
            packet.finalize(true);
            user->Send(packet);
            basic_info_header = BeginBasicInfoPacket(packet);
            visible_count = 0;
            retried_after_flush = true;
        }
    }

    packet.put_short(basic_info_header, visible_count);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x86CB17C
// IDA_NAME: _ZN9GameWorld30updateDetailChannelServiceInfoEv
// DEMANGLED: GameWorld::updateDetailChannelServiceInfo(void)
int GameWorld::updateDetailChannelServiceInfo() {
    OccupationCounts counts;
    for (GameWorldUserMap::const_iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* user = it->second;
        if (!user) {
            continue;
        }

        switch (user->get_state()) {
        case 3:
            ++counts[0];
            break;
        case 5:
            ++counts[1];
            break;
        case 6:
        case 7:
            ++counts[(GetChannelType() == 15 || GetChannelType() == 16) ? 9
                                                                          : 2];
            break;
        case 8:
            ++counts[6];
            break;
        case 10:
            ++counts[8];
            break;
        default:
            break;
        }
    }

    const int private_store_count =
        private_store::GetInstancePrivateStoreMgr()->GetPrivateStoreSize();
    if (private_store_count > 0) {
        counts[0] -= private_store_count;
        counts[7] = private_store_count;
    }

    for (OccupationCounts::const_iterator it = counts.begin();
         it != counts.end(); ++it) {
        struct DetailChannelServiceInfo {
            int occupation;
            int reserved;
            int game_channel_no;
            int user_count;
        } payload;
        payload.occupation = it->first;
        payload.reserved = 0;
        payload.game_channel_no = G_CEnvironment()->get_gc_no();
        payload.user_count = it->second;

        CStreamGuard guard(
            GlobalData::stream_pool()->Acquire("world.cpp", 4367), true);
        // LOGDB uses the same DBTask route header: signal and requester are
        // both 32-bit for signal 106.
        *guard << 106;
        *guard << -1;
        guard->put_binary(&payload, sizeof(payload));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
    }
    return 0;
}

// IDA_EA: 0x86CB61A
// IDA_NAME: _ZN9GameWorld19GetChannelUserCountE17ENUM_SERVER_GROUPR19stOccStatisticByAge
// DEMANGLED: GameWorld::GetChannelUserCount(ENUM_SERVER_GROUP,stOccStatisticByAge &)
int GameWorld::GetChannelUserCount(ENUM_SERVER_GROUP server_group,
                                   stOccStatisticByAge& count_by_age) {
    int user_count = 0;
    for (GameWorldUserMap::const_iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* user = it->second;
        if (!user || user->GetServerGroup() != server_group) {
            continue;
        }

        ++user_count;
        const unsigned int age = static_cast<unsigned int>(user->GetAge());
        if (age <= 99u) {
            ++count_by_age.count_by_age[age ? age - 1u : 0u];
        } else {
            ++count_by_age.count_by_age[99];
        }
    }
    return user_count;
}

// IDA_EA: 0x86CB72A
// IDA_NAME: _ZN9GameWorld17UpdateChannelUserEv
// DEMANGLED: GameWorld::UpdateChannelUser(void)
void GameWorld::UpdateChannelUser() {
    SIG_UPDATE_CHANNEL payload;
    memset(&payload, 0, sizeof(payload));
    payload.gc_no = G_CEnvironment()->get_gc_no();
    payload.gc_now = static_cast<int16_t>(
        users_in_world_.size());

    payload.swordman_count =
        occupation_counts_[oldsrv::config::kCharacterJobSwordman];
    payload.fighter_count =
        occupation_counts_[oldsrv::config::kCharacterJobFighter];
    payload.gunner_count =
        occupation_counts_[oldsrv::config::kCharacterJobGunner];
    payload.mage_count =
        occupation_counts_[oldsrv::config::kCharacterJobMage];
    payload.priest_count =
        occupation_counts_[oldsrv::config::kCharacterJobPriest];
    payload.at_gunner_count =
        occupation_counts_[oldsrv::config::kCharacterJobAtGunner];
    payload.thief_count =
        occupation_counts_[oldsrv::config::kCharacterJobThief];
    payload.hangame_count = hangame_user_count_;
    payload.nexon_count = other_user_count_;

    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("world.cpp", 4422), true);
    // DBTask::DBProcess extracts the route and requester as two 32-bit
    // integers.  The original 0x86CB72A call sequence invokes
    // CStreamGuard::operator<<(int) for both values; using the char
    // overload shifts SIG_UPDATE_CHANNEL over the payload bytes.
    *guard << 46;
    *guard << -1;
    guard->put_binary(&payload, sizeof(payload));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x86CB88A
// IDA_NAME: _ZN9GameWorld13InsertChannelEi17ENUM_SERVER_GROUP
// DEMANGLED: GameWorld::InsertChannel(int,ENUM_SERVER_GROUP)
void GameWorld::InsertChannel(int game_channel_no,
                              ENUM_SERVER_GROUP server_group) {
    SIG_INSERT_CHANNEL payload;
    memset(&payload, 0, sizeof(payload));

    CEnvironment* environment = G_CEnvironment();
    payload.gc_no = game_channel_no;
    strncpy(payload.gc_ip, environment->get_server_ip(),
            sizeof(payload.gc_ip));
    // Original 0x86CB88A stores CEnvironment::get_tcp_port() in gc_port.
    // This is the client-facing game port advertised by game_channel, not
    // the companion UDP port.
    payload.gc_port = static_cast<uint16_t>(environment->get_tcp_port());
    payload.gc_max = static_cast<int16_t>(environment->get_max_user_num());
    // InsertChannel @ 0x86CB88A writes CEnvironment+888 into gc_game.
    // Despite the legacy wire-field name, this is the server-group number.
    payload.gc_game = static_cast<char>(environment->get_server_group());
    // The original copies CEnvironment::get_channel_name() into gc_channel.
    // udp_ip_channel is a separate bridge setting and must not be advertised
    // as the selectable channel name.
    strncpy(payload.gc_channel, environment->get_channel_name(),
            sizeof(payload.gc_channel));
    payload.gc_ch_group = static_cast<int16_t>(environment->get_channel_no());

    const channel_script_t* channel_script =
        G_CDataManager()->GetChannelScript();
    const channel_info_t* channel_info =
        channel_script->getChannelInfo(
            // InsertChannel @ 0x86CB88A reads CEnvironment+888 here:
            // the channel-script outer key is the configured server group.
            static_cast<uint8_t>(environment->get_server_group()),
            static_cast<uint32_t>(environment->get_channel_no()));
    if (channel_info) {
        strncpy(payload.gc_channeltype,
                channel_info->dungeon_group_name.c_str(),
                sizeof(payload.gc_channeltype));
        payload.gc_type = channel_info->channel_type;
    }
    payload.server_group = server_group;

    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("world.cpp", 4512), true);
    // Match the original CStreamGuard::operator<<(int) route header.
    *guard << 45;
    *guard << -1;
    guard->put_binary(&payload, sizeof(payload));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x86CBABA
// IDA_NAME: _ZN9GameWorld13DeleteChannelEi17ENUM_SERVER_GROUP
// DEMANGLED: GameWorld::DeleteChannel(int,ENUM_SERVER_GROUP)
void GameWorld::DeleteChannel(int game_channel_no,
                              ENUM_SERVER_GROUP server_group) {
    SIG_DELETE_CHANNEL payload;
    memset(&payload, 0, sizeof(payload));
    payload.gc_no = game_channel_no;
    payload.server_group = server_group;

    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("world.cpp", 4540), true);
    // Match the original CStreamGuard::operator<<(int) route header.
    *guard << 47;
    *guard << -1;
    guard->put_binary(&payload, sizeof(payload));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x86CBBF6
// IDA_NAME: _ZN9GameWorld31is_dungeon_tag_matching_channelEPc
// DEMANGLED: GameWorld::is_dungeon_tag_matching_channel(char *)
int GameWorld::is_dungeon_tag_matching_channel(char* tag) {
    CEnvironment* environment = G_CEnvironment();
    const channel_info_t* channel_info =
        G_CDataManager()->GetChannelScript()->getChannelInfo(
            // is_dungeon_tag_matching_channel @ 0x86CBBF6 uses the same
            // CEnvironment+888 server-group key as InsertChannel.
            static_cast<uint8_t>(environment->get_server_group()),
            static_cast<uint32_t>(environment->get_channel_no()));
    return channel_info &&
        strcmp(tag, channel_info->dungeon_group_name.c_str()) == 0;
}

// IDA_EA: 0x86CF602
// IDA_NAME: _ZN9GameWorld14EraseLoginUserEP5CUser
// DEMANGLED: GameWorld::EraseLoginUser(CUser *)
bool GameWorld::EraseLoginUser(CUser* user) {
    const uint32_t account_id = static_cast<uint32_t>(user->get_acc_id());
    GameWorldLoginUserMap::iterator found = login_users_.find(account_id);
    if (found == login_users_.end()) {
        return false;
    }
    login_users_.erase(found);
    return true;
}
