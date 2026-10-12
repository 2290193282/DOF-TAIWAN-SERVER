#include "old_game/dungeon/tower/tower_of_despair_mgr_runtime.h"

#include <algorithm>
#include <cstring>
#include <string>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/common/dnf_lex.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/dungeon/tower/tower_of_despair_apc_mgr_runtime.h"
#include "old_game/dungeon/instance/tod_layer.h"
#include "old_game/dungeon/instance/tod_user_state_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

// IDA_EA: 0x8643AC4
// IDA_NAME: _ZN17TowerOfDespairMgrC2Ev
// DEMANGLED: TowerOfDespairMgr::TowerOfDespairMgr(void)
TowerOfDespairMgr::TowerOfDespairMgr() {
    enterable_count_ = 0;
    min_level_ = 0;
    top_layer_ = 0;
    ida_align_352_ = 0;
    base_layer_index_ = 0;

    std::fill(dungeons_by_layer_, dungeons_by_layer_ + 101,
              static_cast<const CDungeon*>(0));
    std::fill(dungeon_indices_by_layer_,
              dungeon_indices_by_layer_ + 101,
              0);

    apc_manager_ = new TowerOfDespair_APC_Mgr();
}

// IDA_EA: 0x8643BE2
// IDA_NAME: _ZN17TowerOfDespairMgrD2Ev
// DEMANGLED: TowerOfDespairMgr::~TowerOfDespairMgr()
TowerOfDespairMgr::~TowerOfDespairMgr() {
    delete apc_manager_;
}

// IDA_EA: 0x8643A1E
// IDA_NAME: _ZN17TowerOfDespairMgr8GetMinLvEv
// DEMANGLED: TowerOfDespairMgr::GetMinLv(void)
int TowerOfDespairMgr::GetMinLv() {
    return min_level_;
}

// IDA_EA: 0x8643C6C
// IDA_NAME: _ZN17TowerOfDespairMgr24_Set_LayerByDungeonIndexEjPK8CDungeon
// DEMANGLED: TowerOfDespairMgr::_Set_LayerByDungeonIndex(unsigned int,CDungeon const*)
void TowerOfDespairMgr::_Set_LayerByDungeonIndex(
    unsigned int layer,
    const CDungeon* dungeon) {
    if (layer > 0u && layer <= 100u && dungeon) {
        dungeons_by_layer_[layer] = dungeon;
    }
}

// IDA_EA: 0x8643C86
// IDA_NAME: _ZN17TowerOfDespairMgr24_Set_LayerByDungeonIndexEjj
// DEMANGLED: TowerOfDespairMgr::_Set_LayerByDungeonIndex(unsigned int,unsigned int)
void TowerOfDespairMgr::_Set_LayerByDungeonIndex(
    unsigned int layer,
    unsigned int dungeon_index) {
    if (layer == 0u || layer > 100u || dungeon_index == 0u) {
        return;
    }
    dungeon_indices_by_layer_[layer] = dungeon_index;
    dungeon_indices_.insert(static_cast<int>(dungeon_index));
    _Set_LayerByDungeonIndex(
        layer,
        G_CDataManager()->find_dungeon(static_cast<int>(dungeon_index)));
}

// IDA_EA: 0x8643CF4
// IDA_NAME: _ZN17TowerOfDespairMgr20_Set_APCIndexByLayerEj
// DEMANGLED: TowerOfDespairMgr::_Set_APCIndexByLayer(unsigned int)
void TowerOfDespairMgr::_Set_APCIndexByLayer(unsigned int layer) {
    if (layer <= 100u &&
        std::find(apc_layers_.begin(), apc_layers_.end(), layer) ==
            apc_layers_.end()) {
        apc_layers_.push_back(layer);
    }
}

