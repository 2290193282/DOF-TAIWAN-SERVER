#include "old_data/PVF/event/event_reward_script.h"

#include "common/config.h"
#include "core/config/env/environment.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_data.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/common/script_file_list.h"
#include "old_data/PVF/equipment/random_option_item_handle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "common/manager/os_api.h"

#include <cstring>
#include <utility>


// IDA_EA: 0x80F1134
// IDA_NAME: _ZN8itemDataC2Ev
// DEMANGLED: itemData::itemData(void)
itemData::itemData()
    : upgrade(0),
      seal(1),
      random_option_seal(1),
      item_index(0),
      count_or_period(0) {}

// IDA_EA: 0x80F1162
// IDA_NAME: _ZN8itemData5clearEv
// DEMANGLED: itemData::clear(void)
itemData* itemData::clear() {
    upgrade = 0;
    seal = 1;
    random_option_seal = 1;
    item_index = 0;
    count_or_period = 0;
    return this;
}

// IDA_EA: 0x80F32FA
// IDA_NAME: _ZN8itemDataD2Ev
// DEMANGLED: itemData::~itemData()
itemData::~itemData() {}

// IDA_EA: 0x80EF0C0
// IDA_NAME: _ZN11eventReward13getSubKeyTypeERSs
// DEMANGLED: eventReward::getSubKeyType(std::string &)
int eventReward::getSubKeyType(std::string& text) {
    if (text == "common") {
        return 0;
    }
    if (text == "level") {
        return 1;
    }
    if (text == "fatigue") {
        return 2;
    }
    return 0;
}

// IDA_EA: 0x80EF128
// IDA_NAME: _ZN11eventReward15getSubKeyActionERSs
// DEMANGLED: eventReward::getSubKeyAction(std::string &)
int eventReward::getSubKeyAction(std::string& text) {
    if (text == "equip") {
        return 2;
    }
    if (text == "inven") {
        return 1;
    }
    if (text == "postal") {
        return 3;
    }
    return 0;
}

// IDA_EA: 0x80EF190
// IDA_NAME: _ZN11eventReward13getOptionTypeERSsR8itemData
// DEMANGLED: eventReward::getOptionType(std::string &,itemData &)
int eventReward::getOptionType(std::string& text, itemData& data) {
    bool success = true;
    if (text == "unseal") {
        data.seal = 0;
    } else if (text == "seal") {
        data.seal = 1;
    } else if (text == "upgrade") {
        data.upgrade = static_cast<uint8_t>(ScanInt(&success));
        if (!success) {
            return 0;
        }
    } else if (text == "unsealROI") {
        data.random_option_seal = 0;
    } else if (text == "sealROI") {
        data.random_option_seal = 1;
    }
    return success;
}

// IDA_EA: 0x80EF25C
// IDA_NAME: _ZN11eventReward13defaultOptionEcR8itemData
// DEMANGLED: eventReward::defaultOption(char,itemData &)
itemData* eventReward::defaultOption(char sub_key_action, itemData& data) {
    if (sub_key_action == 2) {
        data.count_or_period = 1;
        data.seal = 0;
        data.random_option_seal = 0;
    }
    if (data.upgrade == 0) {
        data.upgrade = 1;
    }
    return &data;
}

// IDA_EA: 0x80EF29C
// IDA_NAME: _ZN11eventReward17importRewardTableEPKcS1_
// DEMANGLED: eventReward::importRewardTable(char const*,char const*)
int eventReward::importRewardTable(const char* dir, const char* filename) {
    STScriptFileList script_file_list;
    if (loadRDARScriptListFile(&script_file_list, dir, filename) == 1) {
        for (STScriptFileList::iterator it = script_file_list.begin();
             it != script_file_list.end();
             ++it) {
            if (ImportRewardScript(dir, it->second.c_str()) == 0) {
                return 113;
            }
        }
        return 0;
    }
    return 119;
}

