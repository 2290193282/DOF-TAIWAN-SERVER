#include "old_game/player/core/character_dispatchers.h"

#include "common/config.h"
#include "old_game/player/core/character_db_payloads.h"
#include "old_game/player/core/character_messages.h"
#include "old_game/player/core/character_packets.h"
#include "old_game/activity/condition/character_restrict.h"
#include "old_game/player/core/character_tag_data.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/activity/schedule/decide_levelup_event.h"
#include "old_game/activity/schedule/levelup_support_3rd.h"
#include "old_game/activity/schedule/event_shop_attendance.h"
#include "old_game/economy/equipment/clone_title_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_limit_packets.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/progression/skill/skill_runtime.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_data/db/dungeon/db_get_death_tower_best_record.h"
#include "old_data/db/character/create_charac_payload.h"
#include "old_data/db/character/db_delete_charac.h"
#include "old_data/db/character/load_etc_payload.h"
#include "old_data/db/dungeon/dungeon_clear_payload.h"
#include "old_data/db/inventory/load_inventory_payload.h"
#include "old_data/db/inventory/load_ticket_payload.h"
#include "old_data/db/anti_cheat/db_req_black_count.h"
#include "old_data/db/anti_cheat/db_save_same_mid_other_char_login_hack.h"
#include "old_data/db/character/db_get_user_regdate.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "core/config/env/environment.h"
#include "old_game/progression/skill/create_character_skill_abi.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/character_growth_tables.h"
#include "old_data/PVF/character/datamanager/character_script_runtime.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "old_game/world/region/gameworld_public_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/gm_accounts_abi.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/exchange_server_hit_rate_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/match/channel_zombie_matching_producer.h"
#include "old_game/world/region/game_world.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "old_game/economy/inventory/item.h"
#include "infra/security/policy/restrict_inputting_name.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/slang_filter.h"
#include "common/manager/user.h"
#include "infra/log/format/log_manager.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"
#include "infra/concurrent/timer/timer_queue.h"

// This translation unit is the unique owner for the three state-2 template
// specializations below.  Keeping the definition local prevents accidental
// COMDAT/vtable providers in a consumer header.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

#include <cstdio>
#include <cstring>
#include <new>
#include <stdint.h>
#include <string>
#include <vector>

extern int checkSpecialChar(char* name) __asm__("_Z16checkSpecialCharPc");
extern int checkSpecialCharName(char* name)
    __asm__("_Z20checkSpecialCharNamePc");

typedef char MSG_CREATE_CHARACTER_Size_Must_Be_0x31[
    (sizeof(MSG_CREATE_CHARACTER) == 0x31) ? 1 : -1];
typedef char MSG_DELETE_CHARACTER_Size_Must_Be_0x30[
    (sizeof(MSG_DELETE_CHARACTER) == 0x30) ? 1 : -1];

namespace {

bool IsServiceRestricted(CUser* user, int category, int restrict_id) {
    ServiceRestrictManager* manager =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    return manager &&
           manager->isRestricted(
               user, static_cast<RestrictCategory::Enum>(category),
               restrict_id);
}

bool IsGmAccount(CUser* user) {
    return GlobalData::s_pGMAccounts_ &&
           (GlobalData::s_pGMAccounts_)->isGM(
               static_cast<unsigned int>(user->get_acc_id()));
}

void MakeInvenItemFromItem(CItem* item, int item_index, Inven_Item& inven) {
    // `inven` has just run Inven_Item::C2 at every caller.  The original
    // makeGift path writes the item index at +0x02 before the CItem vtable
    // callback; C2/reset deliberately leaves the GUID member untouched.
    inven.item_id_ = item_index;
    if (!item) {
        return;
    }
    typedef void (*MakeInvenItemFn)(CItem*, Inven_Item*);
    void** vtable = *reinterpret_cast<void***>(item);
    MakeInvenItemFn make =
        reinterpret_cast<MakeInvenItemFn>(vtable[2]);
    make(item, &inven);
}

int LegacyCreateCharacterWeapon(int job) {
    // Jobs 0..10 preserve makeGift @0x81C0A1A exactly; AT Swordman is the
    // additional job-11 value proven by the 0828 official .chr.  makeGift
    // consults the published [create equipment list] first, so this switch is
    // used only when no character-script table was published (bootstrap path).
    switch (job) {
        case oldsrv::config::kCharacterJobSwordman:
        case oldsrv::config::kCharacterJobDemonicSwordman:
        case oldsrv::config::kCharacterJobAtSwordman:
            return 27600;
        case oldsrv::config::kCharacterJobFighter:
        case oldsrv::config::kCharacterJobAtFighter:
            return 29900;
        case oldsrv::config::kCharacterJobGunner:
        case oldsrv::config::kCharacterJobAtGunner:
            return 31300;
        case oldsrv::config::kCharacterJobMage:
        case oldsrv::config::kCharacterJobAtMage:
            return 33600;
        case oldsrv::config::kCharacterJobPriest:
            return 35001;
        case oldsrv::config::kCharacterJobThief:
            return 37002;
        case oldsrv::config::kCharacterJobCreatorMage:
            return 106040042;
        default:
            return 0;
    }
}

void SendBasicInfoForSelectList(CUser* user) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(2);
    user->make_basic_info(packet, 2);
    packet.finalize(true);
    user->Send(packet);
}

int CreateCharacDbErrorToCmdError(int db_result) {
    switch (db_result) {
        case 1:
            return 18;
        case 4:
            return 20;
        case 5:
            return 95;
        case 3:
            return 24;
        default:
            return 2;
    }
}

int DeleteCharacDbErrorToCmdError(int db_result) {
    switch (db_result) {
        case 2:
            return 36;
        case 3:
            return 213;
        case 1:
        default:
            return 2;
    }
}

void MakeRewardInvenItem(CItem* item, int item_index, Inven_Item& inven) {
    MakeInvenItemFromItem(item, item_index, inven);
    inven.set_add_info(1);
}

const char* FindScriptString(const std::string& key) {
    const char* text = g_scriptStringManager_.findString(4, key.c_str(), 0);
    return text ? text : "";
}

bool RewardInfoCanApply(const createChracScript::rewardInfo& reward,
                        int reg_time) {
    if (reward.start_time > 0 && reward.start_time > reg_time) {
        return false;
    }
    if (reward.end_time > 0 && reward.end_time <= reg_time) {
        return false;
    }
    return true;
}

int QueueLoadEtcRequest(CUser* user,
                        uint32_t charac_no,
                        uint32_t tag_charac_no,
                        uint32_t charac_job,
                        bool use_exchange,
                        int last_channel_no,
                        int last_charac_no) {
    if (!user || !charac_no) {
        return 0;
    }

    Stream* stream =
        GlobalData::big_stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                               6164);
    CStreamGuard guard(stream, true);
    *guard << 32;
    *guard << user->GetUID();
    user->DisableCharacInfo(static_cast<CACHE_SECTION>(11));
    if (use_exchange) {
        *guard << last_channel_no;
        *guard << last_charac_no;
    }

    SIG_LOAD_ETC* const payload = guard.GetInBuffer<SIG_LOAD_ETC>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = charac_no;
    payload->account_id = static_cast<uint32_t>(user->get_acc_id());
    CEnvironment* const environment = G_CEnvironment();
    payload->server_info =
        static_cast<uint32_t>(environment->get_server_group());
    payload->channel_no =
        static_cast<uint32_t>(environment->get_channel_no());
    payload->tag_charac_no = tag_charac_no;
    payload->tag_charac_etc.clear();
    payload->combo_skill.charac_job = charac_job;

    const MsgQueueMgr::QUEUE_IDX queue =
        use_exchange ? static_cast<MsgQueueMgr::QUEUE_IDX>(5)
                     : MsgQueueMgr::DB_Q;
    return GlobalData::msgq_mgr()->put(queue, guard) == 1 ? 1 : 0;
}

}  // namespace

namespace character_select_load_order {

int RequestEtcAfterInventory(CUser* user, unsigned int tag_charac_no) {
    if (!user || !user->getCurCharacR() ||
        static_cast<unsigned char>(user->isLocked4DataLoad()) != 1u) {
        return 0;
    }

    // The original exchange-cache decision fields are cleared after the
    // initial fan-out.  A post-inventory ETC request deliberately uses DB_Q:
    // correctness and the load fence take precedence over an ETC-only cache
    // hit, while the large inventory payload still benefits from its cache.
    return QueueLoadEtcRequest(
        user,
        static_cast<uint32_t>(user->getCurCharacNo()),
        static_cast<uint32_t>(tag_charac_no),
        static_cast<uint32_t>(user->get_charac_job()),
        false,
        0,
        0);
}

}  // namespace character_select_load_order

// IDA_EA: 0x84BEE06
// IDA_NAME: _ZN18Inter_CreateCharac12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CreateCharac::dispatch_sig(CUser *,char *,int)
int Inter_CreateCharac::dispatch_sig(CUser* user,
                                     char* payload,
                                     int payload_size) {
    if (!user || !payload ||
        payload_size != static_cast<int>(sizeof(SIG_CREATE_CHARAC))) {
        return 0;
    }
    SIG_CREATE_CHARAC* const create_charac =
        reinterpret_cast<SIG_CREATE_CHARAC*>(payload);

    if (!user->get_state()) {
        return 1728;
    }
    if (static_cast<unsigned char>(user->unlockCreateCharac()) != 1u) {
        return 1732;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_CREATE_CHARACTER);

    // CLIENT_IDA_EVIDENCE: handler sub_55ECC0 receives the framework-decoded
    // success and error bytes and performs no additional packet-body reads.
    const int db_result = static_cast<int>(create_charac->result);
    if (db_result) {
        packet.put_byte(0);
        packet.put_byte(CreateCharacDbErrorToCmdError(db_result));
        packet.finalize(true);
        user->Send(packet);
        return 0;
    }

    if (create_charac->event_character_grow_type > 0) {
        user->setUserEventCharacterFlag(
            create_charac->user_event_character_flag);
    }
    user->CreateCharac(create_charac);
    decide_levelup_event::BindCreatedCharacter(
        user,
        create_charac->character_no,
        create_charac->event_character_grow_type);

    packet.put_byte(1);
    packet.finalize(true);
    user->Send(packet);

    SendBasicInfoForSelectList(user);
    user->UpdateCharacView();
    process_event(user, create_charac);
    return 0;
}

// IDA_EA: 0x84BF4AC
// IDA_NAME: _ZN18Inter_CreateCharac13process_eventEP5CUserP17SIG_CREATE_CHARAC
// DEMANGLED: Inter_CreateCharac::process_event(CUser *,SIG_CREATE_CHARAC *)
int Inter_CreateCharac::process_event(CUser* user, SIG_CREATE_CHARAC* payload) {
    process_createGift(user, payload);
    return 0;
}