// IDA_EA: 0x8643E7E
// IDA_NAME: _ZN17TowerOfDespairMgr10LoadScriptEPKcS1_
// DEMANGLED: TowerOfDespairMgr::LoadScript(char const*,char const*)
int TowerOfDespairMgr::LoadScript(const char* directory,
                                  const char* filename) {
    if (loadRDARScriptFile(directory, filename) != true) {
        return 101;
    }

    // 0828 changed this table from the old two-column layout to triples:
    //   layer, dungeon index, title-book quest index (-1 when absent).
    // The title quest is owned by the quest/title-book systems; this ABI
    // manager only consumes it so that the following layer remains aligned.
    // Parse into temporary state and publish only a complete table.
    unsigned int parsed_dungeons[101] = {};
    std::vector<unsigned int> parsed_apc_layers;
    int parsed_enterable_count = -1;
    int parsed_min_level = -1;
    int parsed_top_layer = -1;
    bool saw_layer_table = false;
    bool saw_apc_table = false;

    std::string token;
    while (ScanType(token, true) == 1) {
        if (token == "[enterable count]") {
            bool success = false;
            parsed_enterable_count = ScanInt(&success);
            if (!success || parsed_enterable_count < 0 ||
                parsed_enterable_count > 0xFFFF) {
                unloadRDARScriptFile();
                return 101;
            }
        } else if (token == "[min Level]") {
            bool success = false;
            parsed_min_level = ScanInt(&success);
            if (!success || parsed_min_level < 0 ||
                parsed_min_level > 0xFFFF) {
                unloadRDARScriptFile();
                return 101;
            }
        } else if (token == "[top layer]") {
            bool success = false;
            parsed_top_layer = ScanInt(&success);
            if (!success || parsed_top_layer <= 0 ||
                parsed_top_layer > 100) {
                unloadRDARScriptFile();
                return 101;
            }
        } else if (token == "[each layer matching dungeon]") {
            saw_layer_table = true;
            for (;;) {
                bool is_type = false;
                bool success = false;
                std::string value_or_type;
                const int layer = ScanTypeOrInt(
                    is_type, value_or_type, success, true);
                if (!success) {
                    unloadRDARScriptFile();
                    return 101;
                }
                if (is_type) {
                    if (value_or_type !=
                        "[/each layer matching dungeon]") {
                        unloadRDARScriptFile();
                        return 101;
                    }
                    break;
                }

                const int dungeon_index = ScanTypeOrInt(
                    is_type, value_or_type, success, true);
                if (!success || is_type) {
                    unloadRDARScriptFile();
                    return 101;
                }
                const int title_quest_index = ScanTypeOrInt(
                    is_type, value_or_type, success, true);
                if (!success || is_type || layer <= 0 || layer > 100 ||
                    dungeon_index <= 0 || title_quest_index < -1 ||
                    parsed_dungeons[layer] != 0u ||
                    !G_CDataManager()->find_dungeon(dungeon_index)) {
                    unloadRDARScriptFile();
                    return 101;
                }
                parsed_dungeons[layer] =
                    static_cast<unsigned int>(dungeon_index);
            }
        } else if (token == "[npc layer]") {
            saw_apc_table = true;
            for (;;) {
                bool is_type = false;
                bool success = false;
                std::string value_or_type;
                const int layer = ScanTypeOrInt(
                    is_type, value_or_type, success, true);
                if (!success) {
                    unloadRDARScriptFile();
                    return 101;
                }
                if (is_type) {
                    if (value_or_type != "[/npc layer]") {
                        unloadRDARScriptFile();
                        return 101;
                    }
                    break;
                }
                if (layer < 0 || layer > 100 ||
                    std::find(parsed_apc_layers.begin(),
                              parsed_apc_layers.end(),
                              static_cast<unsigned int>(layer)) !=
                        parsed_apc_layers.end()) {
                    unloadRDARScriptFile();
                    return 101;
                }
                parsed_apc_layers.push_back(
                    static_cast<unsigned int>(layer));
            }
        } else {
            // A consumed list terminator never reaches the outer loop.  Any
            // other type is a schema mismatch, not a field to guess around.
            unloadRDARScriptFile();
            return 101;
        }
    }

    unloadRDARScriptFile();
    if (parsed_enterable_count < 0 || parsed_min_level < 0 ||
        parsed_top_layer <= 0 || !saw_layer_table || !saw_apc_table) {
        return 101;
    }
    for (int layer = 1; layer <= parsed_top_layer; ++layer) {
        if (parsed_dungeons[layer] == 0u) {
            return 101;
        }
    }
    for (int layer = parsed_top_layer + 1; layer <= 100; ++layer) {
        if (parsed_dungeons[layer] != 0u) {
            return 101;
        }
    }
    for (std::vector<unsigned int>::const_iterator it =
             parsed_apc_layers.begin();
         it != parsed_apc_layers.end(); ++it) {
        if (*it > static_cast<unsigned int>(parsed_top_layer)) {
            return 101;
        }
    }

    std::fill(dungeons_by_layer_, dungeons_by_layer_ + 101,
              static_cast<const CDungeon*>(0));
    std::fill(dungeon_indices_by_layer_,
              dungeon_indices_by_layer_ + 101,
              0u);
    dungeon_indices_.clear();
    apc_layers_.clear();
    base_layer_index_ = 0;
    enterable_count_ = static_cast<uint16_t>(parsed_enterable_count);
    min_level_ = static_cast<uint16_t>(parsed_min_level);
    top_layer_ = static_cast<uint16_t>(parsed_top_layer);
    for (int layer = 1; layer <= parsed_top_layer; ++layer) {
        _Set_LayerByDungeonIndex(
            static_cast<unsigned int>(layer), parsed_dungeons[layer]);
    }
    apc_layers_.swap(parsed_apc_layers);
    std::sort(apc_layers_.begin(), apc_layers_.end());
    return 0;
}