// IDA_EA: 0x80EF3A2
// IDA_NAME: _ZN11eventReward18ImportRewardScriptEPKcS1_
// DEMANGLED: eventReward::ImportRewardScript(char const*,char const*)
int eventReward::ImportRewardScript(const char* dir, const char* filename) {
    if (loadRDARScriptFile(dir, filename) != 1) {
        return 0;
    }

    std::string tag;
    bool success = false;
    rewardItemMap item_map;
    rewardKey key;
    itemData data;

    while (ScanType(tag, true) == 1) {
        if (tag == "[/reward]") {
            return 1;
        }

        if (tag == "[job type]") {
            std::string job_text;
            if (ScanStr(&job_text) != 1) {
                return 0;
            }
            key.first =
                static_cast<uint8_t>(getCharacterJobToEnumValue(job_text));
            key.second = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
            key.third = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return 0;
            }
        } else if (tag == "[sub type]") {
            std::string sub_key_type;
            std::string sub_key_action;
            if (ScanStr(&sub_key_type) != 1) {
                return 0;
            }
            key.sub_key.first =
                static_cast<uint8_t>(getSubKeyType(sub_key_type));
            if (ScanStr(&sub_key_action) != 1) {
                return 0;
            }
            key.sub_key.second =
                static_cast<uint8_t>(getSubKeyAction(sub_key_action));
        } else if (tag == "[option]") {
            std::string option_text;
            if (ScanStr(&option_text) == 1) {
                getOptionType(option_text, data);
            }
        } else if (tag == "[equipment item]") {
            defaultOption(static_cast<char>(key.sub_key.second), data);
            while (1) {
                itemData row = data;
                const int map_key = ScanInt(&success);
                if (!success) {
                    break;
                }
                row.item_index = ScanInt(&success);
                if (!success) {
                    break;
                }

                rewardItemMap::iterator item_it = item_map.find(map_key);
                if (item_it != item_map.end()) {
                    item_it->second.push_back(row);
                } else {
                    std::vector<itemData> rows;
                    rows.push_back(row);
                    item_map.insert(std::make_pair(map_key, rows));
                }
            }
        } else if (tag == "[stackable item]") {
            defaultOption(static_cast<char>(key.sub_key.second), data);
            while (1) {
                itemData row = data;
                row.count_or_period = 1;
                const int map_key = ScanInt(&success);
                if (!success) {
                    break;
                }
                row.item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                row.count_or_period = ScanInt(&success);
                if (!success) {
                    break;
                }

                rewardItemMap::iterator item_it = item_map.find(map_key);
                if (item_it != item_map.end()) {
                    item_it->second.push_back(row);
                } else {
                    std::vector<itemData> rows;
                    rows.push_back(row);
                    item_map.insert(std::make_pair(map_key, rows));
                }
            }
        } else if (tag == "[/sub type]") {
            reward_table.insert(std::make_pair(key, item_map));
            key.sub_key.clear();
            item_map.clear();
        } else if (tag == "[/job type]") {
            key.clear();
            item_map.clear();
        } else if (tag == "[/option]") {
            data.clear();
        }
    }

    return success;
}

// IDA_EA: 0x83738A2
// IDA_NAME: _ZN11eventRewardC2Ev
// DEMANGLED: eventReward::eventReward(void)
eventReward::eventReward()
    : reward_table(),
      slot_bound_checker() {
    reward_table.clear();
}

// IDA_EA: 0x837DFB2
// IDA_NAME: _ZN11eventRewardD2Ev
// DEMANGLED: eventReward::~eventReward()
eventReward::~eventReward() {}

// IDA_EA: 0x80F0270
// IDA_NAME: _ZN11eventReward23createCharacInvenRewardEP10Inven_Item9rewardKeyi
// DEMANGLED: eventReward::createCharacInvenReward(Inven_Item *,rewardKey,int)
bool eventReward::createCharacInvenReward(Inven_Item* items,
                                          rewardKey key,
                                          int reward_value) {
    bool delivered =
        G_CDataManager()->get_event_reward()->rewardWriteUserInven(
            items, key, reward_value) != 0;

    key.second = 0;
    key.sub_key.second = 1;
    if (G_CDataManager()->get_event_reward()->rewardWriteUserInven(
            items, key, reward_value) != 0) {
        delivered = true;
    }

    key.first = 11;
    if (G_CDataManager()->get_event_reward()->rewardWriteUserInven(
            items, key, reward_value) != 0) {
        return true;
    }
    return delivered;
}