// IDA_EA: 0x84BF4CE
// IDA_NAME: _ZN18Inter_CreateCharac18process_createGiftEP5CUserP17SIG_CREATE_CHARAC
// DEMANGLED: Inter_CreateCharac::process_createGift(CUser *,SIG_CREATE_CHARAC *)
void Inter_CreateCharac::process_createGift(CUser* user, SIG_CREATE_CHARAC* payload) {
    if (!user) {
        return;
    }

    const int job = static_cast<int>(payload->job);
    createChracScript::rewardInfoList rewards;
    G_CDataManager()->get_createCharacterGift(job, rewards);
    if (rewards.rewards.empty()) {
        return;
    }

    const int reg_time = user->getRegTime();
    const int server_group = user->GetServerGroupToPvP();
    const unsigned int charac_no = payload->character_no;

    for (std::vector<createChracScript::rewardInfo>::const_iterator reward =
             rewards.rewards.begin();
         reward != rewards.rewards.end();
         ++reward) {
        if (!RewardInfoCanApply(*reward, reg_time)) {
            continue;
        }

        for (std::vector<int>::const_iterator item_it =
                 reward->item_indexes.begin();
             item_it != reward->item_indexes.end();
             ++item_it) {
            CItem* item = G_CDataManager()->find_item(*item_it);
            if (!item || !item->get_index()) {
                continue;
            }

            Inven_Item inven;
            MakeRewardInvenItem(item, *item_it, inven);
            const char* title = FindScriptString(reward->create_title);
            const char* text = FindScriptString(reward->create_text);
            const int text_len = static_cast<int>(std::strlen(text));
            WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                title,
                inven,
                0,
                charac_no,
                text,
                text_len,
                0,
                static_cast<ENUM_SERVER_GROUP>(server_group),
                true,
                false);
        }
    }
}

// IDA_EA: 0x84E9B78
// IDA_NAME: _ZN18Inter_CreateCharacC2Ev
// DEMANGLED: Inter_CreateCharac::Inter_CreateCharac(void)
Inter_CreateCharac::Inter_CreateCharac() {}

// IDA_EA: 0x84E9B94
// IDA_NAME: _ZN18Inter_CreateCharacD2Ev
// DEMANGLED: Inter_CreateCharac::~Inter_CreateCharac()
// IDA_EA: 0x84E9BC4
// IDA_NAME: _ZN18Inter_CreateCharacD0Ev
// DEMANGLED: Inter_CreateCharac::~Inter_CreateCharac() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
Inter_CreateCharac::~Inter_CreateCharac() {}

// IDA_EA: 0x84BF810
// IDA_NAME: _ZN18Inter_DeleteCharac12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_DeleteCharac::dispatch_sig(CUser *,char *,int)
int Inter_DeleteCharac::dispatch_sig(CUser* user,
                                     char* payload,
                                     int payload_size) {
    (void)payload_size;
    SIG_DELETE_CHARAC* const delete_charac =
        reinterpret_cast<SIG_DELETE_CHARAC*>(payload);

    if (!user->get_state()) {
        return 2066;
    }
    if (static_cast<unsigned char>(user->unlockDeleteCharac()) != 1u) {
        return 2071;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_DELETE_CHARACTER);

    const int db_result = static_cast<int>(delete_charac->result);
    if (db_result) {
        packet.put_byte(0);
        packet.put_byte(DeleteCharacDbErrorToCmdError(db_result));
        packet.finalize(true);
        user->Send(packet);
        return db_result == 1 ? 2085 : 0;
    }

    Packet_Monitor_Charac_Delete monitor_packet;
    monitor_packet.account_id =
        static_cast<uint32_t>(user->get_acc_id());
    monitor_packet.charac_no = delete_charac->charac_no;
    monitor_packet.guild_id = delete_charac->guild_id;
    ::CMonitorServerProxy* proxy =
        ::GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendTcpPacket(
        reinterpret_cast<char*>(&monitor_packet),
        sizeof(monitor_packet));

    user->DeleteCharac(delete_charac->charac_no);
    packet.put_byte(1);
    if (delete_charac->event_character_deleted) {
        user->setUserEventCharacterFlag(0);
        packet.put_byte(1);
    } else {
        packet.put_byte(0);
    }
    packet.finalize(true);
    user->Send(packet);

    SendBasicInfoForSelectList(user);
    user->UpdateCharacView();
    return 0;
}

// IDA_EA: 0x84E9C4C
// IDA_NAME: _ZN18Inter_DeleteCharacC2Ev
// DEMANGLED: Inter_DeleteCharac::Inter_DeleteCharac(void)
Inter_DeleteCharac::Inter_DeleteCharac() {}

// IDA_EA: 0x84E9C68
// IDA_NAME: _ZN18Inter_DeleteCharacD2Ev
// DEMANGLED: Inter_DeleteCharac::~Inter_DeleteCharac()
// IDA_EA: 0x84E9C98
// IDA_NAME: _ZN18Inter_DeleteCharacD0Ev
// DEMANGLED: Inter_DeleteCharac::~Inter_DeleteCharac() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
Inter_DeleteCharac::~Inter_DeleteCharac() {}

// IDA_EA: 0x84C3398
// IDA_NAME: _ZN16Inter_LoadCharac12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadCharac::dispatch_sig(CUser *,char *,int)
int Inter_LoadCharac::dispatch_sig(CUser* user,
                                   char* payload,
                                   int payload_size) {
    (void)payload_size;
    SIG_LOAD_CHARAC* const load_charac =
        reinterpret_cast<SIG_LOAD_CHARAC*>(payload);

    if (!user->get_state()) {
        return 5085;
    }
    if (load_charac->result) {
        return 5091;
    }
    if (static_cast<uint32_t>(user->get_acc_id()) !=
        load_charac->charac_data.member_id) {
        // IDA_EA: 0x84C3398, original InterDispatcher.cpp:5113 branch.
        // Account disagreement is auditable, then preserves the original
        // 5125 result; it must not be silently treated as a generic failure.
        Stream* stream = GlobalData::stream_pool()->Acquire(
            "InterDispatcher.cpp", 5113);
        CStreamGuard guard(stream, true);
        guard << 157;
        guard << -1;
        SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK* const audit =
            guard.GetInBuffer<SIG_SAVE_SAME_MID_OTHER_CHAR_LOGIN_HACK>();
        audit->first_mid = static_cast<uint32_t>(user->get_acc_id());
        audit->second_mid = load_charac->charac_data.member_id;
        audit->charac_no = load_charac->charac_data.character_no;
        user->network_.GetPeerIP2(audit->peer_ip, sizeof(audit->peer_ip));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::LOGDB_Q, guard);
        return 5125;
    }
    if (!load_charac->charac_data.village) {
        return 5131;
    }

    const unsigned int charac_no = load_charac->character_no;
    const int slot = static_cast<signed char>(
        user->get_charac_idx( charac_no));
    if (slot < 0 || slot == 0xFF ||
        static_cast<unsigned char>(user->setCharacInfoDetail(
            slot,
            charac_no,
            &load_charac->charac_data)) != 1u) {
        return 5141;
    }

    int tag_slot = -1;
    const unsigned int tag_charac_no =
        load_charac->charac_data.tag_character_no;
    if (tag_charac_no) {
        tag_slot = static_cast<signed char>(
            user->get_charac_idx( tag_charac_no));
        if (tag_slot != -1 && tag_slot != 0xFF) {
            if (static_cast<unsigned char>(user->setTagCharacInfo(
                    tag_slot,
                    tag_charac_no,
                    &load_charac->tag_charac_data)) != 1u) {
                return 5151;
            }
            if (slot >= 0 &&
                user->characters_.size() > static_cast<std::size_t>(slot)) {
                user->characters_[static_cast<std::size_t>(slot)]
                    .fields_.striker_skill_index_ =
                    load_charac->charac_data.striker_skill_index;
            }
        }
    }

    user->SelectCharac(slot, tag_slot);
    const int charac_count = user->get_charac_count();
    user->decideLinkCharacSlotIndexes(static_cast<char>(slot), charac_count);
    if (static_cast<unsigned char>(user->checkCharacLinkType(
            static_cast<char>(slot), user->get_charac_count())) != 1u) {
        return 5169;
    }
    if (static_cast<unsigned char>(
            user->isCharacLinkBonusSetting(static_cast<char>(slot)))) {
        user->setCharacLinkBonusSetting(slot, false);
    }

    user->SetCurCharacEmotion(load_charac->charac_data.emotion);
    const unsigned char exp_slot_stat =
        static_cast<unsigned char>(load_charac->charac_data.add_slot_flag);
    user->SetCurExpEquipSlotStat(
        exp_slot_stat < 4u ? static_cast<char>(exp_slot_stat) : 0);
    user->setCurCharacHelpAbuseComputedRatio(
        static_cast<unsigned int>(
            user->calc_disp_help_abuse_ratio(0)));
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(0));
    // IDA_EA: 0x84C3398 keeps the data-load lock until Inter_LoadEtc's
    // successful completion owns the single unlock4DataLoad() transition.

    WongWork::CMailBoxHelper::ReqDBLoadMail(user, charac_no);
    if (G_GameWorld()) {
        G_GameWorld()->InsertUserToIDMap(
            load_charac->charac_data.character_name,
            static_cast<unsigned int>(user->get_acc_id()));
    }

    CDataManager* data_manager = G_CDataManager();
    EventClassify::CEventScriptMng* event_script_mng =
        data_manager ? data_manager->get_event_script_mng() : 0;
    if (event_script_mng &&
        static_cast<unsigned char>(
            event_script_mng->is_eventing(411u)) == 1u) {
        user->setCurUseBringUpMaterialTotal(
            load_charac->charac_data.event_bring_up_material_count);
        user->setBringUpRewardFlag(
            load_charac->charac_data.event_bring_up_reward_flag != 0);
        user->SendEventBringUpInfo();
    }
    user->select_charac_set_visible_values(
        load_charac->charac_data.visible_flags);
    user->decide_growth_power_reward_system();
    // The third level-up-support watermark is character-scoped.  Login-info
    // returns while the user is still at account/character-list state and no
    // current character exists; queue its load only after SelectCharac has
    // installed the authoritative character identity.
    levelup_support_3rd::RequestLoginState(user);
    event_attendance::RequestLoginState(user);
    return 0;
}

// IDA_EA: 0x84E9D20
// IDA_NAME: _ZN16Inter_LoadCharacC2Ev
// DEMANGLED: Inter_LoadCharac::Inter_LoadCharac(void)
Inter_LoadCharac::Inter_LoadCharac() {}

// IDA_EA: 0x84E9D3C
// IDA_NAME: _ZN16Inter_LoadCharacD2Ev
// DEMANGLED: Inter_LoadCharac::~Inter_LoadCharac()
// IDA_EA: 0x84E9D6C
// IDA_NAME: _ZN16Inter_LoadCharacD0Ev
// DEMANGLED: Inter_LoadCharac::~Inter_LoadCharac() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
Inter_LoadCharac::~Inter_LoadCharac() {}

// IDA_EA: 0x84C528A
// IDA_NAME: _ZN18Inter_SelectCharac12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_SelectCharac::dispatch_sig(CUser *,char *,int)
int Inter_SelectCharac::dispatch_sig(CUser* user,
                                     char* payload,
                                     int payload_size) {
    (void)user;
    (void)payload;
    (void)payload_size;
    return 0;
}

// IDA_EA: 0x84FA236
// IDA_NAME: _ZN18Inter_SelectCharacD2Ev
// DEMANGLED: Inter_SelectCharac::~Inter_SelectCharac()
// IDA_EA: 0x84FA266
// IDA_NAME: _ZN18Inter_SelectCharacD0Ev
// DEMANGLED: Inter_SelectCharac::~Inter_SelectCharac() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
Inter_SelectCharac::~Inter_SelectCharac() {}

