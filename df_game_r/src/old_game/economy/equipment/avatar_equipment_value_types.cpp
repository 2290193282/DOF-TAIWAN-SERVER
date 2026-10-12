#include "old_game/economy/equipment/avatar_equipment_value_types.h"

#include "old_data/PVF/event/in_game_event_pvf_scripts.h"
#include "old_data/PVF/event/action_mail_script.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string.h>


typedef char stAuraAvatarOption_Size_Must_Be_0x0c[
    (sizeof(stAuraAvatarOption) == 0x0c) ? 1 : -1];
typedef char stAvatarEmblemInfo_t_Size_Must_Be_0x1e[
    (sizeof(stAvatarEmblemInfo_t) == 0x1e) ? 1 : -1];
typedef char stAvatarExpansionInfo_t_Size_Must_Be_0x04[
    (sizeof(stAvatarExpansionInfo_t) == 0x04) ? 1 : -1];
typedef char stEquipCondition_Size_Must_Be_0x08[
    (sizeof(stEquipCondition) == 0x08) ? 1 : -1];
typedef char stAvatarTypeSelect_t_Size_Must_Be_0x1e[
    (sizeof(stAvatarTypeSelect_t) == 0x1e) ? 1 : -1];
typedef char AvatarDisjointRandomRewardScript_Size_Must_Be_0x64[
    (sizeof(AvatarDisjointRandomRewardScript) == 0x64) ? 1 : -1];

// IDA_EA: 0x844D616
// IDA_NAME: _ZN18stAuraAvatarOption5resetEv
// DEMANGLED: stAuraAvatarOption::reset(void)
void* stAuraAvatarOption::reset() {
    return memset(this, 0, 0x0cu);
}

// IDA_EA: 0x81506E8
// IDA_NAME: _ZN20stAvatarEmblemInfo_t4initEv
// DEMANGLED: stAvatarEmblemInfo_t::init(void)
bool stAvatarEmblemInfo_t::init() {
    bool result = false;
    for (int n4 = 0;; ++n4) {
        result = n4 <= 4;
        if (n4 > 4) {
            break;
        }
        sockets[n4].socket_type = 0;
        sockets[n4].emblem_item_index = 0;
    }
    return result;
}

// IDA_EA: 0x833421E
// IDA_NAME: _ZN23stAvatarExpansionInfo_t4initEv
// DEMANGLED: stAvatarExpansionInfo_t::init(void)
bool stAvatarExpansionInfo_t::init() {
    bool result = false;
    for (int i = 0;; ++i) {
        result = i <= 1;
        if (i > 1) {
            break;
        }
        avatar_expansion[i] = 0;
    }
    return result;
}

// IDA_EA: 0x86937D6
// IDA_NAME: _ZN16stEquipConditionC2Ev
// DEMANGLED: stEquipCondition::stEquipCondition(void)
stEquipCondition::stEquipCondition()
    : has_equipment_material(0),
      equipment_upgrade(0),
      has_amplify_option(0),
      amplify_ability_type(0),
      amplify_ability_value(0),
      sealed(1),
      reseal_count(0) {}

// IDA_EA: 0x898D1FC
// IDA_NAME: _ZN20stAvatarTypeSelect_t4initEv
// DEMANGLED: stAvatarTypeSelect_t::init(void)
bool stAvatarTypeSelect_t::init() {
    usable_period = 0;
    gold_price = 0;
    win_point_price = 0;
    cera_price = 0;
    socket_count = 0;

    bool result = false;
    for (int n4 = 0;; ++n4) {
        result = n4 <= 4;
        if (n4 > 4) {
            break;
        }
        socket_types[n4] = 0;
    }
    return result;
}

// IDA_EA: 0x89D1348
// IDA_NAME: _ZN20stAvatarTypeSelect_tD2Ev
// DEMANGLED: stAvatarTypeSelect_t::~stAvatarTypeSelect_t()
stAvatarTypeSelect_t::~stAvatarTypeSelect_t() {}



// IDA_EA: 0x8985BE2
// IDA_NAME: _ZN32AvatarDisjointRandomRewardScriptC2Ev
// DEMANGLED: AvatarDisjointRandomRewardScript::AvatarDisjointRandomRewardScript(void)
AvatarDisjointRandomRewardScript::AvatarDisjointRandomRewardScript()
    : InGameEventScript(),
      random_reward_items(),
      send_mail() {
    send_mail.reset();
}


// IDA_EA: 0x89824B4
// IDA_NAME: _Z38importAvatarDisjointRandomRewardScriptP32AvatarDisjointRandomRewardScriptPKc
// DEMANGLED: importAvatarDisjointRandomRewardScript(AvatarDisjointRandomRewardScript *,char const*)
int importAvatarDisjointRandomRewardScript(
    AvatarDisjointRandomRewardScript* script,
    const char* filename) {
    if (!loadRDARScriptFile("Event/", filename)) {
        return 0;
    }

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[/event]") {
            return 1;
        } else if (tag == "[title]") {
            ScanStr(&script->event_name);
        } else if (tag == "[explain]") {
            ScanStr(&script->script_file);
        } else if (tag == "[random reward item]") {
            while (true) {
                const char key =
                    static_cast<char>(ScanInt(&success));
                if (!success) {
                    break;
                }
                const unsigned int item_id =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                script->random_reward_items.insert(
                    std::make_pair(key, item_id));
            }
        } else if (tag == "[send mail]") {
            (void)importActionMailScript(&script->send_mail);
        }
    }

    return success ? 1 : 0;
}