// IDA_EA: 0x80F033C
// IDA_NAME: _ZN11eventReward23createCharacEquipRewardEP10Inven_Item9rewardKeyi
// DEMANGLED: eventReward::createCharacEquipReward(Inven_Item *,rewardKey,int)
bool eventReward::createCharacEquipReward(Inven_Item* items,
                                          rewardKey key,
                                          int reward_value) {
    bool delivered =
        G_CDataManager()->get_event_reward()->rewardWriteUserEquip(
            items, key, reward_value) != 0;

    key.second = 0;
    key.sub_key.second = 2;
    if (G_CDataManager()->get_event_reward()->rewardWriteUserEquip(
            items, key, reward_value) != 0) {
        delivered = true;
    }

    key.first = 11;
    if (G_CDataManager()->get_event_reward()->rewardWriteUserEquip(
            items, key, reward_value) != 0) {
        return true;
    }
    return delivered;
}

// IDA_EA: 0x80F0408
// IDA_NAME: _ZN11eventReward20rewardWriteUserInvenEP10Inven_Item9rewardKeyi
// DEMANGLED: eventReward::rewardWriteUserInven(Inven_Item *,rewardKey,int)
int eventReward::rewardWriteUserInven(Inven_Item* items,
                                      rewardKey key,
                                      int reward_value) {
    if (reward_table.empty()) {
        return 0;
    }

    rewardTable::iterator reward_it = reward_table.find(key);
    if (reward_it == reward_table.end()) {
        return 0;
    }

    rewardItemMap::iterator item_it = reward_it->second.find(reward_value);
    if (item_it == reward_it->second.end() || item_it->second.empty()) {
        return 0;
    }

    for (std::vector<itemData>::const_iterator data_it =
             item_it->second.begin();
         data_it != item_it->second.end();
         ++data_it) {
        const itemData& data = *data_it;
        CItem* const item = G_CDataManager()->find_item(data.item_index);
        if (!item) {
            return 0;
        }

        Inven_Item inventory_item;
        // The raw path stores this id before the item virtual factory fills
        // the canonical inventory record.
        inventory_item.item_id_ = data.item_index;
        item->vtable_->make_item(item, inventory_item);
        applyItemOption(inventory_item, item, data);

        int start = 0;
        int end = 0;
        StartEndPos(
            static_cast<Inven_Item::ITEM_TYPE>(inventory_item.item_category_),
            start,
            end,
            0);
        for (int slot = start; slot <= end; ++slot) {
            if (items[slot].item_id_ == 0) {
                // 0x80F0408 performs the complete packed 0x3d-byte copy,
                // rather than the logical setCopy member operation.
                std::memcpy(&items[slot], &inventory_item,
                            sizeof(Inven_Item));
                break;
            }
        }
    }
    return 1;
}

// IDA_EA: 0x80F06DA
// IDA_NAME: _ZN11eventReward20rewardWriteUserEquipEP10Inven_Item9rewardKeyi
// DEMANGLED: eventReward::rewardWriteUserEquip(Inven_Item *,rewardKey,int)
int eventReward::rewardWriteUserEquip(Inven_Item* items,
                                      rewardKey key,
                                      int reward_value) {
    if (reward_table.empty()) {
        return 0;
    }

    rewardTable::iterator reward_it = reward_table.find(key);
    if (reward_it == reward_table.end()) {
        return 0;
    }

    rewardItemMap::iterator item_it = reward_it->second.find(reward_value);
    if (item_it == reward_it->second.end() || item_it->second.empty()) {
        return 0;
    }

    for (std::vector<itemData>::const_iterator data_it =
             item_it->second.begin();
         data_it != item_it->second.end();
         ++data_it) {
        const itemData& data = *data_it;
        CItem* const item = G_CDataManager()->find_item(data.item_index);
        if (!item) {
            return 0;
        }

        Inven_Item inventory_item;
        inventory_item.item_id_ = data.item_index;
        item->vtable_->make_item(item, inventory_item);
        applyItemOption(inventory_item, item, data);
        if (item->is_stackable()) {
            return 0;
        }

        const int item_type = item->vtable_->get_item_type(item);
        // Callers pass CInventory::GetInvenStart(0), whose raw base is the
        // equipment array. Reward rows are indexed from the normal weapon.
        std::memcpy(&items[
                        item_type -
                        oldsrv::config::kOrdinaryEquipmentTypeFirst],
                    &inventory_item,
                    sizeof(Inven_Item));
    }
    return 1;
}

