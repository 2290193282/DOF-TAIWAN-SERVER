#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "old_game/progression/skill/create_character_skill_abi.h"

#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/force_change_grow_type_quest_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_changer_runtime.h"
#include "old_game/player/core/calc_charac_stat_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <stddef.h>
#include <stdint.h>


namespace {

struct QuestGrowItemRule {
    int item_id;
    uint8_t charac_job;
    uint8_t first_grow_type;
    uint8_t second_grow_type;
    bool count_elixer;
    bool require_first_grow_type;
};

// IDA 0x8668D1A dispatches exactly these quest-item IDs.  The flags preserve
// the original job/grow transition and the two distinct skill reset paths.
const QuestGrowItemRule kQuestGrowItemRules[] = {
    {4041, 0, 1, 0, false, false}, {4042, 0, 2, 0, false, false},
    {4043, 0, 3, 0, false, false}, {4044, 0, 4, 0, false, false},
    {4045, 1, 2, 0, false, false}, {4046, 1, 1, 0, false, false},
    {4047, 1, 3, 0, false, false}, {4048, 1, 4, 0, false, false},
    {4049, 2, 1, 0, false, false}, {4050, 2, 3, 0, false, false},
    {4051, 2, 2, 0, false, false}, {4052, 2, 4, 0, false, false},
    {4079, 3, 1, 0, false, false}, {4080, 3, 3, 0, false, false},
    {4081, 3, 2, 0, false, false}, {4082, 3, 4, 0, false, false},
    {4087, 0, 1, 0, true, false},  {4088, 0, 2, 0, true, false},
    {4089, 0, 3, 0, true, false},  {4090, 0, 4, 0, true, false},
    {4091, 1, 2, 0, true, false},  {4092, 1, 1, 0, true, false},
    {4093, 1, 3, 0, true, false},  {4094, 1, 4, 0, true, false},
    {4095, 2, 1, 0, true, false},  {4096, 2, 3, 0, true, false},
    {4097, 2, 2, 0, true, false},  {4098, 2, 4, 0, true, false},
    {4099, 3, 1, 0, true, false},  {4100, 3, 3, 0, true, false},
    {4101, 3, 2, 0, true, false},  {4102, 3, 4, 0, true, false},
    {4165, 4, 1, 0, false, false}, {4166, 4, 2, 0, false, false},
    {4167, 4, 3, 0, false, false}, {4168, 4, 4, 0, false, false},
    {4188, 1, 1, 1, true, true},   {4189, 1, 2, 1, true, true},
    {4190, 1, 3, 1, true, true},   {4191, 1, 4, 1, true, true},
    {4192, 2, 1, 1, true, true},   {4193, 2, 2, 1, true, true},
    {4194, 2, 3, 1, true, true},   {4195, 2, 4, 1, true, true},
    {4196, 3, 1, 1, true, true},   {4197, 3, 2, 1, true, true},
    {4217, 3, 3, 1, true, true},   {4218, 0, 1, 1, true, true},
    {4219, 0, 2, 1, true, true},   {4220, 0, 3, 1, true, true},
    {4221, 0, 4, 1, true, true},   {7964, 3, 4, 1, true, true},
    {7965, 4, 1, 1, true, true},   {7966, 4, 2, 1, true, true},
    {7967, 4, 3, 1, true, true},   {7968, 5, 1, 0, true, false},
    {7969, 5, 3, 0, true, false},  {7970, 5, 2, 0, true, false},
    {7971, 5, 4, 0, true, false},  {7972, 5, 1, 1, true, true},
    {7973, 5, 3, 1, true, true},   {7974, 5, 2, 1, true, true},
    {7975, 5, 4, 1, true, true},   {7976, 6, 1, 0, true, false},
    {7977, 6, 2, 0, true, false},  {7978, 6, 1, 1, true, true},
    {7979, 6, 2, 1, true, true},   {8016, 7, 1, 0, true, false},
    {8017, 7, 3, 0, true, false},  {8018, 7, 2, 0, true, false},
    {8019, 7, 4, 0, true, false},  {8020, 7, 1, 1, true, true},
    {8021, 7, 3, 1, true, true},   {8022, 7, 2, 1, true, true},
    {8023, 7, 4, 1, true, true},
};

struct RemovedQuestItem {
    int slot;
    int count;
};

}  // namespace

