#include "old_game/player/core/character_view_data.h"

#include <cstring>
#include <ctime>
#include <map>
#include <stdint.h>
#include <vector>

#include "common/config.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/play_info_public_abi.h"
#include "common/manager/user.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/PVF/pvp_mission/fair_pvp_stat_script.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/dungeon/instance/character_blood_dungeon_runtime.h"
#include "old_game/dungeon/instance/revenge_dungeon_runtime.h"
#include "old_game/dungeon/instance/tod_user_state_runtime.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/dungeon/instance/instant_reinforce.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "old_game/pvp/match/league_match_static_state_runtime.h"
#include "old_game/pvp/match/online_preliminary_match_runtime.h"
#include "old_game/world/region/game_world.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/interface_packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/concurrent/thread/mutex.h"
#include "common/manager/system_time.h"


extern int checkSpecialChar(char* name) __asm__("_Z16checkSpecialCharPc");
extern int checkSpecialCharName(char* name)
    __asm__("_Z20checkSpecialCharNamePc");

namespace {

const unsigned int kSkillSlotCount = 204u;
const int kDimensionDungeonIds[6] = {62, 63, 64, 65, 66, 67};
const unsigned int kLimitedInCountCapacity = 3u;
const unsigned char kLimitedInCountShifts[3] = {0u, 2u, 4u};
const unsigned char kLimitedInCountMasks[3] = {3u, 3u, 1u};

struct LimitedInCountState {
    LimitedInCountState()
        : loading(false), loaded(false), dirty(false),
          daily_reset_pending(false), remaining() {}

