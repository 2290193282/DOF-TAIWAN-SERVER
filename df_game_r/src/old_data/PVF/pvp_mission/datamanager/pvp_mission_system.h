#ifndef OLDSRV_DATAMANAGER_PVP_MISSION_SYSTEM_H_
#define OLDSRV_DATAMANAGER_PVP_MISSION_SYSTEM_H_

#include "old_data/PVF/pvp_mission/combo_script_data.h"
#include "old_data/PVF/pvp_mission/combo_script_key.h"

#include <bitset>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <stdint.h>

class CMTRand;
class CUser;
struct MissionInfo;

enum PVP_BATTLE_MODE {
    PVP_BATTLE_MODE_0 = 0,
    PVP_BATTLE_MODE_1 = 1,
    PVP_BATTLE_MODE_2 = 2,
    PVP_BATTLE_MODE_3 = 3
};

#pragma pack(push, 1)
struct MissionClearCondition_Parameter {
    explicit MissionClearCondition_Parameter(PVP_BATTLE_MODE battle_mode);

    uint8_t battle_mode_category;  // +0x00
    uint16_t issue_history;        // +0x01
    int32_t kill_count;            // +0x03
    int32_t channel_type;          // +0x07
    int32_t pvp_channel_type;      // +0x0b
};
#pragma pack(pop)

struct MissionScript {
    int next_mission_index;       // +0x00
    int prev_mission_index;       // +0x04
    int mission_kind;             // +0x08
    int script_index;             // +0x0c
    std::pair<int, int> rank_range;  // +0x10
    std::pair<int, int> rate_range;  // +0x18
    int mission_grade;            // +0x20
    int target_character;         // +0x24
    std::vector<int> pre_required_missions;  // +0x28
    int condition_type;           // +0x34
    std::string name_text;        // +0x38
    std::string condition_text;   // +0x3c
    float difficulty_ratio;       // +0x40
    std::vector<std::pair<int, int> > reward_items;  // +0x44
    std::vector<int> int_values;  // +0x50
    bool stealing_skill;          // +0x5c
    bool reward_skill;            // +0x5d
    bool reward_sp;               // +0x5e
    char pad_5f;
    std::map<combo_script_key, combo_script_data> combo_clear;  // +0x60
    int pvp_battle_mode;          // +0x78
    int condition_param1;         // +0x7c
    int condition_param2;         // +0x80
    int condition_param3;         // +0x84

    MissionScript();
    ~MissionScript();
    MissionScript* clear();
};

struct MissionParameterScript {
    std::vector<unsigned int> frequent_daily_missions;  // +0x00
    unsigned int casual_daily_count;                    // +0x0c
    std::list<std::pair<unsigned int, unsigned int> >
        casual_daily_missions;                          // +0x10
    unsigned int casual_daily_total_weight;             // +0x18
    std::vector<unsigned int> base_mission_exp_by_rank; // +0x1c

    MissionParameterScript();
    ~MissionParameterScript();
    int clear();
};

class CMission {
public:
    explicit CMission(const MissionScript& script);

    virtual void _makeTotalMissionInfo(MissionInfo& info,
                                       const CUser& user) const;
    virtual bool _canUpdate(
        const MissionClearCondition_Parameter& parameter) const;
    virtual bool _isAcceptCondition(const CUser& user) const;
    virtual ~CMission();
    virtual bool isClearable(MissionInfo& info, bool& failed) const = 0;
    virtual bool verifyMission(const MissionInfo& info) const = 0;
    virtual bool isSeriesMission() const = 0;
    virtual int makeMissionInfo(MissionInfo& info, const CUser& user) const;
    virtual int Update(
        MissionInfo& info,
        const MissionClearCondition_Parameter& parameter) const = 0;
    virtual int getClearCondition() const = 0;
    virtual int Reward(CUser& user) const;
    virtual int _Reward(CUser& user) const;