// IDA_EA: 0x8643D14
// IDA_NAME: _ZN17TowerOfDespairMgr19Get_CDungeonByLayerEj
// DEMANGLED: TowerOfDespairMgr::Get_CDungeonByLayer(unsigned int)
const CDungeon* TowerOfDespairMgr::Get_CDungeonByLayer(unsigned int layer) {
    if (layer == 0 || layer > top_layer_ || layer > 100) {
        return 0;
    }
    if (!dungeons_by_layer_[layer]) {
        dungeons_by_layer_[layer] =
            G_CDataManager()->find_dungeon(
                static_cast<int>(dungeon_indices_by_layer_[layer]));
    }
    return dungeons_by_layer_[layer];
}

// IDA_EA: 0x8643D76
// IDA_NAME: _ZN17TowerOfDespairMgr23Get_DungeonIndexByLayerEj
// DEMANGLED: TowerOfDespairMgr::Get_DungeonIndexByLayer(unsigned int)
int TowerOfDespairMgr::Get_DungeonIndexByLayer(unsigned int layer) const {
    return layer != 0 && layer <= top_layer_ && layer <= 100
               ? static_cast<int>(dungeon_indices_by_layer_[layer])
               : 0;
}

// IDA_EA: 0x8643D9C
// IDA_NAME: _ZN17TowerOfDespairMgr19Get_APCIndexByLayerEj
// DEMANGLED: TowerOfDespairMgr::Get_APCIndexByLayer(unsigned int)
int TowerOfDespairMgr::Get_APCIndexByLayer(unsigned int layer) const {
    if (layer > 100) {
        return 0;
    }
    return std::find(apc_layers_.begin(), apc_layers_.end(), layer) !=
                   apc_layers_.end()
               ? static_cast<int>(layer)
               : -1;
}

// IDA_EA: 0x8643E4A
// IDA_NAME: _ZN17TowerOfDespairMgr18Get_APCNameByLayerERK9TOD_Layer
// DEMANGLED: TowerOfDespairMgr::Get_APCNameByLayer(TOD_Layer const&)
const char* TowerOfDespairMgr::Get_APCNameByLayer(const TOD_Layer& layer) const {
    return apc_manager_ ? apc_manager_->Get_APC_Name_byIndex(layer) : 0;
}

// IDA_EA: 0x86440A0
// IDA_NAME: _ZN17TowerOfDespairMgr17GetBaseLayerIndexEv
// DEMANGLED: TowerOfDespairMgr::GetBaseLayerIndex(void)
int TowerOfDespairMgr::GetBaseLayerIndex() {
    if (!base_layer_index_) {
        const CDungeon* const first_layer = Get_CDungeonByLayer(1);
        if (first_layer) {
            base_layer_index_ = static_cast<unsigned int>(first_layer->get_index());
        }
    }
    return static_cast<int>(base_layer_index_);
}