// IDA_EA: 0x85950FC
// IDA_NAME: _ZN23DisPatcher_CreateCharacC2Ev
// DEMANGLED: DisPatcher_CreateCharac::DisPatcher_CreateCharac(void)
DisPatcher_CreateCharac::DisPatcher_CreateCharac()
    : IPacketDispatcher<MSG_CREATE_CHARACTER,
                        stCreateCharac,
                        static_cast<ch_state>(2)>() {}

// IDA_EA: 0x823207A
// IDA_NAME: _ZN23DisPatcher_CreateCharacD2Ev
// DEMANGLED: DisPatcher_CreateCharac::~DisPatcher_CreateCharac()
// IDA_EA: 0x82320AA
// IDA_NAME: _ZN23DisPatcher_CreateCharacD0Ev
// DEMANGLED: DisPatcher_CreateCharac::~DisPatcher_CreateCharac() deleting destructor
DisPatcher_CreateCharac::~DisPatcher_CreateCharac() {}

// IDA_EA: 0x81C01EC
// IDA_NAME: _ZN23DisPatcher_CreateCharac4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_CreateCharac::read(PacketBuf &,MSG_BASE &)
int DisPatcher_CreateCharac::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_CREATE_CHARACTER& create =
        static_cast<MSG_CREATE_CHARACTER&>(message);
    std::memset(&create, 0, sizeof(create));

    if (packet.get_byte(create.character_job) != 1) {
        return LineFunc(1865,
                        "virtual int DisPatcher_CreateCharac::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (create.character_job > oldsrv::config::kCharacterJobCommon) {
        return LineFunc(1867,
                        "virtual int DisPatcher_CreateCharac::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (create.character_job == oldsrv::config::kCharacterJobCommon) {
        return LineFunc(1891,
                        "virtual int DisPatcher_CreateCharac::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }

    if (packet.get_int(create.character_name_length) != 1) {
        return LineFunc(1894,
                        "virtual int DisPatcher_CreateCharac::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (create.character_name_length <= 3u ||
        create.character_name_length > 18u) {
        return LineFunc(1898,
                        "virtual int DisPatcher_CreateCharac::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_str(create.character_name,
                       sizeof(create.character_name),
                       static_cast<int>(create.character_name_length)) != 1) {
        return LineFunc(1902,
                        "virtual int DisPatcher_CreateCharac::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    // CLIENT_IDA_EVIDENCE: writers sub_1387FB0 and sub_138B930 append two
    // bytes.  The first is the event UI selection ordinal; the second is the
    // canonical grow type stored by the existing create-character DB chain.
    if (packet.get_byte(create.event_character_selection_ordinal) != 1 ||
        packet.get_byte(create.event_character_grow_type) != 1) {
        return LineFunc(1920,
                        "virtual int DisPatcher_CreateCharac::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (static_cast<signed char>(
            create.event_character_selection_ordinal) >= 0 &&
        static_cast<signed char>(create.event_character_grow_type) >= 0) {
        return 0;
    }
    return LineFunc(1922,
                    "virtual int DisPatcher_CreateCharac::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81C041E
// IDA_NAME: _ZN23DisPatcher_CreateCharac7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_CreateCharac::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_CreateCharac::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    MSG_CREATE_CHARACTER& create =
        static_cast<MSG_CREATE_CHARACTER&>(message);
    stCreateCharac& create_param = static_cast<stCreateCharac&>(param);
    create_param.result = check_error(user, message);
    const int result = create_param.result;
    if (result > 0) {
        return 0;
    }
    if (result < 0) {
        return LineFunc(
            1948,
            "virtual int DisPatcher_CreateCharac::process(CUser*, MSG_BASE&, ParamBase&)",
            result,
            static_cast<unsigned int>(user->get_acc_id()));
    }

    G_CDataManager()->setCharacNameFilterPass(IsGmAccount(user));

    Stream* stream =
        GlobalData::big_stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                               1963);
    CStreamGuard guard(stream, true);
    *guard << 4;
    *guard << user->GetUID();

    SIG_CREATE_CHARAC* payload = guard.GetInBuffer<SIG_CREATE_CHARAC>();
    std::memset(payload, 0, sizeof(*payload));
    payload->account_id = static_cast<uint32_t>(user->get_acc_id());
    payload->job = static_cast<int8_t>(create.character_job);
    std::strncpy(payload->character_name,
                 create.character_name,
                 sizeof(payload->character_name));
    // The selection ordinal is transient client UI state.  Only the canonical
    // grow type enters the restored SIG_CREATE_CHARAC and DB persistence ABI.
    payload->event_character_grow_type =
        static_cast<int8_t>(create.event_character_grow_type);

    if (payload->event_character_grow_type &&
        static_cast<unsigned char>(user->getUserEventCharacterFlag())) {
        create_param.result = 20;
        return 0;
    }

    payload->user_event_character_flag =
        static_cast<uint8_t>(user->getUserEventCharacterFlag());
    CDataManager* const data_manager = G_CDataManager();
    const CCharacter& initial_character =
        data_manager->get_character_list()
            ->characters_[static_cast<unsigned int>(create.character_job)];
    const _Additioal_info& source_stats = initial_character.base_status_[0];
    CREATE_CHARACTER_INITIAL_STATS& target_stats = payload->initial_stats;
    target_stats.max_hp = source_stats.max_hp;
    target_stats.max_mp = source_stats.max_mp;
    target_stats.physical_attack = source_stats.physical_attack;
    target_stats.physical_defense = source_stats.physical_defense;
    target_stats.magical_attack = source_stats.magical_attack;
    target_stats.magical_defense = source_stats.magical_defense;
    for (unsigned int index = 0; index < 4u; ++index) {
        target_stats.element_resist[index] = source_stats.element_resist[index];
    }
    for (unsigned int index = 0; index < 17u; ++index) {
        target_stats.spec_property[index] = source_stats.spec_property[index];
    }
    target_stats.inventory_weight = source_stats.inventory_weight;
    target_stats.hp_regeneration = source_stats.hp_regen_speed;
    target_stats.mp_regeneration = source_stats.mp_regen_speed;
    target_stats.move_speed = source_stats.move_speed;
    target_stats.attack_speed = source_stats.attack_speed;
    target_stats.cast_speed = source_stats.cast_speed;
    target_stats.hit_recovery = source_stats.hit_recovery;
    target_stats.jump = source_stats.jump_power;
    target_stats.character_weight = source_stats.character_weight;
    payload->initial_grow_flag =
        initial_character.get_initial_create_grow_flag();
    payload->initial_village = initial_character.get_start_town();
    if (!payload->initial_village) {
        payload->initial_village = 1;
    }
    addSkillOnCreateCharacter(payload->skill_slot, payload->job);
    addSkillOnCreateCharacter(payload->skill_slot_second, payload->job);
    payload->remain_sp =
        static_cast<uint32_t>(data_manager->GetSpAtLevelUp(1));
    makeGift(user, payload);

    const char* ssn = user->GetSsnString();
    payload->sex =
        !ssn || std::strlen(ssn) <= 0x0Cu ||
        (ssn[6] != '2' && ssn[6] != '4');
    if (payload->job == oldsrv::config::kCharacterJobDemonicSwordman) {
        user->setComboSkillSet(true);
    }

    if (static_cast<unsigned char>(user->lockCreateCharac()) == 1u) {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        return 0;
    }
    return LineFunc(
        2079,
        "virtual int DisPatcher_CreateCharac::process(CUser*, MSG_BASE&, ParamBase&)",
        0,
        static_cast<unsigned int>(user->get_acc_id()));
}

// IDA_EA: 0x81C07E8
// IDA_NAME: _ZN23DisPatcher_CreateCharac4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_CreateCharac::send(CUser *,ParamBase &)
void DisPatcher_CreateCharac::send(CUser* user, ParamBase& param) {
    const int result = static_cast<stCreateCharac&>(param).result;
    if (result != 0x7FFFFFFF) {
        if (result) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_CREATE_CHARACTER,
                                     static_cast<unsigned char>(result));
        }
    }
}

// IDA_EA: 0x81C0830
// IDA_NAME: _ZN23DisPatcher_CreateCharac11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_CreateCharac::check_error(CUser *,MSG_BASE &)
int DisPatcher_CreateCharac::check_error(CUser* user, MSG_BASE& message) {
    if (user->get_state() != 2) {
        return 0x7FFFFFFF;
    }
    if (static_cast<unsigned char>(user->isLockedDeleteCharac())) {
        return 19;
    }
    if (static_cast<unsigned char>(user->isLocked4DataLoad())) {
        return 19;
    }
    if (IsServiceRestricted(user, 1, 32)) {
        return 209;
    }

    CEventBase* event = GlobalData::s_event_manager
                            ? GlobalData::s_event_manager->GetRepeatEvent(49)
                            : 0;
    if (event && event->IsEventing(user)) {
        CRestrictCharacCreationEvent* restrict_event =
            static_cast<CRestrictCharacCreationEvent*>(event);
        const int limit = restrict_event->getLimitCreateCharacter(
            user->get_charac_count());
        if (limit) {
            return limit;
        }
    }

    const int charac_count = user->get_charac_count();
    if (charac_count >=
        static_cast<unsigned char>(user->getCharacSlotLimit())) {
        return 4;
    }
    if (user->get_charac_count() == 36) {
        cMyTrace trace(
            "int DisPatcher_CreateCharac::check_error(CUser*, MSG_BASE&)",
            2208,
            5);
        trace("DisPatcher_CreateCharac pUser->get_charac_count() == MAX_CHARAC_SLOT false");
        return 4;
    }

    MSG_CREATE_CHARACTER& create =
        static_cast<MSG_CREATE_CHARACTER&>(message);
    // RECONSTRUCTED_0828: the two CMD5 event-character bytes belong to the
    // CMD561 reservation lifecycle; ordinary character creation remains
    // unchanged.  Error 19 is the client's "需要设定预约等级" path.
    if (create.event_character_grow_type > 0 &&
        !decide_levelup_event::HasPendingReservation(user)) {
        return 19;
    }
    if (restrict_inputting_name::isUtf8StrInUnicodeRange(
            create.character_name) !=
        1) {
        return 159;
    }
    if (static_cast<unsigned char>(checkSpecialChar(create.character_name)) ||
        static_cast<unsigned char>(
            checkSpecialCharName(create.character_name))) {
        return 159;
    }
    return 0;
}

// IDA_EA: 0x81C0A1A
// IDA_NAME: _ZN23DisPatcher_CreateCharac8makeGiftEP5CUserP17SIG_CREATE_CHARAC
// DEMANGLED: DisPatcher_CreateCharac::makeGift(CUser *,SIG_CREATE_CHARAC *)
int DisPatcher_CreateCharac::makeGift(CUser* user,
                                      SIG_CREATE_CHARAC* payload) {
    (void)user;
    const int job = static_cast<int>(payload->job);
    int equip0 = 0;
    int equip1 = 0;
    int inventory_gift = 0;
    const bool has_pvf_coat =
        FindCharacterCreateEquipment(job,
                                     oldsrv::config::kEquipmentTypeCoat,
                                     &equip0);
    const bool has_pvf_pants =
        FindCharacterCreateEquipment(job,
                                     oldsrv::config::kEquipmentTypePants,
                                     &equip1);
    const bool has_pvf_weapon =
        FindCharacterCreateEquipment(job,
                                     oldsrv::config::kEquipmentTypeWeapon,
                                     &inventory_gift);
    if (!has_pvf_coat && !has_pvf_pants && !has_pvf_weapon &&
        job >= 0 && job < oldsrv::config::kCharacterJobCount) {
        // Compatibility is deliberately downstream of all three 0828
        // lookups.  A successfully published character PVF is authoritative;
        // the fixed table only keeps old bootstrap/direct-import callers
        // functional when that sidecar has never been published.
        equip0 = static_cast<int>(CREATECHARACTEREQUIP[job * 2]);
        equip1 = static_cast<int>(CREATECHARACTEREQUIP[job * 2 + 1]);
        inventory_gift = LegacyCreateCharacterWeapon(job);
        std::fprintf(stderr,
                     "[character-create] PVF create-equipment table absent "
                     "for job=%d; using legacy bootstrap values\n",
                     job);
    } else if (!has_pvf_coat || !has_pvf_pants || !has_pvf_weapon) {
        // A partial publication is never a valid compatibility state.  Do not
        // combine PVF values with fixed-table values; keep the failure visible
        // and let the existing item-presence gates suppress the incomplete
        // gift projection.
        std::fprintf(stderr,
                     "[character-create] incomplete PVF create-equipment "
                     "record job=%d coat=%d pants=%d weapon=%d\n",
                     job,
                     has_pvf_coat ? 1 : 0,
                     has_pvf_pants ? 1 : 0,
                     has_pvf_weapon ? 1 : 0);
    }
    CItem* item0 = equip0 > 0 ? G_CDataManager()->find_item(equip0) : 0;
    CItem* item1 = equip1 > 0 ? G_CDataManager()->find_item(equip1) : 0;

    if (item0 && item1) {
        payload->has_equipment = 1;
        Inven_Item inven0;
        Inven_Item inven1;
        MakeInvenItemFromItem(item0, equip0, inven0);
        MakeInvenItemFromItem(item1, equip1, inven1);

        // IDA @ 0x81C0AE4 copies all 0x3d bytes from the first temporary to
        // SIG_CREATE_CHARAC::equipment[2].  Keep every physical member here:
        // `setCopy` intentionally omits the GUID and is not equivalent.
        Inven_Item& equipment0 = payload->equipment[2];
        equipment0.seal_or_state_ = inven0.seal_or_state_;
        equipment0.item_category_ = inven0.item_category_;
        equipment0.item_id_ = inven0.item_id_;
        equipment0.item_attr_ = inven0.item_attr_;
        equipment0.add_info_ = inven0.add_info_;
        equipment0.avatar_select_ = inven0.avatar_select_;
        equipment0.extend_info_ = inven0.extend_info_;
        equipment0.amplify_option_.ability_type_and_identification_ =
            inven0.amplify_option_.ability_type_and_identification_;
        equipment0.amplify_option_.ability_value_ =
            inven0.amplify_option_.ability_value_;
        equipment0.amplify_option_.lock_ = inven0.amplify_option_.lock_;
        equipment0.globally_unique_id_.server_id_ =
            inven0.globally_unique_id_.server_id_;
        equipment0.globally_unique_id_.datetime_ =
            inven0.globally_unique_id_.datetime_;
        equipment0.globally_unique_id_.sequence_no_ =
            inven0.globally_unique_id_.sequence_no_;
        equipment0.chronicle_random_option_.enchanted_skill_options_[0].flags_ =
            inven0.chronicle_random_option_.enchanted_skill_options_[0].flags_;
        equipment0.chronicle_random_option_.enchanted_skill_options_[1].flags_ =
            inven0.chronicle_random_option_.enchanted_skill_options_[1].flags_;
        equipment0.chronicle_random_option_.value_ =
            inven0.chronicle_random_option_.value_;
        equipment0.chronicle_random_option_.random_option_.option_fields_[0]
            .option_index_ =
            inven0.chronicle_random_option_.random_option_.option_fields_[0]
                .option_index_;
        equipment0.chronicle_random_option_.random_option_.option_fields_[0]
            .first_value_ =
            inven0.chronicle_random_option_.random_option_.option_fields_[0]
                .first_value_;
        equipment0.chronicle_random_option_.random_option_.option_fields_[0]
            .second_value_ =
            inven0.chronicle_random_option_.random_option_.option_fields_[0]
                .second_value_;
        equipment0.chronicle_random_option_.random_option_.option_fields_[1]
            .option_index_ =
            inven0.chronicle_random_option_.random_option_.option_fields_[1]
                .option_index_;
        equipment0.chronicle_random_option_.random_option_.option_fields_[1]
            .first_value_ =
            inven0.chronicle_random_option_.random_option_.option_fields_[1]
                .first_value_;
        equipment0.chronicle_random_option_.random_option_.option_fields_[1]
            .second_value_ =
            inven0.chronicle_random_option_.random_option_.option_fields_[1]
                .second_value_;
        equipment0.chronicle_random_option_.random_option_.option_fields_[2]
            .option_index_ =
            inven0.chronicle_random_option_.random_option_.option_fields_[2]
                .option_index_;
        equipment0.chronicle_random_option_.random_option_.option_fields_[2]
            .first_value_ =
            inven0.chronicle_random_option_.random_option_.option_fields_[2]
                .first_value_;
        equipment0.chronicle_random_option_.random_option_.option_fields_[2]
            .second_value_ =
            inven0.chronicle_random_option_.random_option_.option_fields_[2]
                .second_value_;
        equipment0.chronicle_random_option_.random_option_.initial_seed_.seed_ =
            inven0.chronicle_random_option_.random_option_.initial_seed_.seed_;
        equipment0.chronicle_random_option_.random_option_.changed_option_
            .option_index_ =
            inven0.chronicle_random_option_.random_option_.changed_option_
                .option_index_;
        equipment0.chronicle_random_option_.random_option_.changed_option_
            .first_value_ =
            inven0.chronicle_random_option_.random_option_.changed_option_
                .first_value_;
        equipment0.chronicle_random_option_.random_option_.changed_option_
            .second_value_ =
            inven0.chronicle_random_option_.random_option_.changed_option_
                .second_value_;
        equipment0.chronicle_random_option_.random_option_.changed_option_seed_
            .seed_ =
            inven0.chronicle_random_option_.random_option_.changed_option_seed_
                .seed_;
        equipment0.upgrade_separate_.bits_ = inven0.upgrade_separate_.bits_;
        equipment0.reserved_capacity_.value_0_ =
            inven0.reserved_capacity_.value_0_;
        equipment0.reserved_capacity_.value_1_ =
            inven0.reserved_capacity_.value_1_;
        equipment0.reserved_capacity_.value_2_ =
            inven0.reserved_capacity_.value_2_;

        // IDA @ 0x81C0BB3 performs the same complete copy for equipment[4].
        Inven_Item& equipment1 = payload->equipment[4];
        equipment1.seal_or_state_ = inven1.seal_or_state_;
        equipment1.item_category_ = inven1.item_category_;
        equipment1.item_id_ = inven1.item_id_;
        equipment1.item_attr_ = inven1.item_attr_;
        equipment1.add_info_ = inven1.add_info_;
        equipment1.avatar_select_ = inven1.avatar_select_;
        equipment1.extend_info_ = inven1.extend_info_;
        equipment1.amplify_option_.ability_type_and_identification_ =
            inven1.amplify_option_.ability_type_and_identification_;
        equipment1.amplify_option_.ability_value_ =
            inven1.amplify_option_.ability_value_;
        equipment1.amplify_option_.lock_ = inven1.amplify_option_.lock_;
        equipment1.globally_unique_id_.server_id_ =
            inven1.globally_unique_id_.server_id_;
        equipment1.globally_unique_id_.datetime_ =
            inven1.globally_unique_id_.datetime_;
        equipment1.globally_unique_id_.sequence_no_ =
            inven1.globally_unique_id_.sequence_no_;
        equipment1.chronicle_random_option_.enchanted_skill_options_[0].flags_ =
            inven1.chronicle_random_option_.enchanted_skill_options_[0].flags_;
        equipment1.chronicle_random_option_.enchanted_skill_options_[1].flags_ =
            inven1.chronicle_random_option_.enchanted_skill_options_[1].flags_;
        equipment1.chronicle_random_option_.value_ =
            inven1.chronicle_random_option_.value_;
        equipment1.chronicle_random_option_.random_option_.option_fields_[0]
            .option_index_ =
            inven1.chronicle_random_option_.random_option_.option_fields_[0]
                .option_index_;
        equipment1.chronicle_random_option_.random_option_.option_fields_[0]
            .first_value_ =
            inven1.chronicle_random_option_.random_option_.option_fields_[0]
                .first_value_;
        equipment1.chronicle_random_option_.random_option_.option_fields_[0]
            .second_value_ =
            inven1.chronicle_random_option_.random_option_.option_fields_[0]
                .second_value_;
        equipment1.chronicle_random_option_.random_option_.option_fields_[1]
            .option_index_ =
            inven1.chronicle_random_option_.random_option_.option_fields_[1]
                .option_index_;
        equipment1.chronicle_random_option_.random_option_.option_fields_[1]
            .first_value_ =
            inven1.chronicle_random_option_.random_option_.option_fields_[1]
                .first_value_;
        equipment1.chronicle_random_option_.random_option_.option_fields_[1]
            .second_value_ =
            inven1.chronicle_random_option_.random_option_.option_fields_[1]
                .second_value_;
        equipment1.chronicle_random_option_.random_option_.option_fields_[2]
            .option_index_ =
            inven1.chronicle_random_option_.random_option_.option_fields_[2]
                .option_index_;
        equipment1.chronicle_random_option_.random_option_.option_fields_[2]
            .first_value_ =
            inven1.chronicle_random_option_.random_option_.option_fields_[2]
                .first_value_;
        equipment1.chronicle_random_option_.random_option_.option_fields_[2]
            .second_value_ =
            inven1.chronicle_random_option_.random_option_.option_fields_[2]
                .second_value_;
        equipment1.chronicle_random_option_.random_option_.initial_seed_.seed_ =
            inven1.chronicle_random_option_.random_option_.initial_seed_.seed_;
        equipment1.chronicle_random_option_.random_option_.changed_option_
            .option_index_ =
            inven1.chronicle_random_option_.random_option_.changed_option_
                .option_index_;
        equipment1.chronicle_random_option_.random_option_.changed_option_
            .first_value_ =
            inven1.chronicle_random_option_.random_option_.changed_option_
                .first_value_;
        equipment1.chronicle_random_option_.random_option_.changed_option_
            .second_value_ =
            inven1.chronicle_random_option_.random_option_.changed_option_
                .second_value_;
        equipment1.chronicle_random_option_.random_option_.changed_option_seed_
            .seed_ =
            inven1.chronicle_random_option_.random_option_.changed_option_seed_
                .seed_;
        equipment1.upgrade_separate_.bits_ = inven1.upgrade_separate_.bits_;
        equipment1.reserved_capacity_.value_0_ =
            inven1.reserved_capacity_.value_0_;
        equipment1.reserved_capacity_.value_1_ =
            inven1.reserved_capacity_.value_1_;
        equipment1.reserved_capacity_.value_2_ =
            inven1.reserved_capacity_.value_2_;
    } else {
        payload->has_equipment = 0;
    }

    if (!inventory_gift) {
        return reinterpret_cast<int>(payload);
    }

    CItem* inventory_item = G_CDataManager()->find_item(inventory_gift);
    if (inventory_item) {
        payload->has_inventory = 1;
        Inven_Item inven;
        MakeInvenItemFromItem(inventory_item, inventory_gift, inven);

        // IDA @ 0x81C0D32 copies the entire temporary into equipment[0].
        Inven_Item& inventory_destination = payload->equipment[0];
        inventory_destination.seal_or_state_ = inven.seal_or_state_;
        inventory_destination.item_category_ = inven.item_category_;
        inventory_destination.item_id_ = inven.item_id_;
        inventory_destination.item_attr_ = inven.item_attr_;
        inventory_destination.add_info_ = inven.add_info_;
        inventory_destination.avatar_select_ = inven.avatar_select_;
        inventory_destination.extend_info_ = inven.extend_info_;
        inventory_destination.amplify_option_.ability_type_and_identification_ =
            inven.amplify_option_.ability_type_and_identification_;
        inventory_destination.amplify_option_.ability_value_ =
            inven.amplify_option_.ability_value_;
        inventory_destination.amplify_option_.lock_ = inven.amplify_option_.lock_;
        inventory_destination.globally_unique_id_.server_id_ =
            inven.globally_unique_id_.server_id_;
        inventory_destination.globally_unique_id_.datetime_ =
            inven.globally_unique_id_.datetime_;
        inventory_destination.globally_unique_id_.sequence_no_ =
            inven.globally_unique_id_.sequence_no_;
        inventory_destination.chronicle_random_option_
            .enchanted_skill_options_[0].flags_ =
            inven.chronicle_random_option_.enchanted_skill_options_[0].flags_;
        inventory_destination.chronicle_random_option_
            .enchanted_skill_options_[1].flags_ =
            inven.chronicle_random_option_.enchanted_skill_options_[1].flags_;
        inventory_destination.chronicle_random_option_.value_ =
            inven.chronicle_random_option_.value_;
        inventory_destination.chronicle_random_option_.random_option_
            .option_fields_[0].option_index_ =
            inven.chronicle_random_option_.random_option_.option_fields_[0]
                .option_index_;
        inventory_destination.chronicle_random_option_.random_option_
            .option_fields_[0].first_value_ =
            inven.chronicle_random_option_.random_option_.option_fields_[0]
                .first_value_;
        inventory_destination.chronicle_random_option_.random_option_
            .option_fields_[0].second_value_ =
            inven.chronicle_random_option_.random_option_.option_fields_[0]
                .second_value_;
        inventory_destination.chronicle_random_option_.random_option_
            .option_fields_[1].option_index_ =
            inven.chronicle_random_option_.random_option_.option_fields_[1]
                .option_index_;
        inventory_destination.chronicle_random_option_.random_option_
            .option_fields_[1].first_value_ =
            inven.chronicle_random_option_.random_option_.option_fields_[1]
                .first_value_;
        inventory_destination.chronicle_random_option_.random_option_
            .option_fields_[1].second_value_ =
            inven.chronicle_random_option_.random_option_.option_fields_[1]
                .second_value_;
        inventory_destination.chronicle_random_option_.random_option_
            .option_fields_[2].option_index_ =
            inven.chronicle_random_option_.random_option_.option_fields_[2]
                .option_index_;
        inventory_destination.chronicle_random_option_.random_option_
            .option_fields_[2].first_value_ =
            inven.chronicle_random_option_.random_option_.option_fields_[2]
                .first_value_;
        inventory_destination.chronicle_random_option_.random_option_
            .option_fields_[2].second_value_ =
            inven.chronicle_random_option_.random_option_.option_fields_[2]
                .second_value_;
        inventory_destination.chronicle_random_option_.random_option_
            .initial_seed_.seed_ =
            inven.chronicle_random_option_.random_option_.initial_seed_.seed_;
        inventory_destination.chronicle_random_option_.random_option_
            .changed_option_.option_index_ =
            inven.chronicle_random_option_.random_option_.changed_option_
                .option_index_;
        inventory_destination.chronicle_random_option_.random_option_
            .changed_option_.first_value_ =
            inven.chronicle_random_option_.random_option_.changed_option_
                .first_value_;
        inventory_destination.chronicle_random_option_.random_option_
            .changed_option_.second_value_ =
            inven.chronicle_random_option_.random_option_.changed_option_
                .second_value_;
        inventory_destination.chronicle_random_option_.random_option_
            .changed_option_seed_.seed_ =
            inven.chronicle_random_option_.random_option_.changed_option_seed_
                .seed_;
        inventory_destination.upgrade_separate_.bits_ = inven.upgrade_separate_.bits_;
        inventory_destination.reserved_capacity_.value_0_ =
            inven.reserved_capacity_.value_0_;
        inventory_destination.reserved_capacity_.value_1_ =
            inven.reserved_capacity_.value_1_;
        inventory_destination.reserved_capacity_.value_2_ =
            inven.reserved_capacity_.value_2_;
    } else {
        payload->has_inventory = 0;
    }
    return reinterpret_cast<int>(payload);
}

// IDA_EA: 0x82320C8
// IDA_NAME: _ZN23DisPatcher_CreateCharac11check_stateEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_CreateCharac::check_state(CUser *,PacketBuf &)
int DisPatcher_CreateCharac::check_state(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() == 2) {
        return 0;
    }
    return LineFunc(325,
                    "virtual int DisPatcher_CreateCharac::check_state(CUser*, PacketBuf&)",
                    0, 0);
}

// IDA_EA: 0x8595134
// IDA_NAME: _ZN23DisPatcher_DeleteCharacC2Ev
// DEMANGLED: DisPatcher_DeleteCharac::DisPatcher_DeleteCharac(void)
DisPatcher_DeleteCharac::DisPatcher_DeleteCharac()
    : IPacketDispatcher<MSG_DELETE_CHARACTER,
                        stDeleteCharac,
                        static_cast<ch_state>(2)>() {}

// IDA_EA: 0x823215E
// IDA_NAME: _ZN23DisPatcher_DeleteCharacD2Ev
// DEMANGLED: DisPatcher_DeleteCharac::~DisPatcher_DeleteCharac()
// IDA_EA: 0x823218E
// IDA_NAME: _ZN23DisPatcher_DeleteCharacD0Ev
// DEMANGLED: DisPatcher_DeleteCharac::~DisPatcher_DeleteCharac() deleting destructor
DisPatcher_DeleteCharac::~DisPatcher_DeleteCharac() {}

// IDA_EA: 0x81C1094
// IDA_NAME: _ZN23DisPatcher_DeleteCharac4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_DeleteCharac::read(PacketBuf &,MSG_BASE &)
int DisPatcher_DeleteCharac::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_DELETE_CHARACTER& remove =
        static_cast<MSG_DELETE_CHARACTER&>(message);
    if (packet.get_byte(remove.character_slot) != 1) {
        return LineFunc(2598,
                        "virtual int DisPatcher_DeleteCharac::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }

    if (packet.get_int(remove.character_name_length) != 1) {
        return LineFunc(2601,
                        "virtual int DisPatcher_DeleteCharac::read(PacketBuf&, MSG_BASE&)",
                        0, 0);
    }
    if (packet.get_str(remove.character_name,
                       sizeof(remove.character_name),
                       static_cast<int>(remove.character_name_length)) == 1) {
        return 0;
    }
    return LineFunc(2608,
                    "virtual int DisPatcher_DeleteCharac::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81C1182
// IDA_NAME: _ZN23DisPatcher_DeleteCharac7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_DeleteCharac::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_DeleteCharac::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    MSG_DELETE_CHARACTER& remove =
        static_cast<MSG_DELETE_CHARACTER&>(message);
    stDeleteCharac& delete_param = static_cast<stDeleteCharac&>(param);
    delete_param.result = check_error(user, message);
    const int result = delete_param.result;
    if (result > 0) {
        return 0;
    }
    if (result < 0) {
        return LineFunc(
            2625,
            "virtual int DisPatcher_DeleteCharac::process(CUser*, MSG_BASE&, ParamBase&)",
            result,
            static_cast<unsigned int>(user->get_acc_id()));
    }

    const int slot = static_cast<signed char>(remove.character_slot);
    const unsigned int charac_no =
        static_cast<unsigned int>(user->get_charac_no( slot));
    user->RemoveCharacMercenaryInfo(charac_no);

    Stream* stream =
        GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                           2637);
    CStreamGuard guard(stream, true);
    *guard << 5;
    *guard << user->GetUID();
    SIG_DELETE_CHARAC* const payload =
        guard.GetInBuffer<SIG_DELETE_CHARAC>();
    std::memset(payload, 0, sizeof(*payload));
    payload->account_id = static_cast<uint32_t>(user->get_acc_id());
    payload->charac_no =
        static_cast<uint32_t>(user->get_charac_no(slot));

    if (!payload->charac_no) {
        delete_param.result = 21;
        if (GlobalData::stream_pool()) {
            GlobalData::stream_pool()->Free(guard);
        }
        return 0;
    }

    payload->guild_id = static_cast<uint32_t>(
        user->GetGuildIdBeforeDeleteCharacter(slot));
    if (static_cast<unsigned char>(user->lockDeleteCharac()) == 1u) {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
        const char* acc_id_text = NumberToString(
            static_cast<unsigned int>(user->get_acc_id()),
            0);
        cMyTrace trace(
            "virtual int DisPatcher_DeleteCharac::process(CUser*, MSG_BASE&, ParamBase&)",
            2670,
            0);
        trace("[%s][%d] accid(%s) delete charac(%d) %s",
              "virtual int DisPatcher_DeleteCharac::process(CUser*, MSG_BASE&, ParamBase&)",
              slot,
              acc_id_text,
              charac_no,
              user->get_charac_name( slot));
        return 0;
    }

    if (GlobalData::stream_pool()) {
        GlobalData::stream_pool()->Free(guard);
    }
    return LineFunc(
        2664,
        "virtual int DisPatcher_DeleteCharac::process(CUser*, MSG_BASE&, ParamBase&)",
        0,
        static_cast<unsigned int>(user->get_acc_id()));
}

// IDA_EA: 0x81C14B8
// IDA_NAME: _ZN23DisPatcher_DeleteCharac4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_DeleteCharac::send(CUser *,ParamBase &)
void DisPatcher_DeleteCharac::send(CUser* user, ParamBase& param) {
    const int result = static_cast<stDeleteCharac&>(param).result;
    if (result) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DELETE_CHARACTER,
                                 static_cast<unsigned char>(result));
    }
}

// IDA_EA: 0x81C14F0
// IDA_NAME: _ZN23DisPatcher_DeleteCharac11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_DeleteCharac::check_error(CUser *,MSG_BASE &)
int DisPatcher_DeleteCharac::check_error(CUser* user, MSG_BASE& message) {
    if (user->get_state() != 2) {
        return -1;
    }
    if (static_cast<unsigned char>(user->isLockedCreateCharac())) {
        return 19;
    }

    MSG_DELETE_CHARACTER& remove =
        static_cast<MSG_DELETE_CHARACTER&>(message);
    const int slot = static_cast<signed char>(remove.character_slot);
    if (slot >= user->get_charac_count() || slot < 0) {
        return -2;
    }
    if (static_cast<unsigned char>(
            user->CheckDeleteCharacTime( slot)) != 1u) {
        return 22;
    }
    if (user->getCurCharacNo() &&
        user->getCurCharacNo() ==
            user->get_charac_no( slot)) {
        cMyTrace trace(
            "int DisPatcher_DeleteCharac::check_error(CUser*, MSG_BASE&)",
            2724,
            5);
        trace("pUser->getCurCharacNo() == recv_pk_del_charac.charac_index %d %d",
              remove.character_slot,
              user->get_charac_no( slot));
        return 22;
    }
    if (static_cast<unsigned char>(user->isLocked4DataLoad())) {
        return 19;
    }
    if (user->getCurCharacR()) {
        return -3;
    }

    if (slot < 0 ||
        user->characters_.size() <= static_cast<std::size_t>(slot)) {
        return -2;
    }
    const _Charac_info& character =
        user->characters_[static_cast<std::size_t>(slot)];
    if (character.fields_.link_type_ > 1) {
        cMyTrace trace(
            "int DisPatcher_DeleteCharac::check_error(CUser*, MSG_BASE&)",
            2740,
            5);
        trace("LINK_LOG : ABNOMAL CHARAC DELETEING!!");
        return -4;
    }

    const char* name = user->get_charac_name( slot);
    if (!name || !remove.character_name[0]) {
        return -5;
    }
    if (std::strncmp(name,
                     remove.character_name,
                     sizeof(remove.character_name))) {
        return -6;
    }
    if (IsServiceRestricted(user, 1, 23)) {
        return 209;
    }

    const int secu_result = (::GlobalData::s_pSecuProtectionField)->Check(
        user,
        static_cast<SECURITY_PROTCTION>(46));
    return secu_result ? secu_result : 0;
}

// IDA_EA: 0x82321AC
// IDA_NAME: _ZN23DisPatcher_DeleteCharac11check_stateEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_DeleteCharac::check_state(CUser *,PacketBuf &)
int DisPatcher_DeleteCharac::check_state(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() == 2) {
        return 0;
    }
    return LineFunc(368,
                    "virtual int DisPatcher_DeleteCharac::check_state(CUser*, PacketBuf&)",
                    0, 0);
}

// IDA_EA: 0x859524C
// IDA_NAME: _ZN23DisPatcher_SelectCharacC2Ev
// DEMANGLED: DisPatcher_SelectCharac::DisPatcher_SelectCharac(void)
DisPatcher_SelectCharac::DisPatcher_SelectCharac()
    : IPacketDispatcher<MSG_CHARAC_SELECT,
                        stSelectCharac,
                        static_cast<ch_state>(2)>() {}

// IDA_EA: 0x8232596
// IDA_NAME: _ZN23DisPatcher_SelectCharacD2Ev
// DEMANGLED: DisPatcher_SelectCharac::~DisPatcher_SelectCharac()
// IDA_EA: 0x82325C6
// IDA_NAME: _ZN23DisPatcher_SelectCharacD0Ev
// DEMANGLED: DisPatcher_SelectCharac::~DisPatcher_SelectCharac() deleting destructor
DisPatcher_SelectCharac::~DisPatcher_SelectCharac() {}

// IDA_EA: 0x81C632A
// IDA_NAME: _ZN23DisPatcher_SelectCharac4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SelectCharac::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SelectCharac::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_CHARAC_SELECT& select = static_cast<MSG_CHARAC_SELECT&>(message);
    // CLIENT_IDA_EVIDENCE: sender sub_138B320 begins CMD4 at 0x138B472,
    // writes exactly one u8 character slot at 0x138B47F, then finalizes.
    const int index_before = packet.get_index();
    const int read_result = packet.get_byte(select.character_slot);
    LogManager::logFormat(
        1,
        "PacketDispatcher_Impl_1.cpp",
        "DisPatcher_SelectCharac::read",
        5480,
        "[CHARVIEW][SELECT_READ] result=%d index_before=%d index_after=%d len=%d slot=%d",
        read_result,
        index_before,
        packet.get_index(),
        packet.get_len(),
        read_result == 1 ? static_cast<int>(select.character_slot) : -1);
    if (read_result == 1) {
        return 0;
    }
    return LineFunc(5487,
                    "virtual int DisPatcher_SelectCharac::read(PacketBuf&, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81C6380
// IDA_NAME: _ZN23DisPatcher_SelectCharac7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SelectCharac::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SelectCharac::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    MSG_CHARAC_SELECT& select = static_cast<MSG_CHARAC_SELECT&>(message);
    stSelectCharac& select_param = static_cast<stSelectCharac&>(param);
    select_param.result = check_error(user, message);
    const int result = select_param.result;
    LogManager::logFormat(
        1,
        "PacketDispatcher_Impl_1.cpp",
        "DisPatcher_SelectCharac::process",
        5505,
        "[CHARVIEW][SELECT_CHECK] result=%d state=%d lock=%d section1=%d section11=%d slot=%d",
        result,
        user->get_state(),
        user->isLocked4DataLoad(),
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1)) ? 1 : 0,
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(11)) ? 1 : 0,
        static_cast<int>(static_cast<signed char>(select.character_slot)));
    if (result > 0) {
        return 0;
    }
    if (result < 0) {
        return LineFunc(
            5511,
            "virtual int DisPatcher_SelectCharac::process(CUser*, MSG_BASE&, ParamBase&)",
            result,
            static_cast<unsigned int>(user->get_acc_id()));
    }

    const int slot = static_cast<signed char>(select.character_slot);

    user->resetSaveFlags();
    // Signal 32 is queued by Inter_LoadInven only after signal 8 has installed
    // the full inventory.  Mark ETC incomplete now so no stale cache-section
    // bit can make a partial selection appear complete.
    user->DisableCharacInfo(static_cast<CACHE_SECTION>(11));
    const int selected_charac_no = user->get_charac_no(slot);
    const int last_login_charac_no = user->GetLastLoginCharacNo();
    const int last_login_channel_no = user->GetLastLoginChannelNo();
    const int current_channel_no = G_CEnvironment()->get_channel_no();
    const bool matches_last_character =
        selected_charac_no == last_login_charac_no &&
        last_login_charac_no != 0 && last_login_channel_no != 0;
    const bool same_channel_reselect =
        matches_last_character && last_login_channel_no == current_channel_no;

    bool use_exchange = false;
    if (user->GetCharacViewState(slot) == 1) {
        user->SetCharacViewState(slot, 0);
    } else {
        use_exchange = matches_last_character && !same_channel_reselect;
    }

    GameWorld* world = G_GameWorld();
    const bool zombie_authoritative_load =
        world->GetChannelType() ==
        channel_zombie_matching_producer::kZombieChannelType;
    if (world->GetChannelType() == 7 ||
        world->IsIntegratedPvPBaseChannel() || zombie_authoritative_load) {
        // These channels already require an authoritative DB load.  Keep the
        // rental-info source aligned with the rest of the character sections.
        use_exchange = false;
    }

    user->SetValidLastLoginCharac(use_exchange);
    user->SetLoadRentalInfoFromExchange(use_exchange);

    if (same_channel_reselect) {
        // CMD7 queues UpdateData before the player can issue this CMD4.
        // Route every section through DB_Q so the queued quest save (signal
        // 41) is consumed before the new quest load (signal 10).  The failing
        // same-character path otherwise publishes the town quest list from a
        // stale exchange snapshot; cross-character DB reloads preserve it.
        LogManager::logFormat(
            1,
            "quest_reselect_diag.cpp",
            "DisPatcher_SelectCharac::process",
            0,
            "[QUEST_RESELECT][FORCE_DB] uid=%d account=%d charac_no=%d last_channel=%d current_channel=%d",
            user->GetUID(),
            user->get_acc_id(),
            selected_charac_no,
            last_login_channel_no,
            current_channel_no);
    }

    exchange_server::CExchangeServerHitRate* hit_rate =
        exchange_server::GetInstanceExchangeServerHitRate();
    if (user->IsValidLastLoginCharac()) {
        hit_rate->IncrementCacheLoadCount();
    } else {
        hit_rate->IncrementDBLoadCount();
    }

    const _Charac_info& character =
        user->characters_[static_cast<std::size_t>(slot)];
    if (world->IsIntegratedPvPBaseChannel()) {
        select.tag_character_no = 0;
        select.striker_skill_index = 0;
    } else {
        select.tag_character_no =
            static_cast<uint32_t>(character.fields_.tag_character_no_);
        select.striker_skill_index =
            character.fields_.striker_skill_index_;
    }

    // Queue SQL004 before the ordinary character-load fan-out.  DB_Q FIFO
    // normally installs the sidecar before make_basic_info; a late response
    // is still safe because packet construction returns zero on loading/miss.
    const uint32_t clone_title_charac_no = static_cast<uint32_t>(
        user->get_charac_no(slot));
    (void)clone_title::RequestLoad(user, clone_title_charac_no);
    if (select.tag_character_no) {
        (void)clone_title::RequestLoad(user, select.tag_character_no);
    }

    LogManager::logFormat(
        1,
        "PacketDispatcher_Impl_1.cpp",
        "DisPatcher_SelectCharac::process",
        5570,
        "[CHARVIEW][SELECT_FANOUT_BEGIN] state=%d lock=%d section1=%d section11=%d cache=%d",
        user->get_state(),
        user->isLocked4DataLoad(),
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1)) ? 1 : 0,
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(11)) ? 1 : 0,
        user->IsValidLastLoginCharac() ? 1 : 0);
    const int charac_request_result = RequestDBCharac(user, select);
    RequestBlackCount(user, select);
    const int inventory_request_result = RequestDBInventory(user, select);
    LogManager::logFormat(
        1,
        "PacketDispatcher_Impl_1.cpp",
        "DisPatcher_SelectCharac::process",
        5580,
        "[CHARVIEW][SELECT_FANOUT_RESULT] charac=%d inventory=%d lock=%d section1=%d section11=%d",
        charac_request_result,
        inventory_request_result,
        user->isLocked4DataLoad(),
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1)) ? 1 : 0,
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(11)) ? 1 : 0);
    RequestDBQuest(user, select);
    RequestDBDungeonClear(user, select);
    APSystem::CDBAndExchangeServerProc::RequestLoadActionPointFromDBAndES(*user, select);
    RequestDBKillMonsterInfo(user, select);
    advancealtar::DBAndExchangeServerProc::requestLoadFromDBAndES(*user,
                                                                    select);
    // RequestDBETC is intentionally deferred until Inter_LoadInven.  Signal 8
    // crosses the native SQL006 continuation queue, so issuing signal 32 here
    // violates the legacy DB_Q FIFO completion order.

    user->SetValidLastLoginCharac( false);
    user->SetLastLoginCharacNo( 0);
    user->SetLastLoginChannelNo( 0);

    const unsigned int charac_no =
        static_cast<unsigned int>(user->get_charac_no( slot));
    DB_GetDeathTowerBestRecord::makeRequest(user->GetUID(), charac_no);

    const int cur_sec = GlobalData::system_time()
                            ? GlobalData::system_time()->getCurSec()
                            : 0;
    const int account_last_play_time = user->GetAccountLastPlayTime();
    if (CheckDailyScheduleTime(G_CEnvironment()->get_fatigue_time(),
                               account_last_play_time,
                               cur_sec)) {
        DB_GetUserRegdate::makeRequest(
            user->GetUID(),
            static_cast<unsigned int>(user->get_acc_id()));
    }

    LimitNpcBuyItemInfoAll monitor_packet;
    monitor_packet.user_id = static_cast<uint32_t>(user->GetUID());
    monitor_packet.account_id =
        static_cast<uint32_t>(user->get_acc_id());
    monitor_packet.charac_no =
        static_cast<uint32_t>(user->get_charac_no(slot));
    ::CMonitorServerProxy* proxy =
        ::GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendTcpPacket(
        reinterpret_cast<char*>(&monitor_packet),
        0x182);
    return 0;
}

