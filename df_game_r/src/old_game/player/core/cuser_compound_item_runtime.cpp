#include "old_game/player/core/cuser.h"

#include "common/config.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/history_log_abi.h"
#include "common/manager/server_utility.h"
#include "common/manager/statistics_runtime.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/item_making_skill_script.h"
#include "old_data/db/skill/item_making_skill_payload.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/gen_unique_no.h"
#include "old_game/economy/inventory/recipe_info.h"
#include "old_game/progression/career/expert_job_statistic.h"
#include "old_game/progression/skill/item_making_skill.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <cstdio>
#include <cstring>

namespace {

enum {
    kCompoundDeleteReason = 8,
    kCompoundMoneySubReason = 12,
    kCompoundMoneyAddReason = 7,
    kCompoundItemAddReason = 9,
    kAvatarDeleteReason = 3,
    kCompoundBroadcastPacket = 86
};

void TraceCompoundStage(CUser* user,
                        unsigned int recipe_id,
                        int recipe_slot,
                        bool avatar_recipe,
                        unsigned short compound_count,
                        int command_id,
                        const char* stage,
                        int error) {
    cMyTrace trace("CUser::compound_item recipe transaction", 0, error ? 5 : 0);
    trace("[RECIPE][CMD25] stage=%s error=%d charac_no=%d recipe=%u slot=%d avatar=%d count=%u command=%d",
          stage,
          error,
          user ? user->get_charac_no(-1) : 0,
          recipe_id,
          recipe_slot,
          avatar_recipe ? 1 : 0,
          static_cast<unsigned int>(compound_count),
          command_id);
}

void TraceCompoundPlan(unsigned int recipe_id,
                       const char* kind,
                       const std::vector<std::pair<int, int> >& items) {
    if (items.empty()) {
        cMyTrace trace("CUser::compound_item recipe plan", 0, 0);
        trace("[RECIPE][PLAN] recipe=%u kind=%s item=0 count=0 empty=1",
              recipe_id,
              kind);
        return;
    }
    for (std::vector<std::pair<int, int> >::const_iterator it = items.begin();
         it != items.end();
         ++it) {
        cMyTrace trace("CUser::compound_item recipe plan", 0, 0);
        trace("[RECIPE][PLAN] recipe=%u kind=%s item=%d count=%d empty=0",
              recipe_id,
              kind,
              it->first,
              it->second);
    }
}

int GetRawItemMakingPoint(const SIG_ITEM_MAKING_SKILL& payload, int type) {
    const int16_t points[11] = {
        payload.weapon,
        payload.cloth,
        payload.leather,
        payload.light_armor,
        payload.heavy_armor,
        payload.plate,
        payload.amulet,
        payload.wrist,
        payload.ring,
        payload.support,
        payload.magic_stone,
    };
    return type >= 0 && type < 11 ? points[type] : -1;
}

bool PreflightItemMakingPostProcess(
    CUser* user,
    CStackableItem* recipe) {
    if (!user || !recipe) {
        return false;
    }
    // Native STStackableScript uses zero for an absent
    // [minimum itemmaking level].  Treat only a positive requirement as an
    // item-making recipe so malformed negative legacy data cannot index the
    // default type 11 as though it were one of the eleven real categories.
    if (recipe->GetNeedLevel() <= 0) {
        return true;
    }

    const int type = recipe->GetUpSkillType();
    stItemMakingSkill* const table =
        G_CDataManager() ? G_CDataManager()->GetItemMakingSkill() : 0;
    CItemMakingSkill* const skill = static_cast<CItemMakingSkill*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(10)));
    int raw_point = -1;
    int effective_level = -1;
    if (skill && type >= 0 && type < 11) {
        SIG_ITEM_MAKING_SKILL payload;
        skill->getData(reinterpret_cast<char*>(&payload));
        raw_point = GetRawItemMakingPoint(payload, type);
        if (table && !table->exp_limits.empty()) {
            effective_level = skill->GetItemMakingSkillLevel(recipe, user);
        }
    }

    const unsigned int exp_limit_count = table ?
        static_cast<unsigned int>(table->exp_limits.size()) : 0u;
    const unsigned int upgrade_rule_count = table ?
        static_cast<unsigned int>(table->upgrade_with_compound.size()) : 0u;
    const stUpgradeWithCompound* upgrade_rule = 0;
    if (table && effective_level >= 0) {
        const std::map<int, stUpgradeWithCompound>::const_iterator found =
            table->upgrade_with_compound.find(effective_level);
        if (found != table->upgrade_with_compound.end()) {
            upgrade_rule = &found->second;
        }
    }

    bool probability_ok = false;
    bool range_ok = false;
    if (upgrade_rule) {
        probability_ok =
            upgrade_rule->compound_exp >= 0 &&
            upgrade_rule->compound_exp <= 10000 &&
            upgrade_rule->upgrade_exp >= 0 &&
            upgrade_rule->upgrade_exp <= 10000 &&
            upgrade_rule->compound_exp + upgrade_rule->upgrade_exp == 10000;
        const int64_t expected_compound_range =
            1000LL * (static_cast<int64_t>(upgrade_rule->max_compound_value) -
                      static_cast<int64_t>(upgrade_rule->min_compound_value)) +
            999LL;
        const int64_t expected_upgrade_range =
            1000LL * (static_cast<int64_t>(upgrade_rule->max_upgrade_value) -
                      static_cast<int64_t>(upgrade_rule->min_upgrade_value)) +
            999LL;
        range_ok =
            upgrade_rule->min_compound_value <=
                upgrade_rule->max_compound_value &&
            upgrade_rule->min_upgrade_value <=
                upgrade_rule->max_upgrade_value &&
            upgrade_rule->compound_range > 0 &&
            upgrade_rule->upgrade_range > 0 &&
            static_cast<int64_t>(upgrade_rule->compound_range) ==
                expected_compound_range &&
            static_cast<int64_t>(upgrade_rule->upgrade_range) ==
                expected_upgrade_range;
    }
    const bool rule_ok = upgrade_rule && probability_ok && range_ok;

    cMyTrace detail("CUser::compound_item item-making preflight", 0,
                    rule_ok ? 0 : 5);
    detail("[RECIPE][ITEMMAKING] stage=preflight recipe=%u type=%d need=%d raw=%d effective=%d exp_limits=%u upgrade_rules=%u rule=%d probability=%d range=%d compound=%d/%d/%d/%d upgrade=%d/%d/%d/%d",
           recipe->get_index(),
           type,
           recipe->GetNeedLevel(),
           raw_point,
           effective_level,
           exp_limit_count,
           upgrade_rule_count,
           upgrade_rule ? 1 : 0,
           probability_ok ? 1 : 0,
           range_ok ? 1 : 0,
           upgrade_rule ? upgrade_rule->min_compound_value : 0,
           upgrade_rule ? upgrade_rule->max_compound_value : 0,
           upgrade_rule ? upgrade_rule->compound_exp : 0,
           upgrade_rule ? upgrade_rule->compound_range : 0,
           upgrade_rule ? upgrade_rule->min_upgrade_value : 0,
           upgrade_rule ? upgrade_rule->max_upgrade_value : 0,
           upgrade_rule ? upgrade_rule->upgrade_exp : 0,
           upgrade_rule ? upgrade_rule->upgrade_range : 0);

    if (!skill || !table || type < 0 || type >= 11 ||
        table->exp_limits.empty() || !rule_ok) {
        cMyTrace failure("CUser::compound_item item-making preflight", 0, 5);
        failure("[RECIPE][PREFLIGHT_FAIL] reason=item_making_postprocess recipe=%u skill=%d table=%d type=%d raw=%d effective=%d exp_limits=%u upgrade_rules=%u rule=%d probability=%d range=%d",
                recipe->get_index(),
                skill ? 1 : 0,
                table ? 1 : 0,
                type,
                raw_point,
                effective_level,
                exp_limit_count,
                upgrade_rule_count,
                upgrade_rule ? 1 : 0,
                probability_ok ? 1 : 0,
                range_ok ? 1 : 0);
        return false;
    }
    return true;
}