    bool loading;
    bool loaded;
    bool dirty;
    bool daily_reset_pending;
    std::map<int, int> remaining;
};

typedef std::map<uint32_t, LimitedInCountState> CharacterInCountStates;
typedef std::map<CUser*, CharacterInCountStates> UserInCountStates;

Mutex& InCountStateMutex() {
    static Mutex mutex;
    return mutex;
}

UserInCountStates& InCountStates() {
    static UserInCountStates states;
    return states;
}

uint32_t CurrentInCountCharacterNo(CUser* user) {
    if (!user || !user->getCurCharacR()) {
        return 0u;
    }
    const int charac_no = user->getCurCharacNo();
    return charac_no > 0 ? static_cast<uint32_t>(charac_no) : 0u;
}

bool LoadLimitedInCountLayout(
    std::vector<dungeon_runtime::InCountRecord>* records,
    unsigned char* used_mask) {
    if (!records || !used_mask) {
        return false;
    }
    records->clear();
    *used_mask = 0u;
    if (!dungeon_runtime::GetLimitedInCountRecords(records) ||
        records->size() > kLimitedInCountCapacity) {
        return false;
    }
    for (std::size_t index = 0; index < records->size(); ++index) {
        const dungeon_runtime::InCountRecord& record = (*records)[index];
        const unsigned char value_mask = kLimitedInCountMasks[index];
        if (record.dungeon_index <= 0 || record.maximum_count <= 0 ||
            record.maximum_count > static_cast<int>(value_mask)) {
            return false;
        }
        *used_mask = static_cast<unsigned char>(
            *used_mask | static_cast<unsigned char>(
                             value_mask << kLimitedInCountShifts[index]));
    }
    return true;
}

bool PackLimitedInCountState(
    const LimitedInCountState& state,
    const std::vector<dungeon_runtime::InCountRecord>& records,
    unsigned char* packed) {
    if (!packed || records.size() > kLimitedInCountCapacity) {
        return false;
    }
    unsigned char value = 0u;
    for (std::size_t index = 0; index < records.size(); ++index) {
        const dungeon_runtime::InCountRecord& record = records[index];
        const std::map<int, int>::const_iterator found =
            state.remaining.find(record.dungeon_index);
        if (found == state.remaining.end()) {
            return false;
        }
        int remaining = found->second;
        if (remaining < 0) {
            remaining = 0;
        } else if (remaining > record.maximum_count) {
            remaining = record.maximum_count;
        }
        value = static_cast<unsigned char>(
            value | static_cast<unsigned char>(
                        remaining << kLimitedInCountShifts[index]));
    }
    *packed = value;
    return true;
}

LimitedInCountState* FindCurrentInCountState(CUser* user) {
    const uint32_t charac_no = CurrentInCountCharacterNo(user);
    if (!charac_no) {
        return 0;
    }
    UserInCountStates::iterator user_state = InCountStates().find(user);
    if (user_state == InCountStates().end()) {
        return 0;
    }
    CharacterInCountStates::iterator character_state =
        user_state->second.find(charac_no);
    return character_state == user_state->second.end()
               ? 0
               : &character_state->second;
}

struct ActorInOutStateRow {
    int dungeon_index;
    int maximum_count;
    int remaining_count;
};

unsigned char ClampInOutCount(int value) {
    if (value <= 0) {
        return 0;
    }
    return static_cast<unsigned char>(value > 0xff ? 0xff : value);
}

void PutActorInOutState(PacketGuard& packet,
                        int dungeon_id,
                        int maximum_count,
                        int remaining_count) {
    packet.put_int(dungeon_id);
    packet.put_byte(ClampInOutCount(maximum_count));
    packet.put_byte(ClampInOutCount(remaining_count));
}

bool BuildActorInOutStateRows(CUser* user,
                              std::vector<ActorInOutStateRow>* rows) {
    if (!user || !rows) {
        return false;
    }
    rows->clear();
    std::vector<dungeon_runtime::InCountRecord> records;
    if (!dungeon_runtime::GetInCountRecords(&records) ||
        records.size() > 0xffu) {
        return false;
    }
    rows->reserve(records.size());
    for (std::size_t record_index = 0;
         record_index < records.size();
         ++record_index) {
        const dungeon_runtime::InCountRecord& record = records[record_index];
        ActorInOutStateRow row;
        row.dungeon_index = record.dungeon_index;
        row.maximum_count = record.maximum_count;
        row.remaining_count = 0;

        bool resolved = false;
        if (dungeon_runtime::IsLimitedInCountDungeon(record.dungeon_index)) {
            int maximum = record.maximum_count;
            int remaining = 0;
            if (dungeon_runtime::GetRemainingInCount(
                    user, record.dungeon_index, &maximum, &remaining)) {
                row.maximum_count = maximum;
                row.remaining_count = remaining;
            }
            resolved = true;
        }
        for (int dimension = 0; !resolved && dimension < 6; ++dimension) {
            if (record.dungeon_index == kDimensionDungeonIds[dimension]) {
                row.remaining_count = user->getDemensionInoutValue(
                    static_cast<char>(dimension));
                resolved = true;
            }
        }
        if (!resolved && record.dungeon_index == 11006) {
            CCharacBloodDungeon* const blood_dungeon =
                static_cast<CCharacBloodDungeon*>(user->GetCharacExpandData(
                    static_cast<ENUM_CHARAC_EXPAND_TYPE>(1)));
            row.remaining_count = blood_dungeon
                                      ? row.maximum_count -
                                            blood_dungeon->GetEnterCount()
                                      : 0;
            resolved = true;
        }
        if (!resolved && record.dungeon_index == 11007) {
            row.remaining_count = user->getUltimateInoutValue(2);
            resolved = true;
        }
        if (!resolved && record.dungeon_index == 122) {
            row.remaining_count = user->getUltimateInoutValue(0);
        }
        rows->push_back(row);
    }
    return true;
}

int CurrentSecond() {
    return GlobalData::system_time() ? GlobalData::system_time()->getCurSec()
                                     : 0;
}

int ForbidRemainDays(_Charac_info& character) {
    if (!character.fields_.forbid_user_to_play_) {
        return 0;
    }
    const int remain = character.fields_.forbid_remaining_seconds_;
    if (remain <= 0) {
        character.fields_.forbid_user_to_play_ = 0;
        character.fields_.forbid_remaining_seconds_ = 0;
        return 0;
    }
    const int days = remain / 86400;
    return days > 0 ? days : 1;
}

unsigned char SelectListRestrictionCode(_Charac_info& character, int cur_sec) {
    character.fields_.ghost_state_ =
        character.fields_.chaos_response_time_late_ > cur_sec;
    if (character.fields_.character_view_state_ == 255) {
        return 8;
    }
    return character.fields_.ghost_state_ ? 4 : 0;
}

void MakeSkillTreeList(CUser* user,
                       PacketGuard& packet,
                       const SkillSlot* skill_slot,
                       const STGuildDBInfoOnly* guild_info,
                       ENUM_SKILL_TREE_KIND skill_tree) {
    int count_index = packet.get_index();
    int count = 0;
    packet.put_byte(0);

    const _Mastered_skill* const skills =
        skill_slot->get_skillslot_buf(skill_tree);
    for (unsigned int slot = 0; slot < kSkillSlotCount; ++slot) {
        const uint8_t skill_index = skills[slot].skill_index;
        if (!skill_index) {
            continue;
        }
        packet.put_byte(skill_index);
        packet.put_byte(skills[slot].skill_level);
        ++count;
    }

    for (unsigned int index = 0; index < guild_info->skill_count; ++index) {
        const STGuildSkill& guild_skill = guild_info->skills[index];
        if (static_cast<unsigned char>(user->getGuildMemberGrade()) &&
            guild_skill.skill_id > 199) {
            packet.put_byte(guild_skill.skill_id);
            packet.put_byte(static_cast<signed char>(guild_skill.level));
            ++count;
        }
    }
    packet.put_byte(count_index, count);
}

void ApplyFairPvPCharacterStatus(_Additioal_info& destination,
                                 const CharacterStatus& source) {
    if (source.max_hp) {
        destination.max_hp = source.max_hp;
    }
    if (source.max_mp) {
        destination.max_mp = source.max_mp;
    }
    if (source.physical_attack) {
        destination.physical_attack = source.physical_attack;
    }
    if (source.physical_defense) {
        destination.physical_defense = source.physical_defense;
    }
    if (source.magical_attack) {
        destination.magical_attack = source.magical_attack;
    }
    if (source.magical_defense) {
        destination.magical_defense = source.magical_defense;
    }
    for (int index = 0; index <= 3; ++index) {
        if (source.element_resist[index]) {
            destination.element_resist[index] =
                source.element_resist[index];
        }
    }
    for (int index = 0; index <= 16; ++index) {
        if (source.spec_property[index]) {
            destination.spec_property[index] =
                source.spec_property[index];
        }
    }
    if (source.inventory_limit) {
        destination.inventory_weight = source.inventory_limit;
    }
    if (source.hp_regen_speed) {
        destination.hp_regen_speed = source.hp_regen_speed;
    }
    if (source.mp_regen_speed) {
        destination.mp_regen_speed = source.mp_regen_speed;
    }
    if (source.move_speed) {
        destination.move_speed = source.move_speed;
    }
    if (source.attack_speed) {
        destination.attack_speed = source.attack_speed;
    }
    if (source.cast_speed) {
        destination.cast_speed = source.cast_speed;
    }
    if (source.hit_recovery) {
        destination.hit_recovery = source.hit_recovery;
    }
    if (source.jump_power) {
        destination.jump_power = source.jump_power;
    }
    if (source.weight) {
        destination.character_weight = source.weight;
    }
}

}  // namespace

