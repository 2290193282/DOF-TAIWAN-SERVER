#include "old_game/dungeon/instance/dungeon_stat_runtime.h"

#include <utility>

#include "common/config.h"
#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"

namespace {

int GetStatisticPartyMemberCount(const SIG_DUNGEON_DATA_STATISTIC& signal) {
    // SIG_DUNGEON_DATA_STATISTIC::character starts at +0x24 and the original
    // read at signal+0x44 is PlayInfo::party_member_count_ at Character +0x20.
    return static_cast<int>(signal.character.party_member_count_);
}

void InitializePartyCountMap(std::map<int, Party>& values) {
    for (int count = 2; count <= 4; ++count) {
        values.insert(std::make_pair(count, Party()));
    }
}

void InitializeJobPartyCountMap(std::map<int, JobParty>& values) {
    for (int count = 2; count <= 4; ++count) {
        values.insert(std::make_pair(count, JobParty()));
    }
}

int ChannelNumber() {
    return G_CEnvironment()->get_channel_no();
}

}  // namespace

// IDA_EA: 0x84865E2
// IDA_NAME: _ZNK6JobKeyltERKS_
// DEMANGLED: JobKey::operator<(JobKey const&)const
bool JobKey::operator<(const JobKey& rhs) const {
    return job < rhs.job || (job == rhs.job && grow_type < rhs.grow_type);
}

// IDA_EA: 0x848662A
// IDA_NAME: _ZN6JobKeyC2Ev
// DEMANGLED: JobKey::JobKey(void)
JobKey::JobKey() : job(0), grow_type(0) {}

// IDA_EA: 0x8484C30
// IDA_NAME: _ZN15CSuccessJobNodeC2Ev
// DEMANGLED: CSuccessJobNode::CSuccessJobNode(void)
CSuccessJobNode::CSuccessJobNode()
    : party_job_counts_(), character_() {
    InitializeJobPartyCountMap(party_job_counts_);
}

// IDA_EA: 0x848757E
// IDA_NAME: _ZN15CSuccessJobNodeC2ERKS_
// DEMANGLED: CSuccessJobNode::CSuccessJobNode(CSuccessJobNode const&)
CSuccessJobNode::CSuccessJobNode(const CSuccessJobNode& rhs)
    : party_job_counts_(rhs.party_job_counts_), character_(rhs.character_) {}

// IDA_EA: 0x8486C5A
// IDA_NAME: _ZN15CSuccessJobNodeD2Ev
// DEMANGLED: CSuccessJobNode::~CSuccessJobNode()
CSuccessJobNode::~CSuccessJobNode() {}

// IDA_EA: 0x8484D4C
// IDA_NAME: _ZN15CSuccessJobNode15UpdateCharacterEP26SIG_DUNGEON_DATA_STATISTIC
// DEMANGLED: CSuccessJobNode::UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC *)
int CSuccessJobNode::UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC* signal) {
    std::map<int, JobParty>::iterator found =
        party_job_counts_.find(GetStatisticPartyMemberCount(*signal));
    if (found != party_job_counts_.end()) {
        ++found->second.job_count;
    }
    character_ += signal->character;
    return 1;
}