// IDA_EA: 0x81C6826
// IDA_NAME: _ZN23DisPatcher_SelectCharac4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_SelectCharac::send(CUser *,ParamBase &)
void DisPatcher_SelectCharac::send(CUser* user, ParamBase& param) {
    const int result = static_cast<stSelectCharac&>(param).result;
    if (result != 0x7FFFFFFF) {
        if (result) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_SELECT_CHARACTER,
                                     static_cast<unsigned char>(result));
        }
    }
}

// IDA_EA: 0x81C686E
// IDA_NAME: _ZN23DisPatcher_SelectCharac11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_SelectCharac::check_error(CUser *,MSG_BASE &)
int DisPatcher_SelectCharac::check_error(CUser* user, MSG_BASE& message) {
    MSG_CHARAC_SELECT& select = static_cast<MSG_CHARAC_SELECT&>(message);
    unsigned int reject_mask = 0u;
    const int state = user->get_state();
    const int create_lock =
        static_cast<unsigned char>(user->isLockedCreateCharac());
    const int human =
        static_cast<unsigned char>(user->isHumanCertified());
    int pvp = -1;
    int slot_value = static_cast<int>(select.character_slot);
    int slot_valid = -1;
    int check_select = -1;
    int load_before = -1;
    int load_acquire = -1;
    const auto finish = [&](int result) -> int {
        LogManager::logFormat(
            1,
            "PacketDispatcher_Impl_1.cpp",
            "DisPatcher_SelectCharac::check_error",
            5800,
            "[CHARVIEW][SELECT_CHECK_DETAIL] result=%d mask=0x%X state=%d create_lock=%d human=%d pvp=%d slot=%d slot_valid=%d check_select=%d load_before=%d load_acquire=%d",
            result,
            reject_mask,
            state,
            create_lock,
            human,
            pvp,
            slot_value,
            slot_valid,
            check_select,
            load_before,
            load_acquire);
        return result;
    };
    if (state != 2) {
        reject_mask |= 1u << 0;
        return finish(0x7FFFFFFF);
    }
    if (create_lock) {
        reject_mask |= 1u << 1;
        return finish(19);
    }
    if (human != 1) {
        reject_mask |= 1u << 2;
        return finish(0x7FFFFFFF);
    }

    if (G_GameWorld()->GetChannelType() == 14) {
        select.character_slot = static_cast<uint8_t>(user->get_charac_idx(
            user->getOnlinePreliminaryCharacNo()));
        slot_value = static_cast<int>(select.character_slot);
        if (select.character_slot == 0xFFu) {
            reject_mask |= 1u << 4;
            return finish(-1);
        }
    }

    pvp = static_cast<unsigned char>(checkPvpEnterCharacter(user, select));
    if (pvp != 1) {
        reject_mask |= 1u << 3;
        return finish(-1);
    }

    const unsigned char slot = select.character_slot;
    slot_value = static_cast<int>(slot);
    if (slot >= 0x24u) {
        cMyTrace trace(
            "int DisPatcher_SelectCharac::check_error(CUser*, MSG_BASE&)",
            5828,
            5);
        trace("pUser->SelectCharac() fail, %d out of range",
              select.character_slot);
        reject_mask |= 1u << 4;
        return finish(4);
    }
    if (user->characters_.size() <= static_cast<std::size_t>(slot)) {
        reject_mask |= 1u << 5;
        return finish(-2);
    }
    slot_valid = 1;

    const _Charac_info& character =
        user->characters_[static_cast<std::size_t>(slot)];
    std::string name(character.fields_.character_name_);
    CSyncSlangFilter* filter = CSyncSlangFilter::GetInstance();
    if (filter->HasSlangName(name) ||
        CSyncSlangFilter::GetInstance()->HasSlang(name)) {
        reject_mask |= 1u << 9;
        return finish(159);
    }

    G_CDataManager()->setCharacNameFilterPass(IsGmAccount(user));
    const char* raw_name = character.fields_.character_name_;
    if (G_CDataManager()->hasPreventPrefixCharacName(
            raw_name,
            static_cast<int>(std::strlen(raw_name)))) {
        reject_mask |= 1u << 10;
        return finish(8);
    }
    check_select =
        static_cast<unsigned char>(user->checkSelectCharac(slot));
    if (check_select != 1) {
        reject_mask |= 1u << 6;
        return finish(-3);
    }
    if (static_cast<unsigned char>(user->isLockedDeleteCharac())) {
        const int charac_no = character.fields_.character_no_;
        cMyTrace trace(
            "int DisPatcher_SelectCharac::check_error(CUser*, MSG_BASE&)",
            5916,
            0);
        trace("select_charac error during delete charac_no(%d)",
              charac_no);
        reject_mask |= 1u << 7;
        return finish(0x7FFFFFFF);
    }
    load_before =
        static_cast<unsigned char>(user->isLocked4DataLoad());
    load_acquire =
        static_cast<unsigned char>(user->lock4DataLoad());
    if (load_acquire == 1) {
        return finish(0);
    }
    reject_mask |= 1u << 8;
    return finish(-4);
}