int PreviewCompoundOutputBranch(
    CUser* user,
    const std::vector<std::pair<int, int> >& materials,
    const std::vector<std::pair<int, int> >& outputs,
    int recipe_slot,
    bool avatar_recipe,
    unsigned short compound_count,
    unsigned int recipe_id,
    const char* kind,
    int insert_error) {
    // Keep the native result-shape guard in front of every branch.  Besides
    // rejecting more than one copy of a non-stackable result, it also proves
    // that every item id resolves to a PVF prototype before any mutation.
    if (!user || user->_IsCompoundResult2MoreEquip(outputs)) {
        cMyTrace trace("CUser::compound_item recipe preview", 0, 5);
        trace("[RECIPE][PREVIEW_FAIL] recipe=%u kind=%s stage=output_shape error=23 outputs=%u",
              recipe_id,
              kind,
              static_cast<unsigned int>(outputs.size()));
        return 23;
    }
    for (std::vector<std::pair<int, int> >::const_iterator it = outputs.begin();
         it != outputs.end();
         ++it) {
        if (it->second <= 0) {
            cMyTrace trace("CUser::compound_item recipe preview", 0, 5);
            trace("[RECIPE][PREVIEW_FAIL] recipe=%u kind=%s stage=output_count error=23 item=%d count=%d",
                  recipe_id,
                  kind,
                  it->first,
                  it->second);
            return 23;
        }
    }

    // CUser requests are serialized by the character owner.  Each possible
    // result branch is therefore replayed from the same immutable inventory
    // snapshot: delete the exact materials/recipe first, then require the
    // inventory API to accept every output pair, not merely the first one.
    CInventory preview;
    InventoryMemory preview_memory;
    preview.SetInventoryMemory(&preview_memory);
    preview.setCopy(*user->getCurCharacInvenRefR());
    if (!preview.tryDeleteEventItems(materials) ||
        (!avatar_recipe && preview.delete_item(
            static_cast<INVEN_TYPE>(1), recipe_slot, compound_count,
            static_cast<eItemDelReason>(kCompoundDeleteReason), false) != 1)) {
        cMyTrace trace("CUser::compound_item recipe preview", 0, 5);
        trace("[RECIPE][PREVIEW_FAIL] recipe=%u kind=%s stage=delete error=17 outputs=%u",
              recipe_id,
              kind,
              static_cast<unsigned int>(outputs.size()));
        return 17;
    }

    const int inserted = preview.tryInsertEventItems(outputs);
    const int expected = static_cast<int>(outputs.size());
    cMyTrace trace("CUser::compound_item recipe preview", 0,
                   inserted == expected ? 0 : 5);
    trace("[RECIPE][PREVIEW] recipe=%u kind=%s inserted=%d expected=%d error=%d",
          recipe_id,
          kind,
          inserted,
          expected,
          inserted == expected ? 0 : insert_error);
    return inserted == expected ? 0 : insert_error;
}

void PutDeletedItems(PacketGuard& packet, const std::vector<int>& items) {
    for (std::vector<int>::const_iterator it = items.begin();
         it != items.end();) {
        packet.put_byte(*it++);
        packet.put_short(*it++);
        packet.put_int(*it++);
    }
}

void PutInvenItem(PacketGuard& packet, const Inven_Item& item, int slot) {
    packet.put_byte(0);
    packet.put_short(slot);
    packet.put_int(item.item_id_);
    packet.put_int(item.add_info_);
    packet.put_byte(item.GetUpgrade());
    packet.put_short(item.avatar_select_);
    packet.put_byte(item.amplify_option_.getAbilityType());
    packet.put_short(item.amplify_option_.getAbilityValue());
    packet.put_byte(item.seal_or_state_ != 0);
    packet.put_packet(item);
}

}  // namespace