// IDA_EA: 0x8486234
// IDA_NAME: _ZN15CSuccessJobNode8InsertDBERK10DungeonKeybRK6JobKey
// DEMANGLED: CSuccessJobNode::InsertDB(DungeonKey const&,bool,JobKey const&)
int CSuccessJobNode::InsertDB(const DungeonKey& dungeon_key,
                              bool success,
                              const JobKey& job_key) {
    if (character_.dword_3c) {
        MySQL* const database = GetMySqLInstance();
        database->set_query(
            "inSert into log_dungeon_charac(clear_time,die_count,hp_consume,mp_consume,hit_count,hit_per_avg_damage,hp_recovery,mp_recovery,update_count,channel_no,dungeon_index,dungeon_diff,success,charac_job,level,charac_grow,last_time,exp_avg) values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,NOW(),%d)",
            static_cast<int>(character_.clear_time_seconds_),
            static_cast<int>(character_.die_count_),
            static_cast<int>(character_.hp_consume_),
            static_cast<int>(character_.mp_consume_),
            static_cast<int>(character_.hit_count_),
            static_cast<int>(character_.hit_per_average_damage_),
            static_cast<int>(character_.hp_recovery_),
            static_cast<int>(character_.mp_recovery_),
            static_cast<int>(character_.dword_3c),
            ChannelNumber(), dungeon_key.dungeon_no, dungeon_key.dungeon_type,
            success, job_key.job,
            static_cast<int>(character_.character_level_),
            static_cast<int>(job_key.grow_type),
            static_cast<int>(character_.dungeon_exp_));
        if (!database->exec(true)) {
            cMyTrace("bool CSuccessJobNode::InsertDB(const DungeonKey&, bool, const JobKey&)",
                     498, 5)("inSert log_dungeon_charac failed");
            return 0;
        }
    }

    character_.Reset();
    for (std::map<int, JobParty>::iterator it = party_job_counts_.begin();
         it != party_job_counts_.end(); ++it) {
        if (it->second.job_count) {
            MySQL* const database = GetMySqLInstance();
            database->set_query(
                "inSert into log_dungeon_party_job(channel_no,dungeon_index,dungeon_diff,charac_job,charac_grow,party_user_count,success,job_count,last_time) values(%d,%d,%d,%d,%d,%d,%d,%d,NOW())",
                ChannelNumber(), dungeon_key.dungeon_no, dungeon_key.dungeon_type,
                job_key.job, static_cast<int>(job_key.grow_type), it->first,
                success, it->second.job_count);
            if (!database->exec(true)) {
                cMyTrace("bool CSuccessJobNode::InsertDB(const DungeonKey&, bool, const JobKey&)",
                         533, 5)("inSert log_dungeon_party_job failed");
                return 0;
            }
        }
        it->second.Reset();
    }
    return 1;
}

// IDA_EA: 0x8484DDA
// IDA_NAME: _ZN19CDungeonSuccessNodeC2Ev
// DEMANGLED: CDungeonSuccessNode::CDungeonSuccessNode(void)
CDungeonSuccessNode::CDungeonSuccessNode()
    : party_statistics_(), job_statistics_() {
    InitializePartyCountMap(party_statistics_);
    for (int job = 0; job < oldsrv::config::kCharacterJobCount; ++job) {
        for (int first_grow = 0; first_grow <= 5; ++first_grow) {
            for (int second_grow = 0; second_grow <= 2; ++second_grow) {
                JobKey key;
                key.job = job;
                key.grow_type = static_cast<uint8_t>(
                    first_grow | (second_grow << 4));
                job_statistics_.insert(std::make_pair(key, CSuccessJobNode()));
            }
        }
    }
}

// IDA_EA: 0x8487A30
// IDA_NAME: _ZN19CDungeonSuccessNodeC2ERKS_
// DEMANGLED: CDungeonSuccessNode::CDungeonSuccessNode(CDungeonSuccessNode const&)
CDungeonSuccessNode::CDungeonSuccessNode(const CDungeonSuccessNode& rhs)
    : party_statistics_(rhs.party_statistics_),
      job_statistics_(rhs.job_statistics_) {}

// IDA_EA: 0x8486AA0
// IDA_NAME: _ZN19CDungeonSuccessNodeD2Ev
// DEMANGLED: CDungeonSuccessNode::~CDungeonSuccessNode()
CDungeonSuccessNode::~CDungeonSuccessNode() {}

// IDA_EA: 0x84851DE
// IDA_NAME: _ZN19CDungeonSuccessNode15UpdateCharacterEP26SIG_DUNGEON_DATA_STATISTIC
// DEMANGLED: CDungeonSuccessNode::UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC *)
int CDungeonSuccessNode::UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC* signal) {
    std::map<int, Party>::iterator party =
        party_statistics_.find(GetStatisticPartyMemberCount(*signal));
    if (party != party_statistics_.end()) {
        static_cast<PlayInfo&>(party->second) +=
            static_cast<const PlayInfo&>(signal->character);
        ++party->second.clear_count;
    }

    std::map<JobKey, CSuccessJobNode>::iterator job =
        job_statistics_.find(signal->job_key);
    if (job != job_statistics_.end()) {
        job->second.UpdateCharacter(signal);
    }
    return 1;
}