// IDA_EA: 0x81C6C22
// IDA_NAME: _ZN23DisPatcher_SelectCharac22checkPvpEnterCharacterEP5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::checkPvpEnterCharacter(CUser *,MSG_CHARAC_SELECT &)
int DisPatcher_SelectCharac::checkPvpEnterCharacter(
    CUser* user,
    MSG_CHARAC_SELECT& message) {
    if (!user) {
        return 0;
    }
    if (!G_GameWorld()->IsPVPChannel()) {
        return 1;
    }
    if (static_cast<unsigned char>(
            user->GetCharacJob(message.character_slot)) == 9u) {
        return 0;
    }
    return static_cast<unsigned char>(
               user->GetCharacJob(message.character_slot)) != 10u;
}

// IDA_EA: 0x81C6CA0
// IDA_NAME: _ZN23DisPatcher_SelectCharac15RequestDBCharacEP5CUserRK17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::RequestDBCharac(CUser *,MSG_CHARAC_SELECT const&)
int DisPatcher_SelectCharac::RequestDBCharac(
    CUser* user,
    const MSG_CHARAC_SELECT& message) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                           5949);
    CStreamGuard guard(stream, true);
    *guard << 7;
    *guard << user->GetUID();
    user->DisableCharacInfo(static_cast<CACHE_SECTION>(0));
    if (user->IsValidLastLoginCharac()) {
        *guard << user->GetLastLoginChannelNo();
        *guard << user->GetLastLoginCharacNo();
    }

    SIG_LOAD_CHARAC* payload = guard.GetInBuffer<SIG_LOAD_CHARAC>();
    payload->charac_data.Reset();
    payload->member_id = static_cast<uint32_t>(user->get_acc_id());
    payload->character_no = static_cast<uint32_t>(
        user->get_charac_no(message.character_slot));
    payload->charac_data.school_id =
        static_cast<uint32_t>(user->GetSchoolNo());
    payload->tag_charac_data.clear();
    payload->charac_data.tag_character_no = message.tag_character_no;
    payload->charac_data.striker_skill_index = message.striker_skill_index;
    user->LoadCharacTypeHotKey(
        user->get_character_job( static_cast<unsigned int>(user->GetLastLoginCharacNo())),
        user->get_character_job(payload->character_no));

    const bool use_exchange = user->IsValidLastLoginCharac() != 0;
    const MsgQueueMgr::QUEUE_IDX queue =
        use_exchange ? static_cast<MsgQueueMgr::QUEUE_IDX>(5)
                     : MsgQueueMgr::DB_Q;
    const int queue_result = GlobalData::msgq_mgr()->put(queue, guard);
    LogManager::logFormat(
        1,
        "PacketDispatcher_Impl_1.cpp",
        "DisPatcher_SelectCharac::RequestDBCharac",
        6000,
        "[CHARVIEW][SELECT_SIGNAL7_PUT] result=%d queue=%d valid_last=%d lock=%d section1=%d section11=%d",
        queue_result,
        static_cast<int>(queue),
        use_exchange ? 1 : 0,
        user->isLocked4DataLoad(),
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1)) ? 1 : 0,
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(11)) ? 1 : 0);
    return 1;
}