// IDA_EA: 0x8670E38
// IDA_NAME: _ZN5CUser14_GetRecipeItemEmbRi
// DEMANGLED: CUser::_GetRecipeItem(unsigned long,bool,int &)
CStackableItem* CUser::_GetRecipeItem(unsigned long recipe_slot,
                                      bool avatar_recipe,
                                      int& recipe_add_info) {
    CItem* item = 0;
    if (avatar_recipe) {
        item = G_CDataManager()->find_item(static_cast<int>(recipe_slot));
    } else {
        const Inven_Item recipe = getCurCharacInvenR()->GetInvenSlot(
            1, static_cast<int>(recipe_slot));
        if (!recipe.item_id_) {
            return 0;
        }
        recipe_add_info = recipe.get_add_info();
        item = G_CDataManager()->find_item(recipe.item_id_);
    }
    if (!item || !item->is_stackable()) {
        return 0;
    }
    CStackableItem* const recipe = static_cast<CStackableItem*>(item);
    return recipe->GetItemType() == 3 ? recipe : 0;
}

// IDA_EA: 0x8670F1E
// IDA_NAME: _ZN5CUser19_CheckCompoundSkillEPK14CStackableItemb
// DEMANGLED: CUser::_CheckCompoundSkill(CStackableItem const*,bool)
bool CUser::_CheckCompoundSkill(const CStackableItem* recipe,
                                bool avatar_recipe) {
    const RecipeInfo& info = recipe->recipe_info();
    if (info.flag_24 && !isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(7))) {
        return false;
    }

    // IDA 0x8670F1E stores `check_skill_list(...) ^ 1` and then negates that
    // byte for the ordinary-recipe return, so the original success contract
    // is a non-zero check_skill_list result.
    const bool has_required_skill =
        getCurCharacSkillR()->check_skill_list(get_charac_job(),
                                                info.pairs_18,
                                                2) != 0;
    if (!info.flag_25) {
        return avatar_recipe || has_required_skill;
    }
    if (!has_required_skill) {
        return false;
    }
    return expert_job::CExpertJob::IsLearnRecipe(this, recipe);
}

// IDA_EA: 0x8671EB2
// IDA_NAME: _ZN5CUser14copyItemOptionER10Inven_ItemS1_
// DEMANGLED: CUser::copyItemOption(Inven_Item &,Inven_Item &)
int CUser::copyItemOption(Inven_Item& output, Inven_Item& material) {
    output.SetUpgrade(static_cast<uint8_t>(material.GetUpgrade()));
    output.set_add_info(material.get_add_info());
    output.upgrade_separate_.bits_ =
        static_cast<uint8_t>((material.upgrade_separate_.bits_ & 0x1fu) |
                             (output.upgrade_separate_.bits_ & 0xe0u));
    output.amplify_option_ = material.amplify_option_;
    output.extend_info_ = material.extend_info_;
    return LogHistory("Compound+,%u,%d,%d,%d,%d,%d,%d,%d,",
                      static_cast<unsigned int>(output.item_id_),
                      output.GetUpgrade(),
                      output.get_add_info(),
                      output.upgrade_separate_.bits_ & 0x1f,
                      output.extend_info_,
                      output.amplify_option_.getAbilityType(),
                      output.amplify_option_.getAbilityValue(),
                      output.amplify_option_.GetLock());
}

// IDA_EA: 0x8682082
// IDA_NAME: _ZNK5CUser18getCompoundUpgradeEPK14CStackableItem16stEquipCondition
// DEMANGLED: CUser::getCompoundUpgrade(CStackableItem const*,stEquipCondition)const
int CUser::getCompoundUpgrade(const CStackableItem* recipe,
                              stEquipCondition condition) const {
    const ServerParameterScript* const server_parameter =
        G_CDataManager()->get_server_parameter_script();
    const bool use_second_table = recipe->IsUse2ndUpgradeTableRecipe();
    const bool amplified_material = condition.has_amplify_option != 0;
    const CompoundUpgradeRateRow& rates =
        server_parameter->get_recipe_compound_upgrade_rates(
            use_second_table,
            amplified_material,
            condition.equipment_upgrade);
    if (use_second_table) {
        if (amplified_material) {
            cMyTrace trace(
                "int CUser::getCompoundUpgrade(const CStackableItem*, stEquipCondition) const",
                34539, 0);
            trace("RECIPE LOG : (Item:%u)AmplifyTable2nd", recipe->get_index());
        } else {
            cMyTrace trace(
                "int CUser::getCompoundUpgrade(const CStackableItem*, stEquipCondition) const",
                34533, 0);
            trace("RECIPE LOG : (Item:%u)UpgradeTable2nd", recipe->get_index());
        }
    } else if (amplified_material) {
        cMyTrace trace(
            "int CUser::getCompoundUpgrade(const CStackableItem*, stEquipCondition) const",
            34554, 0);
        trace("RECIPE LOG : (Item:%u)AmplifyTable", recipe->get_index());
    } else {
        cMyTrace trace(
            "int CUser::getCompoundUpgrade(const CStackableItem*, stEquipCondition) const",
            34548, 0);
        trace("RECIPE LOG : (Item:%u)UpgradeTable", recipe->get_index());
    }

    const int random_value = get_rand_int(10000);
    int cumulative_rate = 0;
    for (int upgrade = 0; upgrade <= 31; ++upgrade) {
        cumulative_rate += rates.rates[upgrade];
        if (cumulative_rate > random_value) {
            return upgrade;
        }
    }
    return 0;
}