// IDA_EA: 0x80F0976
// IDA_NAME: _ZN11eventReward16sendPostalRewardER5CUser9rewardKeyiPKcS4_
// DEMANGLED: eventReward::sendPostalReward(CUser &,rewardKey,int,char const*,char const*)
int eventReward::sendPostalReward(CUser& user,
                                  rewardKey key,
                                  int reward_value,
                                  const char* title_string_key,
                                  const char* body_string_key) {
    if (reward_table.empty()) {
        return 0;
    }

    rewardTable::iterator reward_it = reward_table.find(key);
    if (reward_it == reward_table.end()) {
        return 0;
    }

    rewardItemMap::iterator item_it = reward_it->second.find(reward_value);
    if (item_it == reward_it->second.end() || item_it->second.empty()) {
        return 0;
    }

    for (std::vector<itemData>::const_iterator data_it =
             item_it->second.begin();
         data_it != item_it->second.end();
         ++data_it) {
        const itemData& data = *data_it;
        Inven_Item inventory_item;
        CItem* const item = G_CDataManager()->find_item(data.item_index);

        // The integrated PvP-base channel uses the character's group; all
        // other delivery paths use the current server environment group.
        const ENUM_SERVER_GROUP server_group =
            G_GameWorld()->IsIntegratedPvPBaseChannel()
                ? static_cast<ENUM_SERVER_GROUP>(user.GetServerGroup())
                : static_cast<ENUM_SERVER_GROUP>(
                      G_CEnvironment()->get_server_group());
        if (!item) {
            continue;
        }

        inventory_item.item_id_ = data.item_index;
        item->vtable_->make_item(item, inventory_item);
        applyItemOption(inventory_item, item, data);

        char body[256];
        std::memset(body, 0, sizeof(body));
        const char* const format = g_scriptStringManager_.findString(
            4, body_string_key, 0);
        OS_API::snprintf(body, 0xffu, format, user.get_charac_level());

        if (data.item_index != 0 && item->is_stackable()) {
            const int expiration = item->getExpirationDate();
            const char* const title = g_scriptStringManager_.findString(
                4, title_string_key, 0);
            WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
                title,
                inventory_item,
                0,
                static_cast<unsigned int>(user.getCurCharacNo()),
                body,
                static_cast<int>(std::strlen(body)),
                static_cast<unsigned int>(expiration),
                server_group,
                false,
                true);
        }
    }
    return 1;
}

// IDA_EA: 0x80F0D6E
// IDA_NAME: _ZN11eventReward15applyItemOptionER10Inven_ItemPK5CItemRK8itemData
// DEMANGLED: eventReward::applyItemOption(Inven_Item &,CItem const*,itemData const&)
// The original callers ignore EAX and the original instruction stream has no
// semantic result path; the apparent Inven_Item* in decompiler output is
// residual-register inference, so this source restoration intentionally is
// void.
void eventReward::applyItemOption(Inven_Item& inventory_item,
                                  const CItem* item,
                                  const itemData& data) {
    if (item->is_stackable()) {
        inventory_item.add_info_ = data.count_or_period;
        return;
    }

    inventory_item.seal_or_state_ = data.seal;
    const unsigned int attach_type =
        static_cast<unsigned int>(item->GetAttachType());
    if (attach_type <= 5u && ((1u << attach_type) & 0x27u) != 0u) {
        inventory_item.seal_or_state_ = 0;
    }
    if (static_cast<int8_t>(data.upgrade) > 1) {
        inventory_item.SetUpgrade(data.upgrade);
    }
    if (item->vtable_->is_random_option(item) &&
        data.random_option_seal != 1u) {
        unsealRandomOption(item, inventory_item);
    }
}

// IDA_EA: 0x80F0E40
// IDA_NAME: _ZN11eventReward18unsealRandomOptionEPK5CItemR10Inven_Item
// DEMANGLED: eventReward::unsealRandomOption(CItem const*,Inven_Item &)
bool eventReward::unsealRandomOption(const CItem* item,
                                     Inven_Item& inventory_item) {
    if (!item || item->vtable_->is_random_option(item) != 1) {
        return false;
    }

    const int option_grade = item->vtable_->get_random_option_grade(item);
    if (GlobalData::g_randomOptionHandle->give_option(
            static_cast<unsigned long>(inventory_item.item_id_),
            static_cast<ENUM_RARITY>(item->get_rarity()),
            item->getUsableLevel(),
            item->getItemGroupName(),
            option_grade,
            &inventory_item.chronicle_random_option_.random_option_) != 1) {
        LogManager::logFormat(
            1,
            "eventReward.cpp",
            "bool eventReward::unsealRandomOption(const CItem*, Inven_Item&)",
            0x240,
            "autoUnsealItem : %d",
            inventory_item.item_id_);
    }
    return true;
}
