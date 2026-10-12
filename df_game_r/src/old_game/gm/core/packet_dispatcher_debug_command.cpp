#include "old_game/gm/core/packet_dispatcher_debug_command.h"
#include "infra/network/packet/PacketEnum.h"

#include <algorithm>
#include <ctime>
#include <cstring>
#include <stdint.h>
#include <vector>

#include "common/config.h"
#include "core/config/env/environment.h"
#include "old_game/gm/core/cgm_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/os_api.h"
#include "common/manager/system_time.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/player/core/packet_dispatcher_upgrade_guild_cargo.h"
#include "old_game/pvp/battlefield/assault_power_war_packet_abi.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

namespace {

const char kDispatchSignature[] =
    "virtual int DisPatcher_DebugCommand::dispatch_sig(CUser*, PacketBuf&)";

}  // namespace

// IDA_EA: 0x8233412
// IDA_NAME: _ZN23DisPatcher_DebugCommandC2Ev
// DEMANGLED: DisPatcher_DebugCommand::DisPatcher_DebugCommand(void)
DisPatcher_DebugCommand::DisPatcher_DebugCommand()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x823342E
// IDA_NAME: _ZN23DisPatcher_DebugCommandD2Ev
// DEMANGLED: DisPatcher_DebugCommand::~DisPatcher_DebugCommand()
// IDA_EA: 0x823345E
// IDA_NAME: _ZN23DisPatcher_DebugCommandD0Ev
// DEMANGLED: DisPatcher_DebugCommand::~DisPatcher_DebugCommand()
DisPatcher_DebugCommand::~DisPatcher_DebugCommand() {}

// IDA_EA: 0x81FC4DE
// IDA_NAME: _ZN23DisPatcher_DebugCommand12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_DebugCommand::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_DebugCommand::dispatch_sig(CUser* user, PacketBuf& packet) {
    const char* const raw_packet = packet.get_packet(0);
    const unsigned char command =
        raw_packet ? static_cast<unsigned char>(
                         raw_packet[PacketBuf::PACKET_INDEX_BEGIN])
                   : 0;
    CGM_Manager* const manager = GlobalData::s_GM_Manager;

    // The original admits command 0 to the GM audit branch and admits all
    // other commands only while the account's GM mode is active.  The owner
    // lifecycle supplies the manager before dispatch; retaining this null
    // guard keeps a partially initialized server from treating a missing
    // manager as a privilege grant.
    if (command != 0 &&
        (!manager || manager->GetCurrentGmMode(user->get_acc_id()) == 0)) {
        return LineFunc(35691, kDispatchSignature, 0, 0);
    }
    if (!user->getCurCharacR()) {
        return LineFunc(35696, kDispatchSignature, 0, 0);
    }
    if (!manager || !manager->IsGm(user->get_acc_id())) {
        return LineFunc(35729, kDispatchSignature, 0, 0);
    }

    // Hex-Rays shows the original local `v53` initialized to zero immediately
    // before `if (v53 == 1)`: the large mutation switch below it is therefore
    // intentionally unreachable in this server build.  Do not turn a
    // protocol-restoration task into a privilege escalation by enabling it.
    return 0;
}

// IDA_EA: 0x82333FE
// IDA_NAME: _ZN23DisPatcher_DebugCommand4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_DebugCommand::read(PacketBuf &,MSG_BASE &)
int DisPatcher_DebugCommand::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233408
// IDA_NAME: _ZN23DisPatcher_DebugCommand7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_DebugCommand::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_DebugCommand::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x858E724
// IDA_NAME: _ZN23DisPatcher_DebugCommand15_debugCommandOnEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandOn(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandOn(CUser* user,
                                              MSG_DEBUG_COMMAND& message) {
    (void)message;

    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandOn(CUser*, MSG_DEBUG_COMMAND&)",
        16,
        0);
    trace("%s:'%s' GM Mode ON.",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          user->getCurCharacName());

    user->SetGameMasterMode(true);
    if (!user->getCurCharacR()) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(0);
    packet.put_short(3);
    if (user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(1), 1, packet) != 1) {
        return 0;
    }
    if (user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(1), 0, packet) != 1) {
        return 0;
    }
    if (user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(1), 2, packet) != 1) {
        return 0;
    }
    packet.finalize(true);
    user->Send(packet);
    return 1;
}