// IDA_EA: 0x867306E
// IDA_NAME: _ZN5CUser24setRecipeEqiupmentOptionER10Inven_Item16stEquipConditioniPK14CStackableItemi
// DEMANGLED: CUser::setRecipeEqiupmentOption(Inven_Item &,stEquipCondition,int,CStackableItem const*,int)
void CUser::setRecipeEqiupmentOption(Inven_Item& output,
                                     stEquipCondition condition,
                                     int output_slot,
                                     const CStackableItem* recipe,
                                     int output_type) {
    CItem* const item = G_CDataManager()->find_item(output.item_id_);
    if (!item || output.item_category_ != 1 ||
        condition.equipment_upgrade > 31) {
        return;
    }

    const int upgrade = output_type == 11 ? 0 :
        getCompoundUpgrade(recipe, condition);
    if (condition.has_amplify_option) {
        output.amplify_option_.assign(condition.amplify_ability_type,
                                      condition.amplify_ability_value);
        output.amplify_option_.identify();
    }
    output.SetUpgrade(static_cast<uint8_t>(upgrade));
    output.seal_or_state_ =
        condition.sealed == 1 &&
        (item->GetAttachType() == 3 || item->GetAttachType() == 4);
    if (item->GetAttachType() == 3) {
        output.SetReSealCount(condition.reseal_count);
    }
    getCurCharacInvenW()->update_item(static_cast<INVEN_TYPE>(1),
                                      output_slot,
                                      output);
}

// IDA_EA: 0x8692F62
// IDA_NAME: _ZN5CUser21compoundSystemMessageERK10Inven_Item
// DEMANGLED: CUser::compoundSystemMessage(Inven_Item const&)
void CUser::compoundSystemMessage(const Inven_Item& item) {
    PacketGuard packet;
    packet.put_header(0, kCompoundBroadcastPacket);
    packet.put_byte(12);
    packet.put_short(get_unique_id());
    packet.put_int(item.getKey());
    packet.put_byte(item.GetUpgrade());
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
}

// IDA_EA: 0x869304E
// IDA_NAME: _ZN5CUser20compoundGuildMessageERK10Inven_Item
// DEMANGLED: CUser::compoundGuildMessage(Inven_Item const&)
char CUser::compoundGuildMessage(const Inven_Item& item) {
    CItem* const prototype = G_CDataManager()->find_item(item.getKey());
    if (!prototype) {
        return 0;
    }

    char message[120] = {};
    std::strcpy(message, getCurCharacName());
    // IDA rodata aS_58 is exactly "%s".  The old body still evaluates the
    // item name and upgrade arguments before passing them to sprintf.
    const char* const item_name = prototype->GetItemName();
    const int upgrade = item.GetUpgrade();
    (void)item_name;
    (void)upgrade;
    std::sprintf(message, "%s", getCurCharacName(), upgrade, item_name);

    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
    return static_cast<char>(proxy->SendGuildBoardWrite(
        message,
        0,
        getCurCharacNo(),
        -1,
        -1,
        get_charac_guildkey()));
}