// IDA_EA: 0x86440EE
// IDA_NAME: _ZN17TowerOfDespairMgr14CheckEnterableEP5CUserPK8CDungeon
// DEMANGLED: TowerOfDespairMgr::CheckEnterable(CUser *,CDungeon const*)
int TowerOfDespairMgr::CheckEnterable(CUser* user, const CDungeon* dungeon) {
    if (!user || !dungeon) {
        return 21;
    }
    if (user->get_charac_level() < min_level_) {
        return 14;
    }

    TOD_UserState* const user_state = static_cast<TOD_UserState*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(13)));
    if (!user_state) {
        return 21;
    }
    if (user_state->getEnterCount() > 0) {
        if (user->isGMUser() != 1 &&
            !CheckDailyScheduleTime(
                G_CEnvironment()->get_fatigue_time(),
                user_state->getLastClearTime(),
                GlobalData::system_time()->getCurSec())) {
            return 245;
        }

        const int inventory_slot =
            user->getCurCharacInvenR()->check_item_exist(dungeon->required_item_.item_id);
        if (inventory_slot == -1) {
            return 245;
        }
        const Inven_Item item =
            user->getCurCharacInvenR()->GetInvenSlot(1, inventory_slot);
        if (item.add_info_ < dungeon->required_item_.item_count) {
            return 17;
        }
    }

    CParty* const party = user->GetParty();
    if (!party) {
        return 21;
    }
    return party->get_member_count() <= 1 ? 0 : 171;
}

// IDA_EA: 0x8644272
// IDA_NAME: _ZNK17TowerOfDespairMgr31IsTowerOfDespairDungeon_ByIndexEi
// DEMANGLED: TowerOfDespairMgr::IsTowerOfDespairDungeon_ByIndex(int)const
bool TowerOfDespairMgr::IsTowerOfDespairDungeon_ByIndex(int dungeon_index) const {
    return dungeon_indices_.find(dungeon_index) != dungeon_indices_.end();
}

// IDA_EA: 0x86442CA / 0x864431E
// IDA_NAME: _ZN17TowerOfDespairMgr10SetAPCInfoEjRK8_APCInfo /
//           _ZN17TowerOfDespairMgr12ResetAPCInfoEv
void TowerOfDespairMgr::SetAPCInfo(unsigned int apc_index,
                                   const _APCInfo& info) {
    if (!apc_manager_ || apc_index >= apc_layers_.size()) {
        return;
    }
    const TOD_Layer layer(static_cast<uint16_t>(apc_layers_[apc_index]));
    apc_manager_->Set_APC_Info(layer, info);
}

int TowerOfDespairMgr::ResetAPCInfo() {
    return apc_manager_->Reset_APC_Info();
}

// IDA_EA: 0x8644338
// IDA_NAME: _ZN17TowerOfDespairMgr11SendAPCInfoERK9TOD_LayerP5CUser
// DEMANGLED: TowerOfDespairMgr::SendAPCInfo(TOD_Layer const&,CUser *)
void TowerOfDespairMgr::SendAPCInfo(const TOD_Layer& layer, CUser* user) {
    apc_manager_->Send_APC_Info(layer, user);
}

// IDA_EA: 0x8644366
// IDA_NAME: _ZN17TowerOfDespairMgr18SendMessageToCUserEP5CUserPc
// DEMANGLED: TowerOfDespairMgr::SendMessageToCUser(CUser *,char *)
void TowerOfDespairMgr::SendMessageToCUser(CUser* user, char* message) {
    if (!user || user->isGMUser() != 1) {
        return;
    }
    const int length = static_cast<int>(std::strlen(message));
    PacketGuard packet;
    packet.put_header(0, 12);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_byte(0);
    packet.put_int(length);
    packet.put_str(message, length);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8644360
// IDA_NAME: _ZN17TowerOfDespairMgr15Reload_APC_InfoEv
// DEMANGLED: TowerOfDespairMgr::Reload_APC_Info(void)
void TowerOfDespairMgr::Reload_APC_Info() {
}

// IDA_EA: 0x8644470
// IDA_NAME: _ZN17TowerOfDespairMgr10_layerInfoEP5CUser
// DEMANGLED: TowerOfDespairMgr::_layerInfo(CUser *)
void TowerOfDespairMgr::_layerInfo(CUser*) {
}