namespace dungeon_runtime {

void BeginInCountLoad(CUser* user, uint32_t charac_no) {
    if (!user || !charac_no) {
        return;
    }
    MutexGuard guard(&InCountStateMutex());
    LimitedInCountState& state = InCountStates()[user][charac_no];
    const bool reset_pending = state.daily_reset_pending;
    state = LimitedInCountState();
    state.loading = true;
    state.daily_reset_pending = reset_pending;
}

bool ApplyInCountLoad(CUser* user,
                      uint32_t charac_no,
                      uint8_t packed,
                      bool reset_for_today) {
    if (!user || !charac_no) {
        return false;
    }
    std::vector<InCountRecord> records;
    unsigned char used_mask = 0u;
    if (!LoadLimitedInCountLayout(&records, &used_mask) ||
        (packed & static_cast<unsigned char>(~used_mask)) != 0u) {
        return false;
    }

    MutexGuard guard(&InCountStateMutex());
    UserInCountStates::iterator user_state = InCountStates().find(user);
    if (user_state == InCountStates().end()) {
        return false;
    }
    CharacterInCountStates::iterator character_state =
        user_state->second.find(charac_no);
    if (character_state == user_state->second.end() ||
        !character_state->second.loading) {
        return false;
    }

    LimitedInCountState& state = character_state->second;
    const bool reset = reset_for_today || state.daily_reset_pending;
    state.remaining.clear();
    state.dirty = false;
    for (std::size_t index = 0; index < records.size(); ++index) {
        const InCountRecord& record = records[index];
        const int raw = static_cast<int>(
            (packed >> kLimitedInCountShifts[index]) &
            kLimitedInCountMasks[index]);
        int remaining = raw;
        if (remaining > record.maximum_count) {
            remaining = record.maximum_count;
            state.dirty = true;
        }
        if (reset) {
            if (remaining != record.maximum_count) {
                state.dirty = true;
            }
            remaining = record.maximum_count;
        }
        state.remaining.insert(std::make_pair(record.dungeon_index,
                                              remaining));
    }
    state.loading = false;
    state.loaded = true;
    state.daily_reset_pending = false;
    return true;
}

bool IsInCountLoaded(CUser* user) {
    if (!user) {
        return false;
    }
    MutexGuard guard(&InCountStateMutex());
    LimitedInCountState* const state = FindCurrentInCountState(user);
    return state && state->loaded;
}

bool GetRemainingInCount(CUser* user,
                         int dungeon_index,
                         int* maximum,
                         int* remaining) {
    if (!user || !maximum || !remaining) {
        return false;
    }
    std::vector<InCountRecord> records;
    unsigned char used_mask = 0u;
    if (!LoadLimitedInCountLayout(&records, &used_mask)) {
        return false;
    }
    const InCountRecord* record = 0;
    for (std::size_t index = 0; index < records.size(); ++index) {
        if (records[index].dungeon_index == dungeon_index) {
            record = &records[index];
            break;
        }
    }
    if (!record) {
        return false;
    }

    MutexGuard guard(&InCountStateMutex());
    LimitedInCountState* const state = FindCurrentInCountState(user);
    if (!state || !state->loaded) {
        return false;
    }
    const std::map<int, int>::const_iterator found =
        state->remaining.find(dungeon_index);
    if (found == state->remaining.end()) {
        return false;
    }
    *maximum = record->maximum_count;
    *remaining = found->second;
    return true;
}

bool TryConsumeInCount(CUser* user, int dungeon_index) {
    if (!user) {
        return false;
    }
    MutexGuard guard(&InCountStateMutex());
    LimitedInCountState* const state = FindCurrentInCountState(user);
    if (!state || !state->loaded) {
        return false;
    }
    std::map<int, int>::iterator found = state->remaining.find(dungeon_index);
    if (found == state->remaining.end() || found->second <= 0) {
        return false;
    }
    --found->second;
    state->dirty = true;
    return true;
}

void ResetDailyInCount(CUser* user) {
    if (!user) {
        return;
    }
    std::vector<InCountRecord> records;
    unsigned char used_mask = 0u;
    if (!LoadLimitedInCountLayout(&records, &used_mask)) {
        return;
    }
    const uint32_t charac_no = CurrentInCountCharacterNo(user);
    if (!charac_no) {
        return;
    }

    MutexGuard guard(&InCountStateMutex());
    LimitedInCountState& state = InCountStates()[user][charac_no];
    if (!state.loaded) {
        state.daily_reset_pending = true;
        return;
    }
    for (std::size_t index = 0; index < records.size(); ++index) {
        const InCountRecord& record = records[index];
        int& remaining = state.remaining[record.dungeon_index];
        if (remaining != record.maximum_count) {
            remaining = record.maximum_count;
            state.dirty = true;
        }
    }
}

void MarkDailyResetPending(CUser* user) {
    if (!user) {
        return;
    }
    const uint32_t charac_no = CurrentInCountCharacterNo(user);
    if (!charac_no) {
        return;
    }
    MutexGuard guard(&InCountStateMutex());
    InCountStates()[user][charac_no].daily_reset_pending = true;
}

bool IsInCountDirty(CUser* user) {
    if (!user) {
        return false;
    }
    MutexGuard guard(&InCountStateMutex());
    LimitedInCountState* const state = FindCurrentInCountState(user);
    return state && state->loaded && state->dirty;
}

bool PackInCountForSave(CUser* user, uint8_t* packed) {
    if (!user || !packed) {
        return false;
    }
    std::vector<InCountRecord> records;
    unsigned char used_mask = 0u;
    if (!LoadLimitedInCountLayout(&records, &used_mask)) {
        return false;
    }
    MutexGuard guard(&InCountStateMutex());
    LimitedInCountState* const state = FindCurrentInCountState(user);
    return state && state->loaded &&
           PackLimitedInCountState(*state, records, packed);
}

void MarkInCountSaved(CUser* user, uint8_t packed) {
    if (!user) {
        return;
    }
    std::vector<InCountRecord> records;
    unsigned char used_mask = 0u;
    if (!LoadLimitedInCountLayout(&records, &used_mask) ||
        (packed & static_cast<unsigned char>(~used_mask)) != 0u) {
        return;
    }
    MutexGuard guard(&InCountStateMutex());
    LimitedInCountState* const state = FindCurrentInCountState(user);
    unsigned char current = 0u;
    if (state && state->loaded &&
        PackLimitedInCountState(*state, records, &current) &&
        current == packed) {
        state->dirty = false;
    }
}

void ResetInCountUser(CUser* user) {
    if (!user) {
        return;
    }
    MutexGuard guard(&InCountStateMutex());
    InCountStates().erase(user);
}

}  // namespace dungeon_runtime