// IDA_EA: 0x858E918
// IDA_NAME: _ZN23DisPatcher_DebugCommand16_debugCommandOffEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandOff(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandOff(CUser* user,
                                               MSG_DEBUG_COMMAND& message) {
    (void)message;

    if (user->isGMUser() != 1) {
        return 0;
    }

    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandOff(CUser*, MSG_DEBUG_COMMAND&)",
        65,
        0);
    trace("%s:'%s' GM Mode OFF.",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          user->getCurCharacName());
    user->SetGameMasterMode(false);
    return 1;
}

// IDA_EA: 0x858E9C4
// IDA_NAME: _ZN23DisPatcher_DebugCommand23_debugCommandClearSkillEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandClearSkill(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandClearSkill(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    (void)message;

    if (user->isGMUser() != 1) {
        return 0;
    }

    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandClearSkill(CUser*, MSG_DEBUG_COMMAND&)",
        138,
        0);
    trace("%s:'%s' GM Mode Clear Skill.",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          user->getCurCharacName());

    if (user->ClearSkills() == 0) {
        return 0;
    }
    user->send_skill_info();
    if (user->get_state() == 5) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                             static_cast<ENUM_NOTIPACKET>(2),
                             1);
    }
    return 1;
}

// IDA_EA: 0x858EAB8
// IDA_NAME: _ZN23DisPatcher_DebugCommand21_debugCommandSetSkillEP5CUserii
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandSetSkill(CUser *,int,int)
int DisPatcher_DebugCommand::_debugCommandSetSkill(CUser* user,
                                                    int skill_index,
                                                    int level) {
    if (user->isGMUser() != 1) {
        return 0;
    }
    if (user->SetSkillLevel(skill_index, level) == 0) {
        return 0;
    }

    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandSetSkill(CUser*, int, int)",
        162,
        0);
    trace("%s:'%s' GM Mode Set Skill(%d) Level(%d).",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          user->getCurCharacName(),
          skill_index,
          level);

    user->send_skill_info();
    if (user->get_state() == 5) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                             static_cast<ENUM_NOTIPACKET>(2),
                             1);
    }
    return 1;
}

// IDA_EA: 0x858EBCC
// IDA_NAME: _ZN23DisPatcher_DebugCommand27_debugCommandMasterNewSkillEP5CUserii
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandMasterNewSkill(CUser *,int,int)
int DisPatcher_DebugCommand::_debugCommandMasterNewSkill(CUser* user,
                                                          int skill_index,
                                                          int level) {
    if (user->isGMUser() != 1) {
        return 0;
    }

    stBuySkillInfo skill_info;
    skill_info.skill_tree_index = 0;
    skill_info.skill_index = static_cast<uint8_t>(skill_index);
    skill_info.skill_level_delta = static_cast<int8_t>(level);
    user->master_new_skill(skill_info, false);

    const char* const charac_name = user->getCurCharacName();
    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());
    const char* const account_id_text = NumberToString(account_id, 0);
    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandMasterNewSkill(CUser*, int, int)",
        189,
        0);
    trace("%s:'%s' GM Mode Set Skill(%d) Level(%d).",
          account_id_text,
          charac_name,
          skill_index,
          level);
    return 1;
}