// IDA_EA: 0x8485F8A
// IDA_NAME: _ZN19CDungeonSuccessNode8InsertDBERK10DungeonKeyb
// DEMANGLED: CDungeonSuccessNode::InsertDB(DungeonKey const&,bool)
int CDungeonSuccessNode::InsertDB(const DungeonKey& dungeon_key, bool success) {
    for (std::map<JobKey, CSuccessJobNode>::iterator it =
             job_statistics_.begin();
         it != job_statistics_.end(); ++it) {
        it->second.InsertDB(dungeon_key, success, it->first);
    }

    for (std::map<int, Party>::iterator it = party_statistics_.begin();
         it != party_statistics_.end(); ++it) {
        Party& party = it->second;
        if (party.clear_count) {
            MySQL* const database = GetMySqLInstance();
            database->set_query(
                "inSert into log_dungeon_party(clear_time,die_count,hp_consume,mp_consume,hit_count,hit_per_avg_damage,hp_recovery,mp_recovery,update_count,channel_no,dungeon_index,dungeon_diff,success,level,party_user_count,last_time) values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,NOW())",
                static_cast<int>(party.clear_time_seconds_),
                static_cast<int>(party.die_count_),
                static_cast<int>(party.hp_consume_),
                static_cast<int>(party.mp_consume_),
                static_cast<int>(party.hit_count_),
                static_cast<int>(party.hit_per_average_damage_),
                static_cast<int>(party.hp_recovery_),
                static_cast<int>(party.mp_recovery_),
                party.clear_count, ChannelNumber(), dungeon_key.dungeon_no,
                dungeon_key.dungeon_type, success,
                static_cast<int>(party.character_level_), it->first);
            if (!database->exec(true)) {
                cMyTrace("bool CDungeonSuccessNode::InsertDB(const DungeonKey&, bool)",
                         441, 5)("inSert log_dungeon_party failed");
                return 0;
            }
        }
        party.Reset();
    }
    return 1;
}

// IDA_EA: 0x8486CE8
// IDA_NAME: _ZN15CDungeonJobNodeC2Ev
// DEMANGLED: CDungeonJobNode::CDungeonJobNode(void)
CDungeonJobNode::CDungeonJobNode() : skills_() {}

// IDA_EA: 0x84878D0
// IDA_NAME: _ZN15CDungeonJobNodeC2ERKS_
// DEMANGLED: CDungeonJobNode::CDungeonJobNode(CDungeonJobNode const&)
CDungeonJobNode::CDungeonJobNode(const CDungeonJobNode& rhs)
    : skills_(rhs.skills_) {}

// IDA_EA: 0x8486CFC
// IDA_NAME: _ZN15CDungeonJobNodeD2Ev
// DEMANGLED: CDungeonJobNode::~CDungeonJobNode()
CDungeonJobNode::~CDungeonJobNode() {}

// IDA_EA: 0x8484A86
// IDA_NAME: _ZN15CDungeonJobNode14InsertSkillKeyEi
// DEMANGLED: CDungeonJobNode::InsertSkillKey(int)
int CDungeonJobNode::InsertSkillKey(int skill_index) {
    if (skills_.find(skill_index) != skills_.end()) {
        return 0;
    }
    skills_.insert(std::make_pair(skill_index, Skill()));
    return 1;
}

// IDA_EA: 0x8484BAC
// IDA_NAME: _ZN15CDungeonJobNode11UpdateSkillEP26SIG_DUNGEON_DATA_STATISTIC
// DEMANGLED: CDungeonJobNode::UpdateSkill(SIG_DUNGEON_DATA_STATISTIC *)
int CDungeonJobNode::UpdateSkill(SIG_DUNGEON_DATA_STATISTIC* signal) {
    std::map<int, Skill>::iterator it = skills_.find(signal->skill_index);
    if (it == skills_.end()) {
        return 0;
    }
    it->second += signal->skill;
    return 1;
}

