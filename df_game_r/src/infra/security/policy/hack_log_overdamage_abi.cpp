#include "infra/security/policy/hack_log_overdamage_abi.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"

namespace {

// IDA data at 0x8C10C68.  Entries are indexed by LEVEL_SECTION_TABLE.
const uint32_t kKillMonsterOverDamageConst[7] = {
    600u, 1800u, 3600u, 25000u, 55000u, 70000u, 700000u,
};

// IDA data at 0x8C10CC8.
const WongWork::ENUM_HACKTYPE kKillMonsterOverDamageHackType[7] = {
    static_cast<WongWork::ENUM_HACKTYPE>(101),
    static_cast<WongWork::ENUM_HACKTYPE>(102),
    static_cast<WongWork::ENUM_HACKTYPE>(103),
    static_cast<WongWork::ENUM_HACKTYPE>(104),
    static_cast<WongWork::ENUM_HACKTYPE>(105),
    static_cast<WongWork::ENUM_HACKTYPE>(106),
    static_cast<WongWork::ENUM_HACKTYPE>(107),
};

// IDA data at 0x8C10CA0.  CBattle_Field::get_dungeon_diff() indexes the
// four ordinary difficulty values directly.
const double kKillMonsterOverDamageDungeonRatio[4] = {
    1.8, 1.7, 1.6, 1.0,
};

int GetLevelSection(const int charac_level) {
    // This is the exact 89-entry LEVEL_SECTION_TABLE at 0x8C10D00, expressed
    // as its contiguous ranges: levels 1..9, 10..19, 20..29, 30..47,
    // 48..55, and 56..89.  The legacy server's character-level domain ends
    // within this table; retain the final section for a defensive outlier.
    if (charac_level <= 9) return 0;
    if (charac_level <= 19) return 1;
    if (charac_level <= 29) return 2;
    if (charac_level <= 47) return 3;
    if (charac_level <= 55) return 4;
    return 5;
}

}  // namespace

// IDA_EA: 0x8286790
// IDA_NAME: _ZN19CHackLog_OverDamage32GetDungeonLimitDamageAndHackTypeEP5CUserRjRN8WongWork13ENUM_HACKTYPEE
// DEMANGLED: CHackLog_OverDamage::GetDungeonLimitDamageAndHackType(CUser *,unsigned int &,WongWork::ENUM_HACKTYPE &)
int CHackLog_OverDamage::GetDungeonLimitDamageAndHackType(
    CUser* user,
    uint32_t& limit_damage,
    WongWork::ENUM_HACKTYPE& hack_type) {
    if (!user) return 0;

    CParty* const party = user->GetParty();
    if (!party) return 0;
    CDungeon* const dungeon = party->battle_field().GetDungeon();
    if (!dungeon) return 0;

    const int charac_level = user->get_charac_level();
    if (charac_level <= 0) return 0;

    int level_delta = charac_level - dungeon->get_min_level();
    if (level_delta < 0) level_delta = 0;

    int section = 6;
    double dungeon_ratio = 1.0;
    double level_ratio = 1.0;
    const int dungeon_index = dungeon->get_index();
    if (dungeon_index == 100) {
        dungeon_ratio = kKillMonsterOverDamageDungeonRatio[
            party->battle_field().get_dungeon_diff()];
    } else if (dungeon_index != 11000 && dungeon_index != 11001 &&
               dungeon_index != 11006) {
        level_ratio = static_cast<double>(level_delta / 20);
        if (level_ratio < 1.0) level_ratio = 1.0;
        section = GetLevelSection(charac_level);
        dungeon_ratio = kKillMonsterOverDamageDungeonRatio[
            party->battle_field().get_dungeon_diff()];
    }

    limit_damage = static_cast<uint32_t>(
        static_cast<long double>(kKillMonsterOverDamageConst[section]) *
        dungeon_ratio * level_ratio);
    hack_type = kKillMonsterOverDamageHackType[section];
    return 1;
}

// IDA_EA: 0x8286984
int CHackLog_OverDamage::GetDeathTowerLimitDamage() {
    return 700000;
}

// IDA_EA: 0x828698E
int CHackLog_OverDamage::GetDeathTowerOverDamageHackType() {
    return 107;
}

// IDA_EA: 0x8286998
int CHackLog_OverDamage::GetDungeonLimitMaxDamage() {
    return 700000;
}

// IDA_EA: 0x82869A2
int CHackLog_OverDamage::GetHackTypeOverLimitMaxDamage() {
    return 107;
}

// IDA_EA: 0x82869AC
int CHackLog_OverDamage::GetBossTowerLimitDamage() {
    return 70000;
}

// IDA_EA: 0x82869B6
int CHackLog_OverDamage::GetBossTowerOverDamageHackType() {
    return 133;
}