// IDA_EA: 0x8671102
// IDA_NAME: _ZN5CUser17_ProcCompoundCoreERKSt6vectorISt4pairIiiESaIS2_EERS4_RS0_IiSaIiEEibP11PacketGuardRbt16stEquipConditionP14CStackableItemR10Inven_Item
// DEMANGLED: CUser::_ProcCompoundCore(std::vector<std::pair<int,int> > const&,std::vector<std::pair<int,int> >&,std::vector<int>&,int,bool,PacketGuard*,bool&,unsigned short,stEquipCondition,CStackableItem*,Inven_Item&)
int CUser::_ProcCompoundCore(
    const std::vector<std::pair<int, int> >& materials,
    std::vector<std::pair<int, int> >& results,
    std::vector<int>& changed_slots,
    int recipe_slot,
    bool avatar_recipe,
    PacketGuard* packet,
    bool& equipment_changed,
    unsigned short compound_count,
    stEquipCondition condition,
    CStackableItem* recipe,
    Inven_Item& material) {
    std::vector<int> deleted_slots;
    std::vector<int> inserted_slots;
    const int deleted = getCurCharacInvenW()->delete_event_items(
        materials,
        deleted_slots,
        equipment_changed,
        static_cast<eMoneySubReason>(kCompoundMoneySubReason),
        static_cast<eItemDelReason>(kCompoundDeleteReason),
        avatar_recipe);
    if (!deleted) {
        TraceCompoundStage(this, recipe->get_index(), recipe_slot,
                           avatar_recipe, compound_count, 0,
                           "commit_delete_materials", 17);
        return 0;
    }

    if (avatar_recipe) {
        packet->put_byte(deleted);
    } else {
        packet->put_byte(deleted + compound_count);
        if (getCurCharacInvenW()->delete_item(
                static_cast<INVEN_TYPE>(1), recipe_slot, compound_count,
                static_cast<eItemDelReason>(kCompoundDeleteReason), true) != 1) {
            TraceCompoundStage(this, recipe->get_index(), recipe_slot,
                               avatar_recipe, compound_count, 0,
                               "commit_delete_recipe", 17);
            return 0;
        }
        packet->put_byte(0);
        packet->put_short(recipe_slot);
        packet->put_int(compound_count);
    }
    PutDeletedItems(*packet, deleted_slots);

    const bool succeeded = isSuccessCompound(recipe->recipe_info().value_34);
    if (!succeeded) {
        const_cast<RecipeInfo&>(recipe->recipe_info()).changeOutput(results);
    }
    if (succeeded || !results.empty()) {
        const int inserted = getCurCharacInvenW()->insert_event_items(
            results,
            inserted_slots,
            static_cast<eMoneyAddReason>(kCompoundMoneyAddReason),
            static_cast<eItemAddReason>(kCompoundItemAddReason),
            false,
            "",
            "");
        packet->put_byte(inserted);
        if (inserted != static_cast<int>(results.size())) {
            TraceCompoundStage(this, recipe->get_index(), recipe_slot,
                               avatar_recipe, compound_count, 0,
                               "commit_insert_outputs", 17);
            return 0;
        }

        for (std::vector<int>::const_iterator it = inserted_slots.begin();
             it != inserted_slots.end();) {
            const int slot = *it++;
            const int item_id = *it++;
            const int item_count = *it++;
            Inven_Item output = getCurCharacInvenR()->GetInvenSlot(1, slot);
            CItem* const output_item = G_CDataManager()->find_item(output.item_id_);
            if (output.item_category_ == 1 && output_item) {
                if (condition.has_equipment_material) {
                    if (succeeded) {
                        if (recipe->recipe_info().value_3c == 1) {
                            copyItemOption(output, material);
                        } else {
                            setRecipeEqiupmentOption(
                                output,
                                condition,
                                slot,
                                recipe,
                                output_item->vtable_->get_item_type(output_item));
                        }
                        if (recipe->recipe_info().value_38) {
                            compoundSystemMessage(output);
                            compoundGuildMessage(output);
                        }
                    } else if (recipe->recipe_info().value_3c == 1) {
                        copyItemOption(output, material);
                    }
                    getCurCharacInvenW()->update_item(
                        static_cast<INVEN_TYPE>(1), slot, output);
                } else if (output_item->get_rarity() != 5 &&
                           output_item->vtable_->get_item_type(output_item) != 11 &&
                           recipe->GetNeedLevel() > 0) {
                    CItemMakingSkill* const skill =
                        static_cast<CItemMakingSkill*>(GetCharacExpandData(
                            static_cast<ENUM_CHARAC_EXPAND_TYPE>(10)));
                    if (!skill) {
                        LogManager::logFormat(
                            1,
                            "user.cpp",
                            "bool CUser::_ProcCompoundCore(const IntPairVector&, IntPairVector&, IntVector&, int, bool, PacketGuard*, bool&, RequestItemCountType, stEquipCondition, CStackableItem*, Inven_Item&)",
                            0x5D0C,
                            "item_making_skill_level error : %d",
                            -1);
                        TraceCompoundStage(this, recipe->get_index(), recipe_slot,
                                           avatar_recipe, compound_count, 0,
                                           "commit_item_making_skill_missing", 17);
                        return 0;
                    }
                    const int skill_level =
                        skill->GetItemMakingSkillLevel(recipe, this);
                    stItemMakingSkill* const table =
                        G_CDataManager()->GetItemMakingSkill();
                    if (!table) {
                        LogManager::logFormat(
                            1,
                            "user.cpp",
                            "bool CUser::_ProcCompoundCore(const IntPairVector&, IntPairVector&, IntVector&, int, bool, PacketGuard*, bool&, RequestItemCountType, stEquipCondition, CStackableItem*, Inven_Item&)",
                            0x5D0C,
                            "item_making_skill_level error : %d",
                            skill_level);
                        TraceCompoundStage(this, recipe->get_index(), recipe_slot,
                                           avatar_recipe, compound_count, 0,
                                           "commit_item_making_table_missing", 17);
                        return 0;
                    }
                    const std::map<int, stUpgradeWithCompound>::const_iterator
                        rule = table->upgrade_with_compound.find(skill_level);
                    if (rule == table->upgrade_with_compound.end()) {
                        // Preserve the official 0x8671102 diagnostic.  The
                        // ordinary CMD25 path now validates the same immutable
                        // table before deleting anything, so a valid request
                        // cannot first discover this condition after commit.
                        LogManager::logFormat(
                            1,
                            "user.cpp",
                            "bool CUser::_ProcCompoundCore(const IntPairVector&, IntPairVector&, IntVector&, int, bool, PacketGuard*, bool&, RequestItemCountType, stEquipCondition, CStackableItem*, Inven_Item&)",
                            0x5D0C,
                            "item_making_skill_level error : %d",
                            skill_level);
                        TraceCompoundStage(this, recipe->get_index(), recipe_slot,
                                           avatar_recipe, compound_count, 0,
                                           "commit_item_making_rule_missing", 17);
                        return 0;
                    }
                    int upgrade = 0;
                    if (!recipe->recipe_info().flag_25) {
                        if (get_rand_int(10000) >= rule->second.compound_exp) {
                            upgrade = rule->second.min_upgrade_value +
                                get_rand_int(rule->second.upgrade_range) / 1000;
                        } else {
                            upgrade = rule->second.min_compound_value +
                                get_rand_int(rule->second.compound_range) / 1000;
                        }
                    }
                    if (upgrade >= 32) {
                        upgrade = 0;
                    }
                    output.SetUpgrade(static_cast<uint8_t>(upgrade));
                    if (output.GetUpgrade()) {
                        getCurCharacInvenW()->update_item(
                            static_cast<INVEN_TYPE>(1), slot, output);
                    }
                }
            }
            changed_slots.push_back(slot);
            packet->put_byte(0);
            packet->put_short(slot);
            packet->put_int(item_id);
            packet->put_int(item_count);
            packet->put_byte(output.GetUpgrade());
            packet->put_short(output.avatar_select_);
            packet->put_byte(output.amplify_option_.getAbilityType());
            packet->put_short(output.amplify_option_.getAbilityValue());
            packet->put_byte(output.seal_or_state_ != 0);
            packet->put_packet(output);
            if (avatar_recipe) {
                GetInstanceCubeStatistic()->collectCubeStatistics(
                    item_id, item_count, this,
                    static_cast<CUBE_STATISTIC_FIELD>(3));
            }
        }
    } else {
        packet->put_byte(0);
    }
    return 1;
}