// IDA_EA: 0x8485E00
// IDA_NAME: _ZN15CDungeonJobNode8InsertDBERK10DungeonKeyi
// DEMANGLED: CDungeonJobNode::InsertDB(DungeonKey const&,int)
int CDungeonJobNode::InsertDB(const DungeonKey& dungeon_key,
                              int character_job) {
    for (std::map<int, Skill>::iterator it = skills_.begin();
         it != skills_.end(); ++it) {
        Skill& skill = it->second;
        if (skill.merge_count_0c) {
            MySQL* const database = GetMySqLInstance();
            database->set_query(
                "inSert into log_dungeon_skill(skill_cool_time,skill_use,skill_count,channel_no,dungeon_index,dungeon_diff,charac_job,charac_grow,skill_index,last_time) values(%d,%d,%d,%d,%d,%d,%d,%d,%d,NOW())",
                static_cast<int>(skill.value_04), static_cast<int>(skill.value_08),
                static_cast<int>(skill.value_00), ChannelNumber(),
                dungeon_key.dungeon_no, dungeon_key.dungeon_type,
                character_job, 0, it->first);
            if (!database->exec(true)) {
                cMyTrace("bool CDungeonJobNode::InsertDB(const DungeonKey&, int)",
                         375, 5)("inSert log_dungeon_skill failed");
                return 0;
            }
        }
        skill.Reset();
    }
    return 1;
}

// IDA_EA: 0x8486AE6
// IDA_NAME: _ZN12CDungeonNodeC2Ev
// DEMANGLED: CDungeonNode::CDungeonNode(void)
CDungeonNode::CDungeonNode()
    : monsters_(), job_nodes_(), success_nodes_() {}

// IDA_EA: 0x8487A86
// IDA_NAME: _ZN12CDungeonNodeC2ERKS_
// DEMANGLED: CDungeonNode::CDungeonNode(CDungeonNode const&)
CDungeonNode::CDungeonNode(const CDungeonNode& rhs)
    : monsters_(rhs.monsters_),
      job_nodes_(rhs.job_nodes_),
      success_nodes_{rhs.success_nodes_[0], rhs.success_nodes_[1]} {}

// IDA_EA: 0x8486D3C
// IDA_NAME: _ZN12CDungeonNodeD2Ev
// DEMANGLED: CDungeonNode::~CDungeonNode()
CDungeonNode::~CDungeonNode() {}

// IDA_EA: 0x84852DE
// IDA_NAME: _ZN12CDungeonNode16InsertMonsterKeyEi
// DEMANGLED: CDungeonNode::InsertMonsterKey(int)
int CDungeonNode::InsertMonsterKey(int monster_index) {
    if (monsters_.find(monster_index) != monsters_.end()) {
        return 0;
    }
    monsters_.insert(std::make_pair(monster_index, Monster()));
    return 1;
}

// IDA_EA: 0x8485404
// IDA_NAME: _ZN12CDungeonNode14InsertSkillKeyEiiii
// DEMANGLED: CDungeonNode::InsertSkillKey(int,int,int,int)
int CDungeonNode::InsertSkillKey(int skill_index,
                                 int character_job,
                                 int /* dungeon_index */,
                                 int /* dungeon_type */) {
    std::map<int, CDungeonJobNode>::iterator it =
        job_nodes_.find(character_job);
    if (it == job_nodes_.end()) {
        it = job_nodes_.insert(std::make_pair(character_job,
                                               CDungeonJobNode())).first;
    }
    it->second.InsertSkillKey(skill_index);
    return 1;
}

// IDA_EA: 0x8485564
// IDA_NAME: _ZN12CDungeonNode15UpdateCharacterEP26SIG_DUNGEON_DATA_STATISTIC
// DEMANGLED: CDungeonNode::UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC *)
int CDungeonNode::UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC* signal) {
    success_nodes_[signal->success].UpdateCharacter(signal);
    return 1;
}

