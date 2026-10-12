#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_DUNGEON_STAT_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_DUNGEON_STAT_RUNTIME_H_

#include <cstddef>
#include <map>
#include <stdint.h>

#include "common/manager/play_info_public_abi.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/progression/quest/quest_skill_standalone_value_types.h"
#include "old_game/player/core/character_misc_data.h"

// Canonical dungeon-statistic values.  IData is the original empty marker
// base, so the i386 EBO keeps PlayInfo and job_count at offset zero.
struct Party : public IData, public PlayInfo {
    Party();
    ~Party();

    Party* Reset();

    int clear_count;
};

struct JobParty : public IData {
    JobParty();

    JobParty* Reset();

    int job_count;
};

static_assert(sizeof(Party) == 0x40u,
              "Party dungeon statistic ABI size must stay 0x40");
static_assert(sizeof(JobParty) == 0x04u,
              "JobParty dungeon statistic ABI size must stay 0x04");

// IDA unit: 99_misc_global/Monster.  This is the dungeon-statistics value,
// not a live CMonster game object.  The two totals and merge count are stored
// in CDungeonNode's map<int, Monster> exactly as the original did.
struct Monster : public IData {
    Monster();
    ~Monster();

    Monster* Reset();
    Monster& operator+=(const Monster& other);

    uint32_t damage_total;
    uint32_t damage_count;
    uint32_t merge_count;
};

static_assert(sizeof(Monster) == 0x0cu,
              "Monster dungeon statistic ABI size must stay 0x0C");

// IDA unit: 99_misc_global/JobKey.  Comparison deliberately ignores the
// trailing ABI padding; the original key order is job first, then grow type.
struct JobKey {
    JobKey();

    bool operator<(const JobKey& rhs) const;

    int job;                 // +0x00
    uint8_t grow_type;       // +0x04
    uint8_t reserved_05[3];  // +0x05, not part of the sort key
};

static_assert(sizeof(JobKey) == 0x08u,
              "JobKey dungeon statistic ABI size must stay 0x08");

// Wire record delivered by TimerDungeonDataStatistic.  It is read from the
// stream as a fixed 128-byte payload, so every member boundary below is part
// of the old protocol ABI rather than an internal convenience structure.
struct SIG_DUNGEON_DATA_STATISTIC {
    int signal_kind;        // +0x00
    DungeonKey dungeon_key; // +0x04
    int value_0c;           // +0x0c
    JobKey job_key;         // +0x10
    int monster_index;      // +0x18
    int skill_index;        // +0x1c
    uint8_t success;        // +0x20
    uint8_t reserved_21[3];
    Character character;    // +0x24
    Skill skill;            // +0x64
    Monster monster;        // +0x74
};

static_assert(sizeof(SIG_DUNGEON_DATA_STATISTIC) == 0x80u,
              "SIG_DUNGEON_DATA_STATISTIC must stay a 128-byte stream payload");
static_assert(offsetof(SIG_DUNGEON_DATA_STATISTIC, dungeon_key) == 0x04u,
              "dungeon statistic dungeon key offset must stay +0x04");
static_assert(offsetof(SIG_DUNGEON_DATA_STATISTIC, job_key) == 0x10u,
              "dungeon statistic job key offset must stay +0x10");
static_assert(offsetof(SIG_DUNGEON_DATA_STATISTIC, character) == 0x24u,
              "dungeon statistic character offset must stay +0x24");
static_assert(offsetof(SIG_DUNGEON_DATA_STATISTIC, skill) == 0x64u,
              "dungeon statistic skill offset must stay +0x64");
static_assert(offsetof(SIG_DUNGEON_DATA_STATISTIC, monster) == 0x74u,
              "dungeon statistic monster offset must stay +0x74");

class CSuccessJobNode {
public:
    CSuccessJobNode();
    CSuccessJobNode(const CSuccessJobNode& rhs);
    ~CSuccessJobNode();

    int UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC* signal);
    int InsertDB(const DungeonKey& dungeon_key,
                 bool success,
                 const JobKey& job_key);

private:
    std::map<int, JobParty> party_job_counts_;  // +0x00
    Character character_;                       // +0x18
};

class CDungeonSuccessNode {
public:
    CDungeonSuccessNode();
    CDungeonSuccessNode(const CDungeonSuccessNode& rhs);
    ~CDungeonSuccessNode();

    int UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC* signal);
    int InsertDB(const DungeonKey& dungeon_key, bool success);

private:
    std::map<int, Party> party_statistics_;               // +0x00
    std::map<JobKey, CSuccessJobNode> job_statistics_;    // +0x18
};

class CDungeonJobNode {
public:
    CDungeonJobNode();
    CDungeonJobNode(const CDungeonJobNode& rhs);
    ~CDungeonJobNode();

    int InsertSkillKey(int skill_index);
    int UpdateSkill(SIG_DUNGEON_DATA_STATISTIC* signal);
    int InsertDB(const DungeonKey& dungeon_key, int character_job);

private:
    std::map<int, Skill> skills_;  // +0x00
};

class CDungeonNode {
public:
    CDungeonNode();
    CDungeonNode(const CDungeonNode& rhs);
    ~CDungeonNode();

    int InsertMonsterKey(int monster_index);
    int InsertSkillKey(int skill_index,
                       int character_job,
                       int dungeon_index,
                       int dungeon_type);
    int UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC* signal);
    int UpdateSkill(SIG_DUNGEON_DATA_STATISTIC* signal);
    int UpdateMonster(SIG_DUNGEON_DATA_STATISTIC* signal);
    int InsertDB(const DungeonKey& dungeon_key);

private:
    std::map<int, Monster> monsters_;                 // +0x00
    std::map<int, CDungeonJobNode> job_nodes_;        // +0x18
    CDungeonSuccessNode success_nodes_[2];            // +0x30
};

class CDungeonDataManager {
public:
    CDungeonDataManager();
    ~CDungeonDataManager();

    int InsertDungeonKey(DungeonKey& dungeon_key);
    int InsertMonsterKey(int monster_index);
    int InsertSkillKey(int skill_index, int character_job);
    int UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC* signal);
    int UpdateSkill(SIG_DUNGEON_DATA_STATISTIC* signal);
    int UpdateMonster(SIG_DUNGEON_DATA_STATISTIC* signal);
    int UpdateDungeonDataStatistic();

private:
    std::map<DungeonKey, CDungeonNode> dungeon_nodes_;  // +0x00
    uint8_t value_18;                                  // +0x18
};

#if defined(__i386__)
static_assert(sizeof(CSuccessJobNode) == 0x58u,
              "CSuccessJobNode must preserve the old map-plus-Character ABI");
static_assert(sizeof(CDungeonSuccessNode) == 0x30u,
              "CDungeonSuccessNode must preserve the two-map ABI");
static_assert(sizeof(CDungeonJobNode) == 0x18u,
              "CDungeonJobNode must preserve the old map ABI");
static_assert(sizeof(CDungeonNode) == 0x90u,
              "CDungeonNode must preserve the old statistics graph ABI");
static_assert(sizeof(CDungeonDataManager) == 0x1cu,
              "CDungeonDataManager must preserve the old root-map ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_DUNGEON_STAT_RUNTIME_H_