// IDA_EA: 0x868C45E
// IDA_NAME: _ZN5CUser27is_recipe_items_avatar_typeERKSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: CUser::is_recipe_items_avatar_type(std::vector<std::pair<int,int> > const&)
int CUser::is_recipe_items_avatar_type(
    const std::vector<std::pair<int, int> >& items) {
    for (std::vector<std::pair<int, int> >::const_iterator it = items.begin();
         it != items.end();
         ++it) {
        CItem* const item = G_CDataManager()->find_item(it->first);
        if (item && item->vtable_->is_avatar_item(item)) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x868C4F4
// IDA_NAME: _ZN5CUser27process_recipe_avatar_itemsEibtiPK14CStackableItem
// DEMANGLED: CUser::process_recipe_avatar_items(int,bool,unsigned short,int,CStackableItem const*)
int CUser::process_recipe_avatar_items(int recipe_slot,
                                       bool avatar_recipe,
                                       unsigned short compound_count,
                                       int command_id,
                                       const CStackableItem* recipe) {
    if (!recipe || avatar_recipe || recipe->recipe_info().pairs_0c.size() > 1) {
        return avatar_recipe ? 19 : 17;
    }

    std::vector<std::pair<int, int> > materials(recipe->recipe_info().pairs_00);
    std::vector<std::pair<int, int> > outputs(recipe->recipe_info().pairs_0c);
    std::vector<int> changes;
    CInventory preview;
    InventoryMemory preview_memory;
    preview.SetInventoryMemory(&preview_memory);
    preview.setCopy(*getCurCharacInvenRefR());
    if (preview.delete_item(static_cast<INVEN_TYPE>(1), recipe_slot,
                            compound_count,
                            static_cast<eItemDelReason>(kCompoundDeleteReason),
                            false) != 1) {
        return 17;
    }
    changes.push_back(0);
    changes.push_back(recipe_slot);
    changes.push_back(compound_count);

    for (std::vector<std::pair<int, int> >::const_iterator it = materials.begin();
         it != materials.end();
         ++it) {
        CItem* const material = G_CDataManager()->find_item(it->first);
        if (!material) {
            continue;
        }
        if (material->vtable_->is_avatar_item(material)) {
            const int slot = preview.get_avatar_slot_no(it->first);
            if (slot == -1 || CheckItemLock(2, slot)) {
                return 213;
            }
            changes.push_back(1);
            changes.push_back(slot);
            changes.push_back(1);
        } else {
            const int slot = preview.get_inven_slot_no(it->first);
            if (slot == -1) {
                return 213;
            }
            std::vector<std::pair<int, int> > one_item(1, *it);
            if (!preview.tryDeleteEventItems(one_item)) {
                return 17;
            }
            changes.push_back(0);
            changes.push_back(slot);
            changes.push_back(it->second);
        }
    }

    CItem* const output = G_CDataManager()->find_item(outputs[0].first);
    if (!output) {
        return 17;
    }

    CInventory* const inventory = getCurCharacInvenW();
    for (std::vector<int>::const_iterator it = changes.begin();
         it != changes.end();) {
        const int kind = *it++;
        const int slot = *it++;
        const int count = *it++;
        if (kind == 1) {
            const Inven_Item avatar = getCurCharacInvenR()->GetInvenSlot(2, slot);
            if (inventory->delete_item(static_cast<INVEN_TYPE>(2), slot, 1,
                                       static_cast<eItemDelReason>(kAvatarDeleteReason),
                                       true) != 1) {
                return 22;
            }
            inventory->GetAvatarItemMgrW()->UnRegistItem(avatar.add_info_);
            inventory->SendAvatarEvent(
                54,
                avatar.add_info_,
                1,
                slot + oldsrv::config::kAvatarInventorySlotBase,
                0);
        } else if (inventory->delete_item(static_cast<INVEN_TYPE>(1), slot,
                                          count,
                                          static_cast<eItemDelReason>(kCompoundDeleteReason),
                                          true) != 1) {
            return 17;
        }
    }

    char ipg_no[24] = {};
    const char* const unique_no = WongWork::CGenUniqueNo::genIPGNo(
        static_cast<WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE>(7),
        getCurCharacNo(),
        ipg_no);
    const int output_slot = inventory->AddAvatarItem(
        output->get_index(), output->getUsablePeriod(), false, 0, -1, unique_no,
        static_cast<eAvatarItemAddReason>(2), false, 0);
    if (output_slot == -1) {
        return 4;
    }

    const Inven_Item added = getCurCharacInvenR()->GetInvenSlot(2, output_slot);
    PacketGuard packet;
    packet.put_header(1, command_id);
    packet.put_byte(1);
    packet.put_byte(static_cast<int>(changes.size() / 3));
    PutDeletedItems(packet, changes);
    packet.put_byte(1);
    packet.put_byte(1);
    packet.put_short(output_slot);
    packet.put_int(added.item_id_);
    packet.put_int(added.add_info_);
    packet.put_byte(added.GetUpgrade());
    packet.put_short(added.avatar_select_);
    packet.put_byte(added.amplify_option_.getAbilityType());
    packet.put_short(added.amplify_option_.getAbilityValue());
    packet.put_byte(added.seal_or_state_ != 0);
    packet.put_packet(added);
    packet.finalize(true);
    Send(packet);
    SendUpdateItemList(static_cast<eSendTarget>(1),
                       static_cast<ENUM_ITEMSPACE>(1), output_slot);
    HistoryLog::WriteComposition(history_stream(), recipe->get_index());
    return 0;
}

// IDA_EA: 0x867218C
// IDA_NAME: _ZN5CUser13compound_itemEibti
// DEMANGLED: CUser::compound_item(int,bool,unsigned short,int)
int CUser::compound_item(int recipe_slot,
                         bool avatar_recipe,
                         unsigned short compound_count,
                         int command_id) {
    unsigned int requested_recipe_id =
        avatar_recipe ? static_cast<unsigned int>(recipe_slot) : 0u;
    if (!avatar_recipe && getCurCharacInvenR()) {
        requested_recipe_id = static_cast<unsigned int>(
            getCurCharacInvenR()->GetInvenSlot(1, recipe_slot).item_id_);
    }
    TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                       avatar_recipe, compound_count, command_id,
                       "owner_ingress", 0);

    int recipe_add_info = 0;
    CStackableItem* const recipe = _GetRecipeItem(
        static_cast<unsigned long>(recipe_slot), avatar_recipe, recipe_add_info);
    if (!recipe) {
        TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                           avatar_recipe, compound_count, command_id,
                           "recipe_lookup", 17);
        return 17;
    }
    requested_recipe_id = recipe->get_index();
    const bool ordinary_command =
        command_id == ENUM_CMDPACKET_COMPOUND_ITEM;
    const bool expert_command =
        command_id == ENUM_CMDPACKET_COMPOUND_ITEM_BY_EXPERT_JOB;
    const bool expert_recipe = recipe->recipe_info().flag_25 != 0;
    if ((!ordinary_command && !expert_command) ||
        expert_command != expert_recipe) {
        TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                           avatar_recipe, compound_count, command_id,
                           "command_recipe_route", 17);
        return 17;
    }
    if (is_recipe_items_avatar_type(recipe->recipe_info().pairs_0c)) {
        const int avatar_result = process_recipe_avatar_items(
            recipe_slot, avatar_recipe, compound_count, command_id, recipe);
        TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                           avatar_recipe, compound_count, command_id,
                           "avatar_recipe", avatar_result);
        return avatar_result;
    }
    if (!_CheckCompoundSkill(recipe, avatar_recipe)) {
        TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                           avatar_recipe, compound_count, command_id,
                           "skill_requirement", 22);
        return 22;
    }

    std::vector<std::pair<int, int> > original_materials(
        recipe->recipe_info().pairs_00);
    if (!recipe->recipe_info().pairs_28.empty()) {
        original_materials.insert(original_materials.end(),
                                  recipe->recipe_info().pairs_28.begin(),
                                  recipe->recipe_info().pairs_28.end());
    }
    std::vector<std::pair<int, int> > materials(original_materials);
    if (command_id == ENUM_CMDPACKET_COMPOUND_ITEM_BY_EXPERT_JOB) {
        CalculateExpertJobMaterials(materials);
    }
    std::vector<std::pair<int, int> > results(recipe->recipe_info().pairs_0c);
    const std::vector<std::pair<int, int> > failure_results(
        recipe->recipe_info().pairs_40);
    _CalcItemVectorMultiply(materials, compound_count);
    _CalcItemVectorMultiply(results, compound_count);
    TraceCompoundPlan(requested_recipe_id, "material", materials);
    TraceCompoundPlan(requested_recipe_id, "success_output", results);
    TraceCompoundPlan(requested_recipe_id, "failure_output",
                      failure_results);
    TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                       avatar_recipe, compound_count, command_id,
                       "material_check_begin", 0);
    if (!getCurCharacInvenR()->CheckNeedItemList(materials)) {
        TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                           avatar_recipe, compound_count, command_id,
                           "material_or_money_check", 21);
        return 21;
    }
    TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                       avatar_recipe, compound_count, command_id,
                       "material_check_pass", 0);

    stEquipCondition condition;
    Inven_Item material;
    TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                       avatar_recipe, compound_count, command_id,
                       "equipment_material_check_begin", 0);
    int result = getCurCharacInvenR()->CheckEquipmentItemCondition(
        materials, condition, material);
    if (result) {
        TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                           avatar_recipe, compound_count, command_id,
                           "equipment_material_check", result);
        return result;
    }
    TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                       avatar_recipe, compound_count, command_id,
                       "equipment_material_check_pass", 0);

    // The official post-insert branch uses the recipe's item-making type to
    // roll the output upgrade.  Validate every immutable dependency before
    // the preview/commit boundary, rather than first discovering a missing
    // expansion/table/rule after materials and the recipe have been deleted.
    TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                       avatar_recipe, compound_count, command_id,
                       "item_making_preflight_begin", 0);
    if (!PreflightItemMakingPostProcess(this, recipe)) {
        TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                           avatar_recipe, compound_count, command_id,
                           "item_making_preflight", 17);
        return 17;
    }
    TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                       avatar_recipe, compound_count, command_id,
                       "item_making_preflight_pass", 0);

    int blank_slots[11] = {};
    int needed_slots[11] = {};
    int present_slots[11] = {};
    int empty_slots = getCurCharacInvenR()->QuickEmptyCount();
    getCurCharacInvenR()->CalBlankSlot(blank_slots, materials);
    if (!avatar_recipe && recipe_add_info == 1) {
        ++blank_slots[2];
    }
    getCurCharacInvenR()->CalNeedSlot(needed_slots, original_materials);
    empty_slots += blank_slots[0];
    for (int type = 1; type <= 10; ++type) {
        if (needed_slots[type] > present_slots[type] + blank_slots[type]) {
            empty_slots += blank_slots[type] + present_slots[type] -
                needed_slots[type];
            if (empty_slots < 0) {
                TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                                   avatar_recipe, compound_count, command_id,
                                   "capacity_check", 4);
                return 4;
            }
        }
    }

    STItemScript::SEXPERTJOB::stExpertCompoundResultVariation variation;
    const int expert_job_level = GetCurExpertJobLevel(GetCurCharacExpertJobExp());
    getCurCharacInvenR()->GetExpertJobCompoundResultVariation(
        static_cast<ENUM_EXPERT_JOB_TYPE>(GetCurCharacExpertJobType()),
        expert_job_level,
        variation);

    bool has_non_stackable_result = false;
    std::vector<std::pair<int, int> > preview_results(results);
    if (variation.value_04 > 0.0f) {
        for (std::vector<std::pair<int, int> >::const_iterator it =
                 preview_results.begin();
             it != preview_results.end();
             ++it) {
            CItem* const item = G_CDataManager()->find_item(it->first);
            if (!item || !item->is_stackable() ||
                static_cast<CStackableItem*>(item)->getStackableLimit() == 1) {
                has_non_stackable_result = true;
            }
        }
        if (!has_non_stackable_result && variation.value_00 > 0.0f) {
            const float result_multiplier = variation.value_04 / 100.0f;
            for (std::vector<std::pair<int, int> >::iterator it =
                     preview_results.begin();
                 it != preview_results.end();
                 ++it) {
                const int base_count = it->second / compound_count;
                const int extra_per_success = static_cast<int>(
                    base_count * result_multiplier - base_count);
                if (extra_per_success > 0) {
                    const int64_t maximum_count =
                        static_cast<int64_t>(it->second) +
                        static_cast<int64_t>(compound_count) *
                            extra_per_success;
                    // A corrupt expert modifier must not wrap the preview
                    // below the real count and then pass the capacity gate.
                    it->second = maximum_count <= 0x7fffffffLL ?
                        static_cast<int>(maximum_count) : 0;
                }
            }
        }
    }

    const int success_insert_error =
        variation.value_04 > 0.0f && !has_non_stackable_result ? 100 : 4;
    const int success_preview = PreviewCompoundOutputBranch(
        this, materials, preview_results, recipe_slot, avatar_recipe,
        compound_count, requested_recipe_id, "success", success_insert_error);
    if (success_preview) {
        TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                           avatar_recipe, compound_count, command_id,
                           "preview_success_branch", success_preview);
        return success_preview;
    }

    // Official CUser::compound_item @ 0x867218C previews only pairs_0c, the
    // success branch.  pairs_40 is selected later by RecipeInfo::changeOutput
    // after the real success roll.  In equipment-conversion recipes (for
    // example a base weapon -> upgraded weapon recipe), pairs_40 commonly
    // describes restoration of the consumed base equipment.  Treating that
    // restoration vector as an independent newly-created output during the
    // speculative inventory pass can reject an otherwise valid ordinary
    // recipe.  Keep the failure vector visible in PLAN diagnostics, but do
    // not add a second transaction gate which the official flow never had.

    PacketGuard packet;
    bool equipment_changed = false;
    packet.put_header(1, command_id);
    packet.put_byte(1);
    if (recipe->recipe_info().flag_25) {
        packet.put_byte(static_cast<int>(recipe->recipe_info().pairs_0c.size()));
        for (std::vector<std::pair<int, int> >::const_iterator it =
                 recipe->recipe_info().pairs_0c.begin();
             it != recipe->recipe_info().pairs_0c.end();
             ++it) {
            packet.put_int(it->first);
            packet.put_int(it->second);
        }

        int total_exp = 0;
        int success_count = 0;
        int extra_success_count = 0;
        const float result_multiplier = variation.value_04 / 100.0f;
        for (int count = 0; count < compound_count; ++count) {
            int gained_exp = 0;
            if (expert_job::CExpertJob::IsSuccess(this, recipe, gained_exp)) {
                total_exp += gained_exp;
                if (variation.value_00 > get_rand_int(100) &&
                    !has_non_stackable_result) {
                    ++extra_success_count;
                }
                ++success_count;
            }
        }
        if (!success_count) {
            results.clear();
        } else {
            expert_job::CExpertJob::IncreaseExpertJobExp(
                this, GetCurCharacExpertJobExp() + total_exp);
            for (std::vector<std::pair<int, int> >::iterator it = results.begin();
                 it != results.end();
                 ++it) {
                const int base_count = it->second / compound_count;
                it->second = success_count * base_count;
                if (extra_success_count) {
                    it->second += extra_success_count *
                        static_cast<int>(base_count * result_multiplier - base_count);
                }
                GetInstanceExpertJobStatistic()->increaseItemCount(
                    static_cast<ENUM_EXPERT_JOB_TYPE>(
                        GetCurCharacExpertJobType()),
                    it->first,
                    it->second);
            }
        }
        packet.put_int(success_count);
        packet.put_int(compound_count - success_count);
        packet.put_byte(extra_success_count);
    }

    std::vector<int> changed_slots;
    if (!_ProcCompoundCore(materials, results, changed_slots, recipe_slot,
                           avatar_recipe, &packet, equipment_changed,
                           compound_count, condition, recipe, material)) {
        TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                           avatar_recipe, compound_count, command_id,
                           "commit", 17);
        return 17;
    }
    packet.finalize(true);
    Send(packet);
    for (std::vector<int>::const_iterator it = changed_slots.begin();
         it != changed_slots.end();
         ++it) {
        if (*it >= 0 && getCurCharacInvenR()->GetInvenSlot(1, *it).item_category_ == 1) {
            SendUpdateItem(static_cast<eSendTarget>(1),
                           static_cast<ENUM_ITEMSPACE>(0), *it);
        }
    }
    if (equipment_changed) {
        send_equip(oldsrv::config::kEquipmentTypeCreature);
    }
    _onCompoundItem(recipe->get_index());
    if (recipe->GetNeedLevel() > 0) {
        static_cast<CItemMakingSkill*>(GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(10)))->GainItemMakingSkillPoint(
                recipe, this);
    }
    HistoryLog::WriteComposition(history_stream(), recipe->get_index());
    TraceCompoundStage(this, requested_recipe_id, recipe_slot,
                       avatar_recipe, compound_count, command_id,
                       "success", 0);
    return 0;
}

// IDA_EA: 0x86646C2
// IDA_NAME: _ZN5CUser15_onCompoundItemEi
// DEMANGLED: CUser::_onCompoundItem(int)
void CUser::_onCompoundItem(int item_index) {
    (void)item_index;
}