// IDA_EA: 0x858EC9C
// IDA_NAME: _ZN23DisPatcher_DebugCommand20_debugCommandAddItemEP5CUsermi
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandAddItem(CUser *,unsigned long,int)
int DisPatcher_DebugCommand::_debugCommandAddItem(CUser* user,
                                                   unsigned long item_index,
                                                   int count) {
    if (user->isGMUser() != 1) {
        return 0;
    }

    if (item_index == static_cast<unsigned long>(-1)) {
        const long end_time =
            static_cast<long>(GlobalData::s_systemTime_.getCurSec()) + 86400;
        const long start_time =
            static_cast<long>(GlobalData::s_systemTime_.getCurSec());
        WongWork::CHandlePremium::handleSetUserPCRoom(
            user, start_time, end_time);
    }

    if (count <= 0) {
        count = 100;
    }

    CItem* const item =
        G_CDataManager()->find_item(static_cast<int>(item_index));
    if (item) {
        if (item->is_stackable() &&
            static_cast<CStackableItem*>(item)->getStackableLimit() < count) {
            count = static_cast<CStackableItem*>(item)->getStackableLimit();
        }
        if (item->vtable_->is_avatar_item(item)) {
            return 0;
        }
    }

    ENUM_ITEMSPACE item_space;
    const int slot = user->AddItem(static_cast<int>(item_index), count,
                                   static_cast<eItemAddReason>(6), item_space,
                                   0);
    if (slot < 0) {
        return 0;
    }

    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandAddItem(CUser*, itemIndexOnlyServer_t, int)",
        244,
        0);
    trace("%s:'%s' GM Mode Add Item(%d).",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          user->getCurCharacName(),
          slot);
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1), item_space,
                             slot);
    return 1;
}

// IDA_EA: 0x858F7B6
// IDA_NAME: _ZN23DisPatcher_DebugCommand22_debugCommandAddAvatarEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandAddAvatar(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandAddAvatar(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    if (user->isGMUser() != 1) {
        return 0;
    }

    char ipg_no[24];
    std::memset(ipg_no, 0, sizeof(ipg_no));
    std::time_t timer = std::time(0);
    struct tm current_tm;
    struct tm* const local_tm = ::localtime_r(&timer, &current_tm);
    const int charac_no = user->getCurCharacNo();
    const int channel_no = G_CEnvironment()->get_channel_no();
    OS_API::snprintf(ipg_no,
                     0x14u,
                     "D%02d%02d%02d%02d%010d",
                     channel_no,
                     (local_tm->tm_year + 1900) % 2000,
                     local_tm->tm_mon + 1,
                     local_tm->tm_mday,
                     charac_no);

    const int item_id = static_cast<int>(message.param.add_avatar.item_id);
    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item || !item->vtable_->is_avatar_item(item)) {
        return 0;
    }

    const unsigned int requested_period = message.param.add_avatar.period;
    const int usable_period = item->getUsablePeriod();
    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandAddAvatar(CUser*, MSG_DEBUG_COMMAND&)",
        561,
        0);
    trace("[DEBUG_COMMAND_ADD_AVATAR] Param.Sub.AddAvatar.period : %d, item->getUsablePeriod() : %d",
          requested_period,
          usable_period);

    int slot = -1;
    const char first_flag =
        static_cast<char>(message.param.add_avatar.first_flag);
    const uint16_t hidden_option = message.param.add_avatar.hidden_option;
    if (requested_period == 0) {
        slot = user->getCurCharacInvenW()->AddAvatarItem(
            item_id,
            usable_period,
            false,
            first_flag,
            static_cast<char>(-1),
            ipg_no,
            static_cast<eAvatarItemAddReason>(9),
            false,
            hidden_option);
    } else {
        if (requested_period == 99) {
            message.param.add_avatar.period = 0;
        }
        const unsigned int period = message.param.add_avatar.period;
        slot = user->getCurCharacInvenW()->AddAvatarItem(
            item_id,
            static_cast<int>(period),
            false,
            first_flag,
            static_cast<char>(-1),
            ipg_no,
            static_cast<eAvatarItemAddReason>(9),
            false,
            hidden_option);
    }

    if (slot == -1) {
        return 0;
    }
    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(1),
                             slot);
    return 1;
}