    bool operator<(const CMission& rhs);
    int isAcceptCondition(const CUser& user) const;
    int canUpdate(const MissionClearCondition_Parameter& parameter) const;
    bool _isAcceptCondition_Series(const CUser& user) const;
    int GetMissionIndex() const;
    int GetMissionKind() const;
    int GetPrevMissionIndex() const;
    std::pair<int, int> GetRankRange() const;
    int GetMissionGrade() const;

protected:
    uint8_t pvp_battle_mode_;       // +0x04
    char pad_05_[3];
    int mission_kind_;              // +0x08
    int mission_index_;             // +0x0c
    int next_mission_index_;        // +0x10
    int prev_mission_index_;        // +0x14
    std::pair<int, int> rank_range_;  // +0x18
    std::pair<int, int> rate_range_;  // +0x20
    int mission_grade_;             // +0x28
    float difficulty_ratio_;        // +0x2c
    std::vector<std::pair<int, int> > reward_items_;  // +0x30
    int target_character_;          // +0x3c
};

#define DECLARE_ONE_PARAMETER_MISSION(name)                              \
class name : public CMission {                                           \
public:                                                                  \
    name(const MissionScript& script, int condition);                    \
    virtual ~name();                                                     \
    virtual bool verifyMission(const MissionInfo& info) const;           \
    virtual int Update(MissionInfo& info,                                \
                       const MissionClearCondition_Parameter& parameter) \
        const;                                                           \
    virtual bool isClearable(MissionInfo& info, bool& failed) const;      \
    virtual bool isSeriesMission() const;                                \
    virtual int getClearCondition() const;                               \
protected:                                                               \
    int condition_;                                                      \
};

DECLARE_ONE_PARAMETER_MISSION(CMission_hereafter_join)
DECLARE_ONE_PARAMETER_MISSION(CMission_revenge)
DECLARE_ONE_PARAMETER_MISSION(CMission_hereafter_kill_count)

#undef DECLARE_ONE_PARAMETER_MISSION

// The old vtable at 0x8CC3F88 retains the pure CMission::verifyMission slot,
// so this is intentionally abstract.  Factory @0x85E2132 has no condition-10
// arm; the seven old symbols below are nevertheless a real sibling vtable,
// not a synthetic condition or fallback provider.
class CMission_rank : public CMission {
public:
    virtual ~CMission_rank();
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool isAcceptCondition(const CUser& user) const;
    virtual int Update(
        MissionInfo& info,
        const MissionClearCondition_Parameter& parameter) const;
    virtual bool isSeriesMission() const;
    virtual int getClearCondition() const;

private:
    int condition_;  // +0x40
};

class CMission_winning_count : public CMission {
public:
    CMission_winning_count(const MissionScript& script, int condition);
    virtual ~CMission_winning_count();
    virtual void _makeTotalMissionInfo(MissionInfo& info,
                                       const CUser& user) const;
    virtual bool _canUpdate(
        const MissionClearCondition_Parameter& parameter) const;
    virtual bool _isAcceptCondition(const CUser& user) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
private:
    int condition_;
};

class CMission_winning_streak : public CMission {
public:
    CMission_winning_streak(const MissionScript& script,
                            int streak_length,
                            int clear_count);
    virtual ~CMission_winning_streak();
    virtual void _makeTotalMissionInfo(MissionInfo& info,
                                       const CUser& user) const;
    virtual bool _canUpdate(
        const MissionClearCondition_Parameter& parameter) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
private:
    int streak_length_;
    int clear_count_;
};

class CMission_join : public CMission {
public:
    CMission_join(const MissionScript& script, int condition);
    virtual ~CMission_join();
    virtual void _makeTotalMissionInfo(MissionInfo& info,
                                       const CUser& user) const;
    virtual bool _isAcceptCondition(const CUser& user) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
private:
    int condition_;
};

class CMission_kill_count : public CMission {
public:
    CMission_kill_count(const MissionScript& script, int condition);
    virtual ~CMission_kill_count();
    virtual void _makeTotalMissionInfo(MissionInfo& info,
                                       const CUser& user) const;
    virtual bool _isAcceptCondition(const CUser& user) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
private:
    int condition_;
};

class CMission_hereafter_winning_count : public CMission {
public:
    CMission_hereafter_winning_count(const MissionScript& script,
                                     int condition);
    virtual ~CMission_hereafter_winning_count();
    virtual bool _canUpdate(
        const MissionClearCondition_Parameter& parameter) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
private:
    int condition_;
};

class CMission_remain_hp : public CMission {
public:
    CMission_remain_hp(const MissionScript& script,
                       int remain_percent,
                       int clear_count);
    virtual ~CMission_remain_hp();
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
private:
    int remain_percent_;
    int clear_count_;
};