// IDA_EA: 0x81C6EE6
// IDA_NAME: _ZN23DisPatcher_SelectCharac18RequestDBInventoryEP5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::RequestDBInventory(CUser *,MSG_CHARAC_SELECT &)
int DisPatcher_SelectCharac::RequestDBInventory(CUser* user,
                                                MSG_CHARAC_SELECT& message) {
    return RequestDBInventory_forchina(user, message);
}

// IDA_EA: 0x81C6F08
// IDA_NAME: _ZN23DisPatcher_SelectCharac14RequestDBSkillEP5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::RequestDBSkill(CUser *,MSG_CHARAC_SELECT &)
int DisPatcher_SelectCharac::RequestDBSkill(CUser* user,
                                            MSG_CHARAC_SELECT& message) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                           6049);
    CStreamGuard guard(stream, true);
    *guard << 9;
    *guard << user->GetUID();
    user->DisableCharacInfo(static_cast<CACHE_SECTION>(2));
    if (user->IsValidLastLoginCharac()) {
        *guard << user->GetLastLoginChannelNo();
        *guard << user->GetLastLoginCharacNo();
    }

    SIG_LOAD_SKILL* const payload = guard.GetInBuffer<SIG_LOAD_SKILL>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(
        user->get_charac_no(message.character_slot));

    if (user->IsValidLastLoginCharac()) {
        GlobalData::msgq_mgr()->put(static_cast<MsgQueueMgr::QUEUE_IDX>(5),
                                    guard);
    } else {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
    return 1;
}