// IDA_EA: 0x858EE54
// IDA_NAME: _ZN23DisPatcher_DebugCommand24_debugCommandSetGrowTypeEP5CUserii
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandSetGrowType(CUser *,int,int)
int DisPatcher_DebugCommand::_debugCommandSetGrowType(CUser* user,
                                                       int kind,
                                                       int grow_type) {
    if (user->isGMUser() != 1) {
        return 0;
    }

    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandSetGrowType(CUser*, int, int)",
        307,
        0);
    trace("%s:'%s' GM Mode Grow Type.",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          user->getCurCharacName());

    int first_grow_type = 0;
    int second_grow_type = 0;
    if (kind == 1) {
        first_grow_type = grow_type;
    } else if (kind == 2) {
        if (!user->getCurCharFirstGrowType()) {
            return 0;
        }
        first_grow_type = user->getCurCharFirstGrowType();
        second_grow_type = grow_type;
    } else {
        return 0;
    }

    if (user->ChangeGrowType_GM(first_grow_type, second_grow_type) == 0) {
        return 0;
    }

    const int state = user->get_state();
    if (state == 3) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                             static_cast<ENUM_NOTIPACKET>(2),
                             0);
    } else if (state == 5) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                             static_cast<ENUM_NOTIPACKET>(2),
                             1);
    }
    user->send_skill_info();
    return 1;
}

// IDA_EA: 0x858EFDE
// IDA_NAME: _ZN23DisPatcher_DebugCommand21_debugCommandSetLevelEP5CUseri
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandSetLevel(CUser *,int)
int DisPatcher_DebugCommand::_debugCommandSetLevel(CUser* user, int level) {
    if (user->isGMUser() != 1) {
        return 0;
    }
    if (level > oldsrv::config::kCharacterLevelMaximum) {
        return 0;
    }

    bool level_changed = true;
    const int delta = level - user->get_charac_level();
    if (delta <= 0) {
        for (int step = 0; step > delta; --step) {
            user->SetCharacLevel(
                std::min(200, user->get_charac_level() - 1));
        }
    } else {
        for (int step = 0; step < delta; ++step) {
            user->SetCharacLevel(
                std::min(200, user->get_charac_level() + 1));
        }
    }
    if (delta == 0) {
        level_changed = false;
    }

    user->_onLevelUp();
    user->setCurCharacExp(
        G_CDataManager()->get_level_exp(user->get_charac_level()));
    if (!level_changed) {
        return 0;
    }

    user->CalcurateUserMaxLevel();
    PacketGuard mercenary_packet;
    mercenary_packet.put_header(1, ENUM_CMDPACKET_MERCENARY_INFO);
    mercenary_packet.put_byte(1);
    user->SetMercenaryInfoPacket(mercenary_packet);
    mercenary_packet.finalize(true);
    user->Send(mercenary_packet);

    if (user->GetCurCharacMaxEquipLevel() &&
        user->get_charac_level() == user->GetCurCharacMaxEquipLevel()) {
        user->SetCurCharacMaxEquipLevel(0);
    }

    const int state = user->get_state();
    if (state == 3) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_NOTIPACKET>(37),
                             0);
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                             static_cast<ENUM_NOTIPACKET>(2),
                             0);
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_NOTIPACKET>(2),
                             1);
        PacketGuard quest_packet;
        user->getCurCharacQuestR()->get_quest_info(
            reinterpret_cast<char*>(&quest_packet));
        user->Send(quest_packet);
    } else if (state == 5) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_NOTIPACKET>(37),
                             0);
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                             static_cast<ENUM_NOTIPACKET>(2),
                             1);
        PacketGuard quest_packet;
        user->getCurCharacQuestR()->get_quest_info(
            reinterpret_cast<char*>(&quest_packet));
        user->Send(quest_packet);
    }
    return 1;
}