// IDA_EA: 0x8668D1A
// IDA_NAME: _ZN5CUser12UseQuestItemEi
// DEMANGLED: CUser::UseQuestItem(int)
void CUser::UseQuestItem(int item_slot) {
    if (get_state() != 3) {
        return;
    }

    PacketGuard packet;
    if (CheckInTrade()) {
        packet.clear();
        SendCmdErrorPacket(ENUM_CMDPACKET_INCREASE_STATUS, 19, packet);
        return;
    }

    const Inven_Item used_item =
        getCurCharacInvenR()->GetInvenSlot(1, item_slot);
    const QuestGrowItemRule* selected_rule = 0;
    for (size_t index = 0;
         index != sizeof(kQuestGrowItemRules) / sizeof(kQuestGrowItemRules[0]);
         ++index) {
        if (kQuestGrowItemRules[index].item_id == used_item.getKey()) {
            selected_rule = &kQuestGrowItemRules[index];
            break;
        }
    }

    if (!selected_rule ||
        (selected_rule->require_first_grow_type &&
         !getCurCharFirstGrowType()) ||
        selected_rule->charac_job >=
            oldsrv::config::kCharacterJobCount ||
        static_cast<int>(selected_rule->charac_job) != get_charac_job()) {
        packet.clear();
        packet.put_header(1, ENUM_CMDPACKET_INCREASE_STATUS);
        packet.put_byte(0);
        packet.put_byte(17);
        packet.finalize(true);
        Send(packet);
        return;
    }

    const int current_level_exp =
        G_CDataManager()->get_level_exp(get_charac_level());
    const int next_level_exp =
        G_CDataManager()->get_level_exp(get_charac_level() + 1);
    if (getCurCharacExp() >= current_level_exp) {
        if (getCurCharacExp() > next_level_exp) {
            setCurCharacExp(next_level_exp - 1);
        }
    } else {
        setCurCharacExp(current_level_exp);
    }

    RemovedQuestItem removed_items[54] = {};
    int removed_count = 0;
    for (int slot = oldsrv::config::kMainInventoryQuickSlotFirst;
         slot <= oldsrv::config::kMainInventoryQuickSlotLast;
         ++slot) {
        const Inven_Item item = getCurCharacInvenR()->GetInvenSlot(1, slot);
        const unsigned int item_id = item.getKey();
        const bool should_remove =
            selected_rule->second_grow_type != 0u
                ? (item_id > 0x105Bu && item_id <= 0x1065u)
                : ((item_id > 0xFC8u && item_id <= 0xFD4u) ||
                   (item_id > 0xFEEu && item_id <= 0xFF2u) ||
                   (item_id > 0xFF6u && item_id <= 0x1006u) ||
                   (item_id > 0x1044u && item_id <= 0x1048u) ||
                   (item_id > 0x1F1Fu && item_id <= 0x1F23u) ||
                   (item_id > 0x1F27u && item_id <= 0x1F29u) ||
                   (item_id > 0x1F4Fu && item_id <= 0x1F53u));
        if (should_remove) {
            removed_items[removed_count].slot = slot;
            removed_items[removed_count].count = item.get_add_info();
            ++removed_count;
        }
    }
    for (int slot = oldsrv::config::kMainInventoryQuestSlotFirst;
         slot <= oldsrv::config::kMainInventoryQuestSlotLast;
         ++slot) {
        const Inven_Item item = getCurCharacInvenR()->GetInvenSlot(1, slot);
        const unsigned int item_id = item.getKey();
        const bool should_remove =
            selected_rule->second_grow_type != 0u
                ? (item_id > 0x105Bu && item_id <= 0x1065u)
                : ((item_id > 0xFC8u && item_id <= 0xFD4u) ||
                   (item_id > 0xFEEu && item_id <= 0xFF2u) ||
                   (item_id > 0xFF6u && item_id <= 0x1006u) ||
                   (item_id > 0x1044u && item_id <= 0x1048u) ||
                   (item_id > 0x1F1Fu && item_id <= 0x1F23u) ||
                   (item_id > 0x1F27u && item_id <= 0x1F29u) ||
                   (item_id > 0x1F4Fu && item_id <= 0x1F53u));
        if (should_remove) {
            removed_items[removed_count].slot = slot;
            removed_items[removed_count].count = item.get_add_info();
            ++removed_count;
        }
    }

    if (removed_count > 0) {
        packet.clear();
        packet.put_header(1, 18);
        packet.put_byte(1);
        packet.put_byte(0);
        packet.put_byte(removed_count);
        for (int index = 0; index < removed_count; ++index) {
            getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(1),
                removed_items[index].slot,
                removed_items[index].count,
                static_cast<eItemDelReason>(10),
                true);
            packet.put_short(removed_items[index].slot);
            packet.put_int(removed_items[index].count);
        }
        packet.finalize(true);
        Send(packet);
    }

    WongWork::CSkillChanger skill_changer;
    if (selected_rule->count_elixer) {
        skill_changer.CountElixer(this);
        const int character_job = get_charac_job();
        const int character_level = get_charac_level();
        getCurCharacSkillW()->debugCheckGrowTypeSkill(
            character_level,
            character_job,
            selected_rule->first_grow_type,
            selected_rule->second_grow_type);
    } else {
        getCurCharacSkillW()->clear_all_skills_both();
        getCurCharacSkillW()->set_parent(this);
        const int character_job_for_first_tree = get_charac_job();
        addSkillOnCreateCharacter(
            getCurCharacSkillW()->get_mastered_skill_tree(ENUM_SKILL_TREE_0),
            character_job_for_first_tree);
        const int character_job_for_second_tree = get_charac_job();
        addSkillOnCreateCharacter(
            getCurCharacSkillW()->get_mastered_skill_tree(ENUM_SKILL_TREE_1),
            character_job_for_second_tree);
        skill_changer.SkillInitialize(this, 0, true);
    }

    const int init_character_job = get_charac_job();
    WongWork::CCalcCharacStat::InitializeStat(getCurCharacAddInfoW(),
                                               init_character_job);
    const int stat_character_level = get_charac_level();
    const int stat_character_job = get_charac_job();
    WongWork::CCalcCharacStat::CalcStat(getCurCharacAddInfoW(),
                                        stat_character_job,
                                        selected_rule->first_grow_type,
                                        selected_rule->second_grow_type,
                                        stat_character_level);

    int32_t* const need_elixer_count = skill_changer.GetNeedElixerCount();
    int32_t* const elixer_count = skill_changer.GetElixerCount();
    int32_t* const need_elixer_ability =
        skill_changer.GetNeedElixerAbility();
    for (int index = 1; index <= need_elixer_ability[0]; ++index) {
        if (need_elixer_count[index] > elixer_count[index]) {
            const int count_delta =
                need_elixer_count[index] - elixer_count[index];
            const int ability = need_elixer_ability[index];
            WongWork::CCalcCharacStat::ApplyElixerItem(
                getCurCharacAddInfoW(), ability, count_delta);
        }
    }

    set_grow_type(selected_rule->first_grow_type,
                  selected_rule->second_grow_type,
                  0,
                  static_cast<eChangeGrowTypeReason>(1));
    _postCheckForceChangeGrowType(this,
                                  selected_rule->first_grow_type,
                                  selected_rule->second_grow_type);
    SendNotiPacket(static_cast<eSendTarget>(0),
                   static_cast<ENUM_NOTIPACKET>(2),
                   0);
    send_skill_info();
    SendNotiPacket(static_cast<eSendTarget>(1),
                   static_cast<ENUM_NOTIPACKET>(2),
                   1);
}