// IDA_EA: 0x8651322
// IDA_NAME: _ZN5CUser27updateCharacViewVisibleDataERK12_Charac_infoR17CHARAC_LOGIN_DATA
// DEMANGLED: CUser::updateCharacViewVisibleData(_Charac_info const&, CHARAC_LOGIN_DATA &)
char CUser::updateCharacViewVisibleData(const _Charac_info& character,
                                        CHARAC_LOGIN_DATA& out) {
    std::memset(out.visible_avatar, 0, sizeof(out.visible_avatar));
    const _Charac_info* const current = getCurCharacR();
    if (current &&
        current->fields_.character_no_ == character.fields_.character_no_) {
        out.request_rename_state =
            static_cast<uint8_t>(getRequestRenameState());
        const CInventory& inventory = character.fields_.inventory_;
        for (int slot = 0; slot <= 8; ++slot) {
            const Inven_Item& item = inventory.equipment_items_[slot];
            if (!item.item_id_) {
                continue;
            }
            const stAvatarExpansionInfo_t* const expansion =
                inventory.GetAvatarItemMgrR()->GetExpansionInfo(
                    item.add_info_);
            if (!expansion) {
                continue;
            }
            const int clear_avatar = inventory.GetClearAvatar(slot);
            CHARAC_VIEW_AVATAR_DATA& visible = out.visible_avatar[slot];
            visible.item_id = clear_avatar
                                  ? static_cast<uint32_t>(clear_avatar)
                                  : static_cast<uint32_t>(item.item_id_);
            visible.avatar_expansion_0 = expansion->avatar_expansion[0];
            visible.avatar_expansion_1 = expansion->avatar_expansion[1];
        }

        // 0828 inserts the weapon-avatar at native equipment slot 10.  It is
        // an avatar appearance, so persist the clear-avatar projection and
        // both color/expansion values just like the eight standard avatar
        // parts; the legacy item-id-only tail would erase these two fields on
        // the next character-view save.
        {
            const int slot = oldsrv::config::kEquipmentTypeWeaponAvatar;
            const Inven_Item& item = inventory.equipment_items_[slot];
            if (item.item_id_) {
                const int clear_avatar = inventory.GetClearAvatar(slot);
                CHARAC_VIEW_AVATAR_DATA& visible =
                    out.visible_avatar[slot];
                visible.item_id =
                    clear_avatar
                        ? static_cast<uint32_t>(clear_avatar)
                        : static_cast<uint32_t>(item.item_id_);
                const stAvatarExpansionInfo_t* const expansion =
                    inventory.GetAvatarItemMgrR()->GetExpansionInfo(
                        item.add_info_);
                if (expansion) {
                    visible.avatar_expansion_0 =
                        expansion->avatar_expansion[0];
                    visible.avatar_expansion_1 =
                        expansion->avatar_expansion[1];
                }
            }
        }
        for (int slot = oldsrv::config::kEquipmentTypeAuroraAvatar;
             slot <= oldsrv::config::kCharacterViewEquipmentTypeLast;
             ++slot) {
            if (slot == oldsrv::config::kEquipmentTypeWeaponAvatar) {
                continue;
            }
            const Inven_Item& item = inventory.equipment_items_[slot];
            if (checkRentedItem(
                    static_cast<unsigned int>(character.fields_.character_no_),
                    item.item_id_,
                    1) ||
                checkRentedItem(
                    static_cast<unsigned int>(character.fields_.character_no_),
                    item.item_id_,
                    2)) {
                continue;
            }
            out.visible_avatar[slot].item_id =
                static_cast<uint32_t>(item.item_id_);
        }
        return 0;
    }

    for (int slot = oldsrv::config::kCharacterViewEquipmentTypeFirst;
         slot <= oldsrv::config::kCharacterViewEquipmentTypeLast;
         ++slot) {
        const CInventory::AvatarVisibleData& source =
            character.fields_.inventory_.avatar_visible_data_[slot];
        CHARAC_VIEW_AVATAR_DATA& destination = out.visible_avatar[slot];
        destination.item_id = source.item_id;
        destination.avatar_expansion_0 = source.avatar_expansion_0;
        destination.avatar_expansion_1 = source.avatar_expansion_1;
    }
    out.request_rename_state =
        static_cast<char>(character.fields_.character_view_state_);
    return static_cast<char>(reinterpret_cast<uintptr_t>(&out));
}