// IDA_EA: 0x858F3BA
// IDA_NAME: _ZN23DisPatcher_DebugCommand24_debugCommandLevelUpDownEP5CUseri
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandLevelUpDown(CUser *,int)
int DisPatcher_DebugCommand::_debugCommandLevelUpDown(CUser* user,
                                                       int delta) {
    if (user->isGMUser() != 1) {
        return 0;
    }

    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandLevelUpDown(CUser*, int)",
        462,
        0);
    trace("%s:'%s' GM Mode Level Up.",
          NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
          user->getCurCharacName());

    if (delta * delta > 1) {
        return 0;
    }

    bool succeeded = true;
    const int target_level = delta + user->get_charac_level();
    if (user->SetCharacLevel(std::min(200, target_level)) != 1) {
        succeeded = false;
    }
    if (!succeeded) {
        return 0;
    }

    const int state = user->get_state();
    if (state == 3) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_NOTIPACKET>(37),
                             0);
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                             static_cast<ENUM_NOTIPACKET>(2),
                             0);
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_NOTIPACKET>(2),
                             1);
        PacketGuard quest_packet;
        user->getCurCharacQuestR()->get_quest_info(
            reinterpret_cast<char*>(&quest_packet));
        user->Send(quest_packet);
    } else if (state == 5) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_NOTIPACKET>(37),
                             0);
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                             static_cast<ENUM_NOTIPACKET>(2),
                             1);
        PacketGuard quest_packet;
        user->getCurCharacQuestR()->get_quest_info(
            reinterpret_cast<char*>(&quest_packet));
        user->Send(quest_packet);
    }
    return 1;
}

// IDA_EA: 0x858F64C
// IDA_NAME: _ZN23DisPatcher_DebugCommand25_debugCommandGuildLevelUpEP5CUser
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandGuildLevelUp(CUser *)
int DisPatcher_DebugCommand::_debugCommandGuildLevelUp(CUser* user) {
    if (user->isGMUser() != 1) {
        return 0;
    }
    if (user->get_charac_guildkey() == 0) {
        const int charac_no = user->getCurCharacNo();
        cMyTrace trace(
            "bool DisPatcher_DebugCommand::_debugCommandGuildLevelUp(CUser*)",
            522,
            0);
        trace("GM is not Guild Member!( %d )", charac_no);
        return 0;
    }

    const unsigned int guild_key =
        static_cast<unsigned int>(user->get_charac_guildkey());
    const unsigned int charac_no =
        static_cast<unsigned int>(user->getCurCharacNo());
    const int uid = user->GetUID();
    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(0));
    proxy->SendGuildLevelup(uid, charac_no, guild_key, 0);

    STGuildDBInfoOnly* const guild_db_info = user->GetGuildDBInfo();
    STGuildProxy guild_proxy(
        guild_db_info->guild_name,
        static_cast<uint8_t>(guild_db_info->guild_level + 1u),
        guild_db_info->power_side);
    guild_proxy.guild_agit_flag = 0;

    const unsigned int refreshed_guild_key =
        static_cast<unsigned int>(user->get_charac_guildkey());
    CGuildServerProxy* const refreshed_proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(0));
    refreshed_proxy->ModifyGuildInfoProxy(refreshed_guild_key, guild_proxy);
    return 1;
}

// IDA_EA: 0x858FABA
// IDA_NAME: _ZN23DisPatcher_DebugCommand23_debugCommandPVPRankSetEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandPVPRankSet(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandPVPRankSet(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    if (user->isGMUser() != 1) {
        return 0;
    }

    const uint8_t rank = message.param.pvp_rank;
    if (rank > 0x25u) {
        return 0;
    }

    const int next_rank_point =
        GlobalData::s_ref_pvp_grade->GetPvpNextRankPoint(rank);
    user->getPVPResultW()->values[2] = next_rank_point;
    user->getPVPResultW()->values[5] = rank;
    G_GameWorld()->send_AllBasicInfo(user);
    return 1;
}