// IDA_EA: 0x848559A
// IDA_NAME: _ZN12CDungeonNode11UpdateSkillEP26SIG_DUNGEON_DATA_STATISTIC
// DEMANGLED: CDungeonNode::UpdateSkill(SIG_DUNGEON_DATA_STATISTIC *)
int CDungeonNode::UpdateSkill(SIG_DUNGEON_DATA_STATISTIC* signal) {
    std::map<int, CDungeonJobNode>::iterator it =
        job_nodes_.find(signal->job_key.job);
    if (it == job_nodes_.end()) {
        return 0;
    }
    return it->second.UpdateSkill(signal);
}

// IDA_EA: 0x8485628
// IDA_NAME: _ZN12CDungeonNode13UpdateMonsterEP26SIG_DUNGEON_DATA_STATISTIC
// DEMANGLED: CDungeonNode::UpdateMonster(SIG_DUNGEON_DATA_STATISTIC *)
int CDungeonNode::UpdateMonster(SIG_DUNGEON_DATA_STATISTIC* signal) {
    std::map<int, Monster>::iterator it = monsters_.find(signal->monster_index);
    if (it == monsters_.end()) {
        return 0;
    }
    it->second += signal->monster;
    return 1;
}

// IDA_EA: 0x8485BA4
// IDA_NAME: _ZN12CDungeonNode8InsertDBERK10DungeonKey
// DEMANGLED: CDungeonNode::InsertDB(DungeonKey const&)
int CDungeonNode::InsertDB(const DungeonKey& dungeon_key) {
    for (std::map<int, Monster>::iterator it = monsters_.begin();
         it != monsters_.end(); ++it) {
        Monster& monster = it->second;
        if (monster.merge_count) {
            MySQL* const database = GetMySqLInstance();
            database->set_query(
                "inSert into log_dungeon_monster(channel_no,dungeon_index,dungeon_diff,monster_index,damage_total,damage_count,last_time) values(%d,%d,%d,%d,%d,%d,NOW())",
                ChannelNumber(), dungeon_key.dungeon_no, dungeon_key.dungeon_type,
                it->first, static_cast<int>(monster.damage_total),
                static_cast<int>(monster.damage_count));
            if (!database->exec(true)) {
                cMyTrace("bool CDungeonNode::InsertDB(const DungeonKey&)",
                         317, 5)("inSert log_dungeon_monster failed ");
                return 0;
            }
        }
        monster.Reset();
    }

    for (std::map<int, CDungeonJobNode>::iterator it = job_nodes_.begin();
         it != job_nodes_.end(); ++it) {
        it->second.InsertDB(dungeon_key, it->first);
    }
    success_nodes_[0].InsertDB(dungeon_key, false);
    success_nodes_[1].InsertDB(dungeon_key, true);
    return 1;
}

// IDA_EA: 0x8486BB4
// IDA_NAME: _ZN19CDungeonDataManagerC2Ev
// DEMANGLED: CDungeonDataManager::CDungeonDataManager(void)
CDungeonDataManager::CDungeonDataManager() : dungeon_nodes_(), value_18(0) {}

// IDA_EA: 0x8486BDA
// IDA_NAME: _ZN19CDungeonDataManagerD2Ev
// DEMANGLED: CDungeonDataManager::~CDungeonDataManager()
CDungeonDataManager::~CDungeonDataManager() {}

// IDA_EA: 0x84856AC
// IDA_NAME: _ZN19CDungeonDataManager16InsertDungeonKeyER10DungeonKey
// DEMANGLED: CDungeonDataManager::InsertDungeonKey(DungeonKey &)
int CDungeonDataManager::InsertDungeonKey(DungeonKey& dungeon_key) {
    if (dungeon_nodes_.find(dungeon_key) != dungeon_nodes_.end()) {
        return 0;
    }
    dungeon_nodes_.insert(std::make_pair(dungeon_key, CDungeonNode()));
    return 1;
}