class CMission_kill : public CMission {
public:
    CMission_kill(const MissionScript& script,
                  int required_kills,
                  int clear_count);
    virtual ~CMission_kill();
    virtual bool _canUpdate(
        const MissionClearCondition_Parameter& parameter) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
private:
    int required_kills_;
    int clear_count_;
};

class CMission_within : public CMission {
public:
    CMission_within(const MissionScript& script,
                    int attempts,
                    int required_wins,
                    int unused);
    virtual ~CMission_within();
    virtual bool _isAcceptCondition(const CUser& user) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
private:
    int attempts_;
    int required_wins_;
    int unused_;
};

class CMission_remains_hp : public CMission {
public:
    CMission_remains_hp(const MissionScript& script,
                        int remain_percent,
                        int clear_count);
    virtual ~CMission_remains_hp();
    virtual bool _isAcceptCondition(const CUser& user) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
private:
    int remain_percent_;
    int clear_count_;
};

class CMission_move_channel : public CMission {
public:
    CMission_move_channel(const MissionScript& script, int channel_type);
    virtual ~CMission_move_channel();
    virtual bool _isAcceptCondition(const CUser& user) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
protected:
    int channel_type_;
};

class CMission_move_channel_total : public CMission {
public:
    CMission_move_channel_total(const MissionScript& script,
                                int channel_type);
    virtual ~CMission_move_channel_total();
    virtual bool _isAcceptCondition(const CUser& user) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
private:
    int channel_type_;
};

class CMission_combo_clear : public CMission {
public:
    CMission_combo_clear(
        const MissionScript& script,
        const std::map<combo_script_key, combo_script_data>& combo_clear);
    virtual ~CMission_combo_clear();
    virtual void _makeTotalMissionInfo(MissionInfo& info,
                                       const CUser& user) const;
    virtual bool _isAcceptCondition(const CUser& user) const;
    virtual bool isClearable(MissionInfo& info, bool& failed) const;
    virtual bool verifyMission(const MissionInfo& info) const;
    virtual bool isSeriesMission() const;
    virtual int Update(MissionInfo& info,
                       const MissionClearCondition_Parameter& parameter)
        const;
    virtual int getClearCondition() const;
    virtual int _Reward(CUser& user) const;
private:
    std::map<combo_script_key, combo_script_data> combo_clear_;
};

struct MissionLessThan {
    bool operator()(const CMission* lhs, const CMission* rhs);
};

class PvP_MissionSystem {
public:
    PvP_MissionSystem();
    ~PvP_MissionSystem();

    int Load_list();
    int Load_EtcParameter();
    int copy_missionParameter(const MissionParameterScript& script);
    int sort_list();
    CMission* Factory(const MissionScript& script);
    CMission* find_mission(int mission_index);
    std::list<CMission*>& get_kind_mission_list(unsigned int mission_kind);
    void get_New_DailyMission(
        std::vector<std::pair<unsigned int, unsigned int> >& missions);
    std::bitset<32> get_daily_mission_kind() const;
    unsigned int get_BaseMissionExp_byRank(unsigned short rank) const;
    int get_WithinMissionIndex(int pvp_grade) const;

private:
    typedef std::map<int, CMission*> MissionMap;
    typedef std::vector<std::list<CMission*> > KindMissionLists;

    MissionMap missions_;             // +0x00
    KindMissionLists kind_missions_;  // +0x18
    std::vector<unsigned int> frequent_daily_missions_;  // +0x24
    unsigned int casual_daily_count_; // +0x30
    std::list<std::pair<unsigned int, unsigned int> >
        casual_daily_missions_;       // +0x34
    unsigned int casual_daily_total_weight_;  // +0x3c
    std::bitset<32> daily_mission_kind_;      // +0x40
    CMTRand* random_;                          // +0x44
    std::vector<unsigned int> base_mission_exp_by_rank_; // +0x48
};

bool is_Victory_in_a_row(std::bitset<32> issue_history,
                         short required_count);
int _ReadThreeInt(MissionScript* script);
int _ReadFourInt(MissionScript* script);
int importPvpMissionScript(MissionScript* script, const char* filename);
int importPvpMissionParameterScript(MissionParameterScript& script);

#endif  // OLDSRV_DATAMANAGER_PVP_MISSION_SYSTEM_H_