// IDA_EA: 0x8651740
// IDA_NAME: _ZN5CUser16UpdateCharacViewEv
// DEMANGLED: CUser::UpdateCharacView(void)
void CUser::UpdateCharacView() {
    CUser* const user = this;
    if (G_GameWorld()->GetChannelType() == 7) {
        return;
    }

    if (static_cast<unsigned char>(character_view_.isSaveCharacView()) != 1u) {
        return;
    }

    Stream* stream = GlobalData::stream_pool()
                         ? GlobalData::stream_pool()->Acquire("user.cpp", 6943)
                         : 0;
    CStreamGuard guard(stream, true);
    *guard << 6;
    *guard << user->GetUID();

    SIG_UPDATE_CHARAC_VIEW* const payload =
        guard.GetInBuffer<SIG_UPDATE_CHARAC_VIEW>();
    std::memset(payload, 0, sizeof(*payload));
    payload->server_group = static_cast<uint8_t>(user->GetServerGroup());
    payload->account_id = user->get_acc_id();

    const int charac_count = static_cast<int>(characters_.size());
    for (int slot = 0; slot < charac_count; ++slot) {
        const _Charac_info& character = characters_[slot];
        CHARAC_LOGIN_DATA& record = payload->characters[slot];

        record.character_no =
            static_cast<uint32_t>(character.fields_.character_no_);
        std::strncpy(record.character_name,
                     character.fields_.character_name_,
                     sizeof(record.character_name));
        record.job = static_cast<uint8_t>(character.fields_.job_);
        record.grow_type = static_cast<uint8_t>(character.fields_.grow_type_.growth_type);
        record.level = static_cast<uint16_t>(character.fields_.level_);

        user->updateCharacViewVisibleData(character, record);

        record.create_time = static_cast<uint32_t>(
            static_cast<long>(character.fields_.create_time_));
        record.chaos_die_time =
            static_cast<uint32_t>(character.fields_.chaos_die_time_);
        record.state_flags =
            static_cast<char>((character.fields_.user_event_character_flag_ != 0)
                              | (record.state_flags & 0xFE));
        record.striker_skill_index = character.fields_.striker_skill_index_;
        record.tag_character_no =
            static_cast<uint32_t>(character.fields_.tag_character_no_);
        record.state_flags = static_cast<char>(
            (16 * (((character.fields_.visible_flag_.visible_flag << 6) >> 7) &
                    1)) |
            (record.state_flags & 0xEF));
        record.chaos_response_time_late = static_cast<uint32_t>(
            character.fields_.chaos_response_time_late_);
    }

    if (GlobalData::msgq_mgr()) {
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    }
    character_view_.disableSaveCharacView();
}

// IDA_EA: 0x82301AE
// IDA_NAME: _ZN5CUser21GetGuildPowerWarPointEv
// DEMANGLED: CUser::GetGuildPowerWarPoint(void)
int CUser::GetGuildPowerWarPoint() {
    return static_cast<int>(guild_info_.guild_power_war_point);
}

// IDA_EA: 0x82301D8
// IDA_NAME: _ZN5CUser16IsExistGuildAgitEv
// DEMANGLED: CUser::IsExistGuildAgit(void)
bool CUser::IsExistGuildAgit() {
    return guild_info_.guild_agit_flag != 0;
}

// IDA_EA: 0x82F0E70
// IDA_NAME: _ZN5CUser12GetUserStateEv
// DEMANGLED: CUser::GetUserState(void)
int CUser::GetUserState() {
    const int user_state = static_cast<int>(state_);
    if (user_state <= 3) {
        return GetUserStateByAssault();
    }
    switch (user_state) {
        case 5:
        case 10:
        case 12:
            return 1;
        case 8:
            return 3;
        case 6:
        case 7:
            return 2;
        case 11:
            return 7;
        default:
            return user_state == 12;
    }
}

// IDA_EA: 0x85BFD66
// IDA_NAME: _ZN5CUser12IsReturnUserEv
// DEMANGLED: CUser::IsReturnUser(void)
int CUser::IsReturnUser() {
    return return_user_;
}

// IDA_EA: 0x869742A
// IDA_NAME: _ZN5CUser12GetGuildNameEv
// DEMANGLED: CUser::GetGuildName(void)
char* CUser::GetGuildName() {
    return GetGuildDBInfo()->guild_name;
}

// IDA_EA: 0x8697438
// IDA_NAME: _ZN5CUser13GetGuildLevelEv
// DEMANGLED: CUser::GetGuildLevel(void)
int CUser::GetGuildLevel() {
    return GetGuildDBInfo()->guild_level;
}

// IDA_EA: 0x8694A88
// IDA_NAME: _ZNK8WongWork12CUserPremium17GetAdditionalInfoEv
// DEMANGLED: WongWork::CUserPremium::GetAdditionalInfo(void)const
_Additioal_info WongWork::CUserPremium::GetAdditionalInfo() const {
    return calculated_additional_info_;
}

// IDA_EA: 0x8659FAA
// IDA_NAME: _ZN5CUser16make_charac_statER11PacketGuard
// DEMANGLED: CUser::make_charac_stat(PacketGuard &)
int CUser::make_charac_stat(PacketGuard& packet) {
    if (G_GameWorld()->IsCharacterStatRevisionChannel()) {
        _Additioal_info additional_info;
        std::memcpy(&additional_info,
                    getCurCharacAddInfoR(),
                    sizeof(additional_info));
        FairPvPStatScript* const fair_pvp_stats =
            G_CDataManager()->get_fair_pvp_stat_script();
        CharacterStatus* const revised_status =
            fair_pvp_stats->FindCharacStat(get_charac_job(),
                                           getCurCharFirstGrowType());
        if (revised_status) {
            ApplyFairPvPCharacterStatus(additional_info, *revised_status);
        }
        instant_reinforce::ApplyToStatusSnapshot(this, &additional_info);
        packet.put_int(83);
        packet.put_binary(reinterpret_cast<const char*>(&additional_info),
                          sizeof(additional_info));
        return packet.put_byte(getCurCharacR()->fields_.stamina_);
    }

    const _Charac_info* const character = getCurCharacR();
    if (IsHavePremiumAdvantage()) {
        _Additioal_info additional_info;
        std::memcpy(&additional_info,
                    getCurCharacAddInfoR(),
                    sizeof(additional_info));
        additional_info += GetPremiumInfo()->GetAdditionalInfo();
        instant_reinforce::ApplyToStatusSnapshot(this, &additional_info);
        packet.put_int(83);
        packet.put_binary(reinterpret_cast<const char*>(&additional_info),
                          sizeof(additional_info));
        return packet.put_byte(character->fields_.stamina_);
    }

    _Additioal_info additional_info;
    std::memcpy(&additional_info,
                getCurCharacAddInfoR(),
                sizeof(additional_info));
    instant_reinforce::ApplyToStatusSnapshot(this, &additional_info);
    packet.put_int(83);
    packet.put_binary(reinterpret_cast<const char*>(&additional_info),
                      sizeof(additional_info));
    return packet.put_byte(character->fields_.stamina_);
}