// IDA_EA: 0x858FB4E
// IDA_NAME: _ZN23DisPatcher_DebugCommand23_debugCommandSetEnchantEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandSetEnchant(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandSetEnchant(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    if (user->isGMUser() != 1) {
        return 0;
    }
    if (!user->getCurCharacR()) {
        return 0;
    }

    const int32_t raw_item_space = message.param.set_enchant.item_space;
    const int32_t slot = message.param.set_enchant.inventory_slot;
    const int32_t raw_enchant = message.param.set_enchant.extend_info;
    const ENUM_ITEMSPACE item_space =
        static_cast<ENUM_ITEMSPACE>(raw_item_space);
    const int inventory_type = GetInvenTypeFromItemSpace(item_space);
    Inven_Item* const item =
        user->getCurCharacInvenW()->GetInvenRef(inventory_type, slot);
    if (!item) {
        return 0;
    }
    std::memcpy(&item->extend_info_,
                &raw_enchant,
                sizeof(item->extend_info_));

    PacketGuard packet;
    packet.put_header(0, 14);
    packet.put_byte(raw_item_space);
    packet.put_short(1);
    if (user->getCurCharacInvenR()->MakeItemPacket(
            static_cast<INVEN_TYPE>(inventory_type), slot, packet) != 1) {
        return 0;
    }
    packet.finalize(true);
    user->Send(packet);
    return 1;
}

// IDA_EA: 0x858FCC4
// IDA_NAME: _ZN23DisPatcher_DebugCommand26_debugCommandSetBlackCountEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandSetBlackCount(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandSetBlackCount(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    if (user->isGMUser() != 1) {
        return 0;
    }

    user->setCurrCharacBlackCount(message.param.black_count);

    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet, user);
    return 1;
}

// IDA_EA: 0x858FDC8
// IDA_NAME: _ZN23DisPatcher_DebugCommand26_debugCommandCargoCapacityEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandCargoCapacity(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandCargoCapacity(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    if (user->isGMUser() != 1) {
        return 0;
    }

    Packet_Guild_Guild_Cargo_Upgrade packet;
    packet.charac_guildkey =
        static_cast<uint32_t>(user->get_charac_guildkey());
    packet.charac_no = static_cast<uint32_t>(user->getCurCharacNo());
    packet.old_capacity = user->GetGuildCargoCapacity();
    packet.new_capacity = message.param.guild_cargo_capacity;

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(0));
    proxy->SendTcpPacket(reinterpret_cast<char*>(&packet), packet.size);
    return 1;
}

// IDA_EA: 0x858FE62
// IDA_NAME: _ZN23DisPatcher_DebugCommand28_debugCommandPowerwarPointUpEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandPowerwarPointUp(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandPowerwarPointUp(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    if (user->isGMUser() != 1) {
        return 0;
    }

    Packet_Guild_Change_Power_War_Point packet;
    packet.winner_point = message.param.guild_power_war_point;
    packet.loser_point = 0;
    packet.power_side = static_cast<uint8_t>(user->getPowerSide());
    packet.winner_charac_nos[0] =
        static_cast<uint32_t>(user->get_charac_no(-1));

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(0));
    proxy->SendPowerWarPoint(packet);
    return 1;
}

// IDA_EA: 0x859019E
// IDA_NAME: _ZN23DisPatcher_DebugCommand23_debugCommandClearInvenEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandClearInven(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandClearInven(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    (void)message;

    for (int slot = oldsrv::config::kMainInventoryEquipmentSlotFirst;
         slot <= oldsrv::config::kMainInventoryLastSlot;
         ++slot) {
        user->getCurCharacInvenW()->GetInvenStart(1)[slot].reset();
    }
    if (user->send_itemspace(0) == 1) {
        return 1;
    }

    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());
    const char* const account_id_text = NumberToString(account_id, 0);
    LogManager::logFormat(
        1,
        "DisPatcher_DebugCommand_First.h",
        "bool DisPatcher_DebugCommand::_debugCommandClearInven(CUser*, MSG_DEBUG_COMMAND&)",
        0x3AF,
        "DisPatcher_DebugCommand::_debugCommandClearInven, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_INVENTORY ) return FAILED",
        account_id_text);
    return 0;
}