// IDA_EA: 0x81C70C2
// IDA_NAME: _ZN23DisPatcher_SelectCharac14RequestDBQuestEP5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::RequestDBQuest(CUser *,MSG_CHARAC_SELECT &)
int DisPatcher_SelectCharac::RequestDBQuest(CUser* user,
                                            MSG_CHARAC_SELECT& message) {
    Stream* stream =
        GlobalData::big_stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                               6077);
    CStreamGuard guard(stream, true);
    *guard << 10;
    *guard << user->GetUID();
    user->DisableCharacInfo(static_cast<CACHE_SECTION>(3));
    if (user->IsValidLastLoginCharac()) {
        *guard << user->GetLastLoginChannelNo();
        *guard << user->GetLastLoginCharacNo();
    }

    SIG_LOAD_QUEST* const payload = guard.GetInBuffer<SIG_LOAD_QUEST>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(
        user->get_charac_no(message.character_slot));

    if (user->IsValidLastLoginCharac()) {
        GlobalData::msgq_mgr()->put(static_cast<MsgQueueMgr::QUEUE_IDX>(5),
                                    guard);
    } else {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
    return 1;
}

// IDA_EA: 0x81C727C
// IDA_NAME: _ZN23DisPatcher_SelectCharac15RequestDBTicketEP5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::RequestDBTicket(CUser *,MSG_CHARAC_SELECT &)
int DisPatcher_SelectCharac::RequestDBTicket(CUser* user,
                                             MSG_CHARAC_SELECT& message) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                           6107);
    CStreamGuard guard(stream, true);
    *guard << 11;
    *guard << user->GetUID();
    user->DisableCharacInfo(static_cast<CACHE_SECTION>(4));
    if (user->IsValidLastLoginCharac()) {
        *guard << user->GetLastLoginChannelNo();
        *guard << user->GetLastLoginCharacNo();
    }

    SIG_LOAD_TICKET* const payload = guard.GetInBuffer<SIG_LOAD_TICKET>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(
        user->get_charac_no(message.character_slot));

    if (user->IsValidLastLoginCharac()) {
        GlobalData::msgq_mgr()->put(static_cast<MsgQueueMgr::QUEUE_IDX>(5),
                                    guard);
    } else {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
    return 1;
}