// IDA_EA: 0x865A44E
// IDA_NAME: _ZN5CUser15make_basic_infoEPcc
// DEMANGLED: CUser::make_basic_info(char *, char)
int CUser::make_basic_info(PacketGuard& packet, char mode) {
    if (mode == 1) {
        if (!getCurCharacR()) {
            return 0;
        }

        lock();
        packet.put_short(get_unique_id());
        if (G_GameWorld()->IsCharacterLevelRevisionChannel()) {
            packet.put_int(G_CDataManager()->get_level_exp(
                oldsrv::config::kCharacterLevelRevision));
        } else {
            packet.put_int(get_charac_exp());
        }
        make_charac_stat(packet);
        packet.put_byte(GetCurExEquipSlotStat());

        CInventory* const inventory = getCurCharacInvenR();
        inventory->MakeEquipList(&packet,
                                 true,
                                 static_cast<ENUM_USERINFO>(mode),
                                 static_cast<ENUM_EQUIPSLOT>(0));
        packet.put_byte(GetCurCharacSkillTreeIndex());

        const STGuildDBInfoOnly* const guild_info = GetGuildDBInfo();
        const SkillSlot* const skill_slot = getCurCharacSkillR();
        MakeSkillTreeList(this,
                          packet,
                          skill_slot,
                          guild_info,
                          ENUM_SKILL_TREE_0);
        MakeSkillTreeList(this,
                          packet,
                          skill_slot,
                          guild_info,
                          ENUM_SKILL_TREE_1);

        packet.put_byte(inventory->GetCreatureMgrR()->GetEquipedCreatureLevel());

        // 0828 ParseActorInOutStateMap consumes:
        // u8 count; count * (u32 dungeon, u8 maximum, u8 remaining);
        // u8 revenge; u8 Tower-of-Despair enter count.
        std::vector<ActorInOutStateRow> inout_rows;
        if (!BuildActorInOutStateRows(this, &inout_rows)) {
            inout_rows.clear();
        }
        packet.put_byte(static_cast<unsigned char>(inout_rows.size()));
        for (std::size_t index = 0; index < inout_rows.size(); ++index) {
            const ActorInOutStateRow& row = inout_rows[index];
            PutActorInOutState(packet,
                               row.dungeon_index,
                               row.maximum_count,
                               row.remaining_count);
        }

        packet.put_byte(
            GlobalData::s_revengeDungeonMgr &&
                    static_cast<unsigned char>(GlobalData::s_revengeDungeonMgr
                                                   ->IsOpenRevengeDungeon())
                ? ClampInOutCount(GetCurRevengeDungeonCount())
                : 0);
        TOD_UserState* const tod_state = static_cast<TOD_UserState*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(13)));
        packet.put_byte(tod_state
                            ? ClampInOutCount(tod_state->getEnterCount())
                            : 0);

        CQuestShop* const quest_shop = static_cast<CQuestShop*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
        quest_shop->makePacket_BuyingStatusList(packet, false);
        // The 0828 client consumes two bytes after the QP list.  The first
        // is the grow-avatar visibility flag; the second is the account
        // adventure/manage level.  It immediately resolves that level
        // through linksystem/charactermanage.etc [manage option] and applies
        // the value to strength, intelligence, vitality, and spirit.
        packet.put_byte(static_cast<unsigned char>(
            get_charac_visible_grow_avatar() != 0));
        packet.put_byte(static_cast<unsigned char>(GetManageLevel()));
        unlock();
        return 1;
    }

    if (mode == 3) {
        if (!getCurCharacR()) {
            return 0;
        }

        lock();
        packet.put_short(get_unique_id());
        if (G_GameWorld()->IsCharacterLevelRevisionChannel()) {
            packet.put_int(G_CDataManager()->get_level_exp(
                oldsrv::config::kCharacterLevelRevision));
        } else {
            packet.put_int(get_charac_exp());
        }
        make_charac_stat(packet);
        packet.put_byte(GetCurExEquipSlotStat());

        CInventory* const inventory = getCurCharacInvenR();
        inventory->MakeEquipList(&packet,
                                 true,
                                 static_cast<ENUM_USERINFO>(mode),
                                 static_cast<ENUM_EQUIPSLOT>(0));
        packet.put_byte(GetCurCharacSkillTreeIndex());

        const STGuildDBInfoOnly* const guild_info = GetGuildDBInfo();
        const SkillSlot* const skill_slot = getCurCharacSkillR();
        MakeSkillTreeList(this,
                          packet,
                          skill_slot,
                          guild_info,
                          ENUM_SKILL_TREE_0);
        MakeSkillTreeList(this,
                          packet,
                          skill_slot,
                          guild_info,
                          ENUM_SKILL_TREE_1);

        packet.put_byte(inventory->GetCreatureMgrR()->GetEquipedCreatureLevel());
        const PvpResultType* const pvp_result = getPVPResultR();
        packet.put_int(pvp_result->values[0]);
        packet.put_int(pvp_result->values[1]);
        packet.put_int(pvp_result->values[5]);
        packet.put_byte(pvp_result->values[23]);

        const CommonTime& create_time =
            getCurCharacR()->fields_.create_time_;
        packet.put_byte(create_time.year);
        packet.put_byte(create_time.month);
        packet.put_byte(create_time.day);
        packet.put_int(getCurCharacHelpAbuseComputedRatio());
        packet.put_short(GetUserPowerWarPoint());
        packet.put_int(GetGuildPowerWarPoint());

        // 客户端在这里直接读取 guild power 值和三个公会标志，
        // 不再消费旧的 guild-name 长度/字符串。
        packet.put_byte(GetGuildLevel());
        packet.put_byte(getPowerSide());
        packet.put_byte(IsExistGuildAgit() ? 1 : 0);

        CQuestShop* const quest_shop = static_cast<CQuestShop*>(
            GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
        quest_shop->makePacket_BuyingStatusList(packet, false);
        // Other-character detail uses the same two-byte tail as self detail:
        // grow-avatar visibility followed by the account manage level.
        packet.put_byte(static_cast<unsigned char>(
            get_charac_visible_grow_avatar() != 0));
        packet.put_byte(static_cast<unsigned char>(GetManageLevel()));
        unlock();
        return 1;
    }

    if (mode == 0) {
        if (!getCurCharacR()) {
            LogManager::logFormat(
                1,
                "user.cpp",
                "bool CUser::make_basic_info(char*, char)",
                12428,
                "Error ENUM_USERINFO_MINIMUM, getCurCharacR() == 0");
            return 0;
        }

        lock();
        packet.put_short(get_unique_id());

        const char* const character_name = getCurCharacName();
        const int character_name_length =
            static_cast<int>(std::strlen(character_name));
        packet.put_int(character_name_length);
        packet.put_str(character_name, character_name_length);
        packet.put_byte(get_charac_job());
        packet.put_byte(getCurCharacGrowType());
        packet.put_byte(
            G_GameWorld()->IsCharacterLevelRevisionChannel()
                ? oldsrv::config::kCharacterLevelRevision
                : get_charac_level());
        packet.put_byte(get_pvp_grade());
        packet.put_byte(getPVPResultR()->values[23]);
        packet.put_byte(GetUserState());
        CInventory* const inventory = getCurCharacInvenR();
        inventory->MakeEquipList(&packet,
                                 false,
                                 static_cast<ENUM_USERINFO>(mode),
                                 static_cast<ENUM_EQUIPSLOT>(0));

        (void)get_acc_id();
        packet.put_byte(0);
        if (G_GameWorld()->IsSchoolPvPChannel() && GetSchoolNo()) {
            packet.put_int(GetSchoolNo());
        }

        packet.put_int(G_GameWorld()->GetChannelType() == 14
                           ? GetCurCharacOnlinePreliminaryTeamId()
                           : get_charac_guildkey());
        packet.put_byte(IsPermissionPrivateStore());

        const user_creature::CCreatureMgr* const creature_manager =
            inventory->GetCreatureMgrR();
        packet.put_int(creature_manager->GetCreatureItemId());
        const char* const creature_name = creature_manager->GetCreatureName();
        const int creature_name_length =
            static_cast<int>(std::strlen(creature_name));
        packet.put_int(creature_name_length);
        packet.put_str(creature_name, creature_name_length);
        packet.put_byte(creature_manager->IsDieCreature() == 0);
        packet.put_byte(
            isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(12)));
        packet.put_int(GetPCRoomNo());
        packet.put_int(getCurrCharacBlackCount());

        const STGuildDBInfoOnly* const guild_info = GetGuildDBInfo();
        if (G_GameWorld()->IsSchoolPvPChannel() && GetSchoolNo()) {
            packet.put_byte(CLeagueMatch::GetPlayCountPerOneTeam());
            const int school_name_length =
                static_cast<int>(character_string_.length());
            packet.put_int(school_name_length);
            packet.put_str(character_string_.c_str(), school_name_length);
        } else if (G_GameWorld()->GetChannelType() == 14) {
            packet.put_byte(
                online_preliminary::COnlinePreliminary::
                    GetPlayCountPerOneTeam());
            const char* const team_name =
                GetCurCharacOnlinePreliminaryTeamName();
            const int team_name_length =
                static_cast<int>(std::strlen(team_name));
            packet.put_int(team_name_length);
            packet.put_str(team_name, team_name_length);
        } else {
            packet.put_byte(guild_info->guild_level);
            const int guild_name_length =
                static_cast<int>(std::strlen(guild_info->guild_name));
            packet.put_int(guild_name_length);
            packet.put_str(guild_info->guild_name, guild_name_length);
        }

        packet.put_int(GetCurCharacChaosPoint());
        packet.put_byte(static_cast<unsigned int>(GetAge()) > 14u);
        packet.put_byte(getPowerSide());
        packet.put_byte(isJoinPowerWar());
        packet.put_byte(GetCurCharacExpertJobType());
        packet.put_int(GetCurCharacExpertJobExp());
        packet.put_byte(getDisguiseKind());
        packet.put_short(getDisguiseIndex());
        packet.put_byte(get_charac_visible_values());

        GameWorld* const world = G_GameWorld();
        if (world->IsForbiddenChat(character_name)) {
            unsigned int left_time =
                world->GetLeftTimeOfForbiddenChat(character_name);
            if (!left_time) {
                world->EnableChat(character_name);
            }
            cMyTrace before_trace(
                "bool CUser::make_basic_info(char*, char)", 12628, 0);
            before_trace("%s FCT1: %ld\n",
                         character_name,
                         static_cast<long>(left_time));
            left_time /= 60000u;
            cMyTrace after_trace(
                "bool CUser::make_basic_info(char*, char)", 12630, 0);
            after_trace("%s FCT2: %ld\n",
                        character_name,
                        static_cast<long>(left_time));
            packet.put_int(left_time);
        } else {
            packet.put_int(0);
        }

        if (world->IsForbiddenMove(character_name)) {
            unsigned int left_time =
                world->GetLeftTimeOfForbiddenMove(character_name);
            if (!left_time) {
                world->EnableMove(character_name);
            }
            cMyTrace before_trace(
                "bool CUser::make_basic_info(char*, char)", 12643, 0);
            before_trace("%s FMT1: %ld\n",
                         character_name,
                         static_cast<long>(left_time));
            left_time /= 60000u;
            cMyTrace after_trace(
                "bool CUser::make_basic_info(char*, char)", 12645, 0);
            after_trace("%s FMT2: %ld\n",
                        character_name,
                        static_cast<long>(left_time));
            packet.put_int(left_time);
        } else {
            packet.put_int(0);
        }

        packet.put_int(getCurCharacHelpAbuseComputedRatio());
        packet.put_byte(getCurCharacR()->fields_.stamina_);
        packet.put_short(getCurCharacUsedFatigue());
        packet.put_byte(GetServerGroup());
        packet.put_short(GetUserPowerWarPoint());
        packet.put_byte(GlobalData::s_power_manager->GetRankingByCharacNo(
            getCurCharacNo()));
        packet.put_short(GetCurCharacEmotion());
        packet.put_byte(GetCurCharacSkillTreeIndex());
        packet.put_byte(IsReturnUser() ? 1 : 0);
        // 0828 ParseUserInfoMinimumEntry consumes the independently persisted
        // user-event-character flag immediately after the return-user flag.
        packet.put_byte(getUserEventCharacterFlag() != 0 ? 1 : 0);

        const int character_link_type = getCharacLinkType();
        packet.put_byte(getLinkCharacSlotIndex());
        packet.put_byte(character_link_type);
        unlock();
        return 1;
    }

    if (mode != 2) {
        return 0;
    }

    lock();
    packet.put_byte(getCharacSlotLimit());
    packet.put_byte(getSlotEffectCount());
    packet.put_byte(GetManageLevel());
    packet.put_int(GetManagePoint());

    int count = 0;
    int count_index = packet.get_index();
    packet.put_short(0);
    const int cur_sec = CurrentSecond();
    const int charac_count = static_cast<int>(characters_.size());
    for (int slot = 0; slot < charac_count; ++slot) {
        _Charac_info& character = characters_[slot];
        if (G_GameWorld()->GetChannelType() == 14) {
            const unsigned int charac_no = static_cast<unsigned int>(
                character.fields_.character_no_);
            if (charac_no != getOnlinePreliminaryCharacNo()) {
                continue;
            }
        }

        packet.put_short(count++);
        const int name_len = static_cast<int>(
            std::strlen(character.fields_.character_name_));
        packet.put_int(name_len);
        packet.put_str(character.fields_.character_name_, name_len);

        bool invalid_name = true;
        if (!static_cast<unsigned char>(
                checkSpecialChar(character.fields_.character_name_)) &&
            !static_cast<unsigned char>(
                checkSpecialCharName(character.fields_.character_name_))) {
            invalid_name = false;
        }
        packet.put_byte(invalid_name);
        packet.put_byte(character.fields_.job_);
        packet.put_byte(character.fields_.grow_type_.growth_type);
        packet.put_byte(character.fields_.level_);
        packet.put_byte(0);
        packet.put_byte(SelectListRestrictionCode(character, cur_sec));
        character.fields_.inventory_.MakeEquipList(
            &packet,
            false,
            static_cast<ENUM_USERINFO>(mode),
            static_cast<ENUM_EQUIPSLOT>(0));
        packet.put_int(static_cast<int>(character.fields_.guild_id_));
        packet.put_byte(IsPermissionPrivateStore());
        packet.put_byte(isAffectedPremium(
            static_cast<ENUM_PREMIUM_TYPE>(12)));
        packet.put_int(GetPCRoomNo());
        packet.put_byte(character.fields_.visible_flag_.visible_flag);
        packet.put_byte(character.fields_.user_event_character_flag_);
        // CLIENT_IDA_EVIDENCE: ParseUserInfoCharacterListEntry widens the old
        // link-type byte to u32, then consumes three u8 fields before
        // readUseCharacterLockTime consumes the following u32 duration.
        packet.put_int(static_cast<uint32_t>(character.fields_.link_type_));
        // readVisibleChina applies the first byte to the character actor. The
        // following two bytes are cursor-only in this reader; one actor field
        // is explicitly reset to zero and neither packet value is retained.
        packet.put_byte(0);
        packet.put_byte(0);
        packet.put_byte(0);

        const bool forbid_active =
            character.fields_.forbid_user_to_play_ != 0;
        const int forbid_remain = forbid_active
            ? static_cast<int>(character.fields_.forbid_remaining_seconds_)
            : 0;
        if (forbid_active && forbid_remain <= 0) {
            packet.put_int(0);
            character.fields_.forbid_user_to_play_ = 0;
            character.fields_.forbid_remaining_seconds_ = 0;

            CStreamGuard stream_guard(
                GlobalData::stream_pool()->Acquire("user.cpp", 13388), true);
            *stream_guard << 147;
            *stream_guard << GetUID();
            SIG_FORBID_USER_TO_PLAY* const payload =
                stream_guard->GetInBuffer<SIG_FORBID_USER_TO_PLAY>();
            std::memset(payload, 0, sizeof(*payload));
            payload->charac_no = static_cast<uint32_t>(
                character.fields_.character_no_);
            payload->forbid = 0;
            payload->duration_days = 0;
            GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, stream_guard);
        } else {
            packet.put_int(ForbidRemainDays(character));
        }
    }
    packet.put_short(count_index, count);
    unlock();
    return 1;
}