// IDA_EA: 0x8590254
// IDA_NAME: _ZN23DisPatcher_DebugCommand26_debugCommandClearAvtInvenEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandClearAvtInven(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandClearAvtInven(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    (void)message;

    for (int slot = 0;
         slot <= oldsrv::config::kAvatarInventoryLastSlot;
         ++slot) {
        const Inven_Item* const item =
            &user->getCurCharacInvenR()->GetInvenStart(2)[slot];
        if (item->item_id_ == 0) {
            continue;
        }

        const int avatar_key =
            user->getCurCharacInvenR()->GetInvenStart(2)[slot].add_info_;
        if (user->getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(2),
                slot,
                1,
                static_cast<eItemDelReason>(3),
                true) != 1) {
            const unsigned int account_id =
                static_cast<unsigned int>(user->get_acc_id());
            const char* const account_id_text =
                NumberToString(account_id, 0);
            LogManager::logFormat(
                1,
                "DisPatcher_DebugCommand_First.h",
                "bool DisPatcher_DebugCommand::_debugCommandClearAvtInven(CUser*, MSG_DEBUG_COMMAND&)",
                0x40C,
                "DisPatcher_DebugCommand::_debugCommandClearAvtInven, pUser(%s)->getCurCharacInvenW()->delete_item return FAILED",
                account_id_text);
            return 0;
        }

        user->getCurCharacInvenW()->SendAvatarEvent(
            54,
            avatar_key,
            1,
            slot + oldsrv::config::kAvatarInventorySlotBase,
            0);
    }
    if (user->send_itemspace(1) == 1) {
        return 1;
    }

    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());
    const char* const account_id_text = NumberToString(account_id, 0);
    LogManager::logFormat(
        1,
        "DisPatcher_DebugCommand_First.h",
        "bool DisPatcher_DebugCommand::_debugCommandClearAvtInven(CUser*, MSG_DEBUG_COMMAND&)",
        0x417,
        "DisPatcher_DebugCommand::_debugCommandClearAvtInven, pUser(%s)->send_itemspace( ENUM_ITEMSPACE_AVATAR ) return FAILED",
        account_id_text);
    return 0;
}

// IDA_EA: 0x858FEE8
// IDA_NAME: _ZN23DisPatcher_DebugCommand32_debugCommandUserPowerwarPointUpEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandUserPowerwarPointUp(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandUserPowerwarPointUp(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    if (user->isGMUser() != 1) {
        return 0;
    }

    user->IncreasePowerWarPoint(
        static_cast<int16_t>(message.param.user_power_war_point));

    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet, user);
    return 1;
}

// IDA_EA: 0x858FFEE
// IDA_NAME: _ZN23DisPatcher_DebugCommand22_debugCommandPowerUpOnEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandPowerUpOn(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandPowerUpOn(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    (void)message;

    if (user->isGMUser() != 1) {
        return 0;
    }

    user->SetPowerUp(true);
    std::vector<CUser*> users;
    users.push_back(user);
    G_GameWorld()->SendPowerUpList(
        static_cast<ENUM_POWER_UP_ACTIVATE_TYPE>(1), users);
    return 1;
}

// IDA_EA: 0x8590092
// IDA_NAME: _ZN23DisPatcher_DebugCommand34_debugCommandInitUserPowerwarPointEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandInitUserPowerwarPoint(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandInitUserPowerwarPoint(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    (void)message;

    if (user->isGMUser() != 1) {
        return 0;
    }

    user->DecreasePowerWarPoint(
        static_cast<int16_t>(user->GetUserPowerWarPoint()));

    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet, user);
    return 1;
}

// IDA_EA: 0x859040E
// IDA_NAME: _ZN23DisPatcher_DebugCommand26_debugCommandPowerWarStartEP5CUser
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandPowerWarStart(CUser *)
int DisPatcher_DebugCommand::_debugCommandPowerWarStart(CUser* user) {
    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());
    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(0));
    proxy->SendPowerWarStart(account_id);
    return 1;
}