// IDA_EA: 0x81C7436
// IDA_NAME: _ZN23DisPatcher_SelectCharac21RequestDBDungeonClearEP5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::RequestDBDungeonClear(CUser *,MSG_CHARAC_SELECT &)
int DisPatcher_SelectCharac::RequestDBDungeonClear(
    CUser* user,
    MSG_CHARAC_SELECT& message) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                           6136);
    CStreamGuard guard(stream, true);
    *guard << 12;
    *guard << user->GetUID();
    user->DisableCharacInfo(static_cast<CACHE_SECTION>(5));
    if (user->IsValidLastLoginCharac()) {
        *guard << user->GetLastLoginChannelNo();
        *guard << user->GetLastLoginCharacNo();
    }

    SIG_LOAD_DUNGEON_CLEAR* const payload =
        guard.GetInBuffer<SIG_LOAD_DUNGEON_CLEAR>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(
        user->get_charac_no(message.character_slot));
    payload->account_id = static_cast<uint32_t>(user->get_acc_id());

    if (user->IsValidLastLoginCharac()) {
        GlobalData::msgq_mgr()->put(static_cast<MsgQueueMgr::QUEUE_IDX>(5),
                                    guard);
    } else {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
    return 1;
}

// IDA_EA: 0x81C7602
// IDA_NAME: _ZN23DisPatcher_SelectCharac12RequestDBETCEP5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::RequestDBETC(CUser *,MSG_CHARAC_SELECT &)
int DisPatcher_SelectCharac::RequestDBETC(CUser* user,
                                          MSG_CHARAC_SELECT& message) {
    const bool use_exchange = user->IsValidLastLoginCharac() != 0;
    return QueueLoadEtcRequest(
        user,
        static_cast<uint32_t>(user->get_charac_no(message.character_slot)),
        message.tag_character_no,
        static_cast<uint32_t>(user->GetCharacJob(message.character_slot)),
        use_exchange,
        use_exchange ? user->GetLastLoginChannelNo() : 0,
        use_exchange ? user->GetLastLoginCharacNo() : 0);
}

// IDA_EA: 0x81C7836
// IDA_NAME: _ZN23DisPatcher_SelectCharac24RequestDBKillMonsterInfoEP5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::RequestDBKillMonsterInfo(CUser *,MSG_CHARAC_SELECT &)
int DisPatcher_SelectCharac::RequestDBKillMonsterInfo(
    CUser* user,
    MSG_CHARAC_SELECT& message) {
    Stream* stream =
        GlobalData::big_stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                               6211);
    CStreamGuard guard(stream, true);
    *guard << 0x282;
    *guard << user->GetUID();
    user->DisableCharacInfo(static_cast<CACHE_SECTION>(9));
    if (user->IsValidLastLoginCharac()) {
        *guard << user->GetLastLoginChannelNo();
        *guard << user->GetLastLoginCharacNo();
    }

    SIG_LOAD_KILL_MONSTER_INFO* const payload =
        guard.GetInBuffer<SIG_LOAD_KILL_MONSTER_INFO>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(
        user->get_charac_no(message.character_slot));

    if (user->IsValidLastLoginCharac()) {
        GlobalData::msgq_mgr()->put(static_cast<MsgQueueMgr::QUEUE_IDX>(5),
                                    guard);
    } else {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
    return 1;
}

// IDA_EA: 0x81C79F2
// IDA_NAME: _ZN23DisPatcher_SelectCharac17RequestBlackCountEP5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::RequestBlackCount(CUser *,MSG_CHARAC_SELECT &)
int DisPatcher_SelectCharac::RequestBlackCount(CUser* user,
                                               MSG_CHARAC_SELECT& message) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                           6242);
    CStreamGuard guard(stream, true);
    *guard << 179;
    *guard << user->GetUID();
    user->DisableCharacInfo(static_cast<CACHE_SECTION>(7));
    if (user->IsValidLastLoginCharac()) {
        *guard << user->GetLastLoginChannelNo();
        *guard << user->GetLastLoginCharacNo();
    }

    SIG_REQ_BLACK_COUNT* const payload =
        guard.GetInBuffer<SIG_REQ_BLACK_COUNT>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(
        user->get_charac_no(message.character_slot));

    if (user->IsValidLastLoginCharac()) {
        GlobalData::msgq_mgr()->put(static_cast<MsgQueueMgr::QUEUE_IDX>(5),
                                    guard);
    } else {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
    return 1;
}

// IDA_EA: 0x81C7BAC
// IDA_NAME: _ZN23DisPatcher_SelectCharac20RegistTimerSchedulesEP5CUser
// DEMANGLED: DisPatcher_SelectCharac::RegistTimerSchedules(CUser *)
int DisPatcher_SelectCharac::RegistTimerSchedules(CUser* user) {
    const int cur_charac_no = user->getCurCharacNo();
    const unsigned int unique_id =
        static_cast<unsigned short>(user->get_unique_id());
    const int uid = user->GetUID();
    TimerQueue* timer_queue = G_TimerQueue();
    timer_queue->InsertTimer(0, uid, 81, 0, unique_id, cur_charac_no);
    return 1;
}

// IDA_EA: 0x81C7C1C
// IDA_NAME: _ZN23DisPatcher_SelectCharac27RequestDBInventory_forchinaEP5CUserR17MSG_CHARAC_SELECT
// DEMANGLED: DisPatcher_SelectCharac::RequestDBInventory_forchina(CUser *,MSG_CHARAC_SELECT &)
int DisPatcher_SelectCharac::RequestDBInventory_forchina(
    CUser* user,
    MSG_CHARAC_SELECT& message) {
    Stream* stream =
        GlobalData::big_stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                               6319);
    CStreamGuard guard(stream, true);
    *guard << 8;
    *guard << user->GetUID();
    user->DisableCharacInfo(static_cast<CACHE_SECTION>(1));
    if (user->IsValidLastLoginCharac()) {
        *guard << user->GetLastLoginChannelNo();
        *guard << user->GetLastLoginCharacNo();
    }

    SIG_LOAD_INVENTORY* const payload =
        guard.GetInBuffer<SIG_LOAD_INVENTORY>();
    std::memset(payload, 0, sizeof(*payload));
    payload->account_id = static_cast<uint32_t>(user->get_acc_id());
    payload->charac_no = static_cast<uint32_t>(
        user->get_charac_no(message.character_slot));
    payload->tag_inventory.clear();
    payload->tag_charac_no = message.tag_character_no;
    // RECONSTRUCTED_0828: signal 8 remains on the legacy DB queue, but its
    // completed response is continued on the native SQL006 worker.  Carry
    // the explicit server group after the untouched official request ABI.
    *guard << user->GetServerGroup();

    const bool use_exchange = user->IsValidLastLoginCharac() != 0;
    const MsgQueueMgr::QUEUE_IDX queue =
        use_exchange ? static_cast<MsgQueueMgr::QUEUE_IDX>(5)
                     : MsgQueueMgr::DB_Q;
    const int queue_result = GlobalData::msgq_mgr()->put(queue, guard);
    LogManager::logFormat(
        1,
        "PacketDispatcher_Impl_1.cpp",
        "DisPatcher_SelectCharac::RequestDBInventory_forchina",
        6350,
        "[CHARVIEW][SELECT_SIGNAL8_PUT] result=%d queue=%d valid_last=%d lock=%d section1=%d section11=%d",
        queue_result,
        static_cast<int>(queue),
        use_exchange ? 1 : 0,
        user->isLocked4DataLoad(),
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1)) ? 1 : 0,
        user->IsCharacInfoLoaded(static_cast<CACHE_SECTION>(11)) ? 1 : 0);
    return 1;
}


// The old character-selection vtables refer to these exact state-2 template
// families.  Each explicit instantiation below owns one raw i386 provider.
// IDA_EA: 0x825A614 / 0x825A764 / 0x825AE7A / 0x825AF0E / 0x82363A2 /
//         0x82363D2 / 0x8597FF6
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CREATE_CHARACTER14stCreateCharacL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CREATE_CHARACTER14stCreateCharacL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CREATE_CHARACTER14stCreateCharacL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CREATE_CHARACTER14stCreateCharacL8ch_state2EE4sendEP5CUserR9ParamBase
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CREATE_CHARACTER14stCreateCharacL8ch_state2EED2Ev
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CREATE_CHARACTER14stCreateCharacL8ch_state2EED0Ev
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_CREATE_CHARACTER14stCreateCharacL8ch_state2EEC2Ev
template class IPacketDispatcher<MSG_CREATE_CHARACTER,
                                 stCreateCharac,
                                 static_cast<ch_state>(2)>;

// IDA_EA: 0x825A2CE / 0x825A41E / 0x825ADDA / 0x825AE6E / 0x823643E /
//         0x823646E / 0x859802E
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_DELETE_CHARACTER14stDeleteCharacL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_DELETE_CHARACTER14stDeleteCharacL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_DELETE_CHARACTER14stDeleteCharacL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_DELETE_CHARACTER14stDeleteCharacL8ch_state2EE4sendEP5CUserR9ParamBase
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_DELETE_CHARACTER14stDeleteCharacL8ch_state2EED2Ev
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_DELETE_CHARACTER14stDeleteCharacL8ch_state2EED0Ev
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_DELETE_CHARACTER14stDeleteCharacL8ch_state2EEC2Ev
template class IPacketDispatcher<MSG_DELETE_CHARACTER,
                                 stDeleteCharac,
                                 static_cast<ch_state>(2)>;

// IDA_EA: 0x8258B42 / 0x8258C9E / 0x8258CA8 / 0x825AD18 / 0x823674A /
//         0x823677A / 0x8598146
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_CHARAC_SELECT14stSelectCharacL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_CHARAC_SELECT14stSelectCharacL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_CHARAC_SELECT14stSelectCharacL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_CHARAC_SELECT14stSelectCharacL8ch_state2EE4sendEP5CUserR9ParamBase
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_CHARAC_SELECT14stSelectCharacL8ch_state2EED2Ev
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_CHARAC_SELECT14stSelectCharacL8ch_state2EED0Ev
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_CHARAC_SELECT14stSelectCharacL8ch_state2EEC2Ev
template class IPacketDispatcher<MSG_CHARAC_SELECT,
                                 stSelectCharac,
                                 static_cast<ch_state>(2)>;