// IDA_EA: 0x848580A
// IDA_NAME: _ZN19CDungeonDataManager16InsertMonsterKeyEi
// DEMANGLED: CDungeonDataManager::InsertMonsterKey(int)
int CDungeonDataManager::InsertMonsterKey(int monster_index) {
    for (std::map<DungeonKey, CDungeonNode>::iterator it =
             dungeon_nodes_.begin();
         it != dungeon_nodes_.end(); ++it) {
        it->second.InsertMonsterKey(monster_index);
    }
    return 1;
}

// IDA_EA: 0x848589A
// IDA_NAME: _ZN19CDungeonDataManager14InsertSkillKeyEii
// DEMANGLED: CDungeonDataManager::InsertSkillKey(int,int)
int CDungeonDataManager::InsertSkillKey(int skill_index, int character_job) {
    for (std::map<DungeonKey, CDungeonNode>::iterator it =
             dungeon_nodes_.begin();
         it != dungeon_nodes_.end(); ++it) {
        it->second.InsertSkillKey(skill_index, character_job,
                                  it->first.dungeon_no,
                                  it->first.dungeon_type);
    }
    return 1;
}

// IDA_EA: 0x8485956
// IDA_NAME: _ZN19CDungeonDataManager15UpdateCharacterEP26SIG_DUNGEON_DATA_STATISTIC
// DEMANGLED: CDungeonDataManager::UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC *)
int CDungeonDataManager::UpdateCharacter(SIG_DUNGEON_DATA_STATISTIC* signal) {
    std::map<DungeonKey, CDungeonNode>::iterator it =
        dungeon_nodes_.find(signal->dungeon_key);
    return it == dungeon_nodes_.end() ? 0 : it->second.UpdateCharacter(signal);
}

// IDA_EA: 0x84859D8
// IDA_NAME: _ZN19CDungeonDataManager11UpdateSkillEP26SIG_DUNGEON_DATA_STATISTIC
// DEMANGLED: CDungeonDataManager::UpdateSkill(SIG_DUNGEON_DATA_STATISTIC *)
int CDungeonDataManager::UpdateSkill(SIG_DUNGEON_DATA_STATISTIC* signal) {
    std::map<DungeonKey, CDungeonNode>::iterator it =
        dungeon_nodes_.find(signal->dungeon_key);
    return it == dungeon_nodes_.end() ? 0 : it->second.UpdateSkill(signal);
}

// IDA_EA: 0x8485A5A
// IDA_NAME: _ZN19CDungeonDataManager13UpdateMonsterEP26SIG_DUNGEON_DATA_STATISTIC
// DEMANGLED: CDungeonDataManager::UpdateMonster(SIG_DUNGEON_DATA_STATISTIC *)
int CDungeonDataManager::UpdateMonster(SIG_DUNGEON_DATA_STATISTIC* signal) {
    std::map<DungeonKey, CDungeonNode>::iterator it =
        dungeon_nodes_.find(signal->dungeon_key);
    return it == dungeon_nodes_.end() ? 0 : it->second.UpdateMonster(signal);
}

// IDA_EA: 0x8485ADC
// IDA_NAME: _ZN19CDungeonDataManager26UpdateDungeonDataStatisticEv
// DEMANGLED: CDungeonDataManager::UpdateDungeonDataStatistic(void)
int CDungeonDataManager::UpdateDungeonDataStatistic() {
    for (std::map<DungeonKey, CDungeonNode>::iterator it =
             dungeon_nodes_.begin();
         it != dungeon_nodes_.end(); ++it) {
        it->second.InsertDB(it->first);
    }
    return 1;
}

// IDA_EA: 0x84849B4
// IDA_NAME: _Z29GetInstanceDungeonDataManagerv
// DEMANGLED: GetInstanceDungeonDataManager(void)
CDungeonDataManager* GetInstanceDungeonDataManager() {
    static CDungeonDataManager instance;
    return &instance;
}

// IDA_EA: 0x8485B79
// IDA_NAME: _Z16GetMySqLInstancev
// DEMANGLED: GetMySqLInstance(void)
MySQL* GetMySqLInstance() {
    return GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_LOG,
                                               SERVER_GROUP_DEFAULT);
}