// IDA_EA: 0x859044E
// IDA_NAME: _ZN23DisPatcher_DebugCommand24_debugCommandPowerWarEndEP5CUser
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandPowerWarEnd(CUser *)
int DisPatcher_DebugCommand::_debugCommandPowerWarEnd(CUser* user) {
    const unsigned int account_id =
        static_cast<unsigned int>(user->get_acc_id());
    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(0));
    proxy->SendPowerWarEnd(account_id);
    return 1;
}

// IDA_EA: 0x859048E
// IDA_NAME: _ZN23DisPatcher_DebugCommand22_debugCommandUserRepelEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandUserRepel(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandUserRepel(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    const int charac_name_size = message.param.character_name.length;
    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendUserRepel(static_cast<uint32_t>(user->get_acc_id()),
                          user->GetUID(),
                          message.param.character_name.text,
                          charac_name_size);
    return 1;
}

// IDA_EA: 0x8590506
// IDA_NAME: _ZN23DisPatcher_DebugCommand23_debugCommandRequestMidEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandRequestMid(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandRequestMid(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    const int charac_name_size = message.param.character_name.length;
    // The original accepts all signed values no greater than 29.  Do not add
    // a nonnegative check: the sender preserves that old machine behavior.
    if (charac_name_size <= 29) {
        CMonitorServerProxy* const proxy =
            GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        proxy->SendRequestUserMid(static_cast<uint32_t>(user->get_acc_id()),
                                  user->GetUID(),
                                  message.param.character_name.text,
                                  charac_name_size);
    }
    return 1;
}

// IDA_EA: 0x859058A
// IDA_NAME: _ZN23DisPatcher_DebugCommand22_debugCommandInvisibleEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandInvisible(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandInvisible(
    CUser* user,
    MSG_DEBUG_COMMAND& message) {
    (void)message;

    user->SetInvisible();
    PacketGuard packet;
    packet.put_header(0, 6);
    packet.put_short(user->get_unique_id());
    packet.finalize(true);
    G_GameWorld()->send_all_excepted_user(packet, user);
    user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                         static_cast<ENUM_NOTIPACKET>(2),
                         0);

    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandInvisible(CUser*, MSG_DEBUG_COMMAND&)",
        92,
        0);
    trace("GM(%s) is invisible \n", user->getCurCharacName());
    return 1;
}

// IDA_EA: 0x85906AC
// IDA_NAME: _ZN23DisPatcher_DebugCommand20_debugCommandVisibleEP5CUserR17MSG_DEBUG_COMMAND
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandVisible(CUser *,MSG_DEBUG_COMMAND &)
int DisPatcher_DebugCommand::_debugCommandVisible(CUser* user,
                                                   MSG_DEBUG_COMMAND& message) {
    (void)message;

    user->SetVisible();
    if (user->get_state() == 3) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                             static_cast<ENUM_NOTIPACKET>(2),
                             0);
    }

    PacketGuard packet;
    packet.put_header(0, 23);
    packet.put_short(user->get_unique_id());
    packet.put_byte(user->getCurCharacVill());
    packet.put_byte(user->get_area(false));
    packet.put_short(user->get_posX());
    packet.put_short(user->get_posY());
    packet.put_byte(user->get_direction());
    packet.put_byte(user->get_charac_visible_values());
    packet.finalize(true);
    G_GameWorld()->send_to_area(user->getCurCharacVill(),
                                 user->get_area(false),
                                 packet);

    cMyTrace trace(
        "bool DisPatcher_DebugCommand::_debugCommandVisible(CUser*, MSG_DEBUG_COMMAND&)",
        131,
        0);
    trace("GM(%s) is visible \n", user->getCurCharacName());
    return 1;
}

// IDA_EA: 0x85908BA
// IDA_NAME: _ZN23DisPatcher_DebugCommand26_debugCommandSetUserDefineEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_DebugCommand::_debugCommandSetUserDefine(CUser *,PacketBuf &)
int DisPatcher_DebugCommand::_debugCommandSetUserDefine(CUser* user,
                                                         PacketBuf& packet) {
    (void)packet;
    return user == 0;
}
