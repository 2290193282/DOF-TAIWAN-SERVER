#ifndef OLDSRV_OLD_DATA_PVF_PVP_MISSION_PVP_SKILL_TREE_PARAMETER_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_PVP_MISSION_PVP_SKILL_TREE_PARAMETER_SCRIPT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <utility>
#include <vector>


struct PvPSkillTreeKey {
    PvPSkillTreeKey();
    PvPSkillTreeKey(uint8_t charac_job_value,
                    uint8_t first_grow_type_value,
                    uint8_t second_grow_type_value);

    bool operator<(const PvPSkillTreeKey& rhs) const;

    uint8_t charac_job;
    uint8_t first_grow_type;
    uint8_t second_grow_type;
};

struct PvPSkillTreeValue {
    PvPSkillTreeValue();
    PvPSkillTreeValue(const PvPSkillTreeValue& rhs);
    ~PvPSkillTreeValue();

    std::vector<std::pair<int, int> > normal_skill_tree;
    std::vector<std::pair<int, int> > pvp_skill_tree;
};

struct FAIR_PVP_SKILL_TREE_SKILL_PER_POINT {
    FAIR_PVP_SKILL_TREE_SKILL_PER_POINT();
    FAIR_PVP_SKILL_TREE_SKILL_PER_POINT(
        const FAIR_PVP_SKILL_TREE_SKILL_PER_POINT& rhs);
    ~FAIR_PVP_SKILL_TREE_SKILL_PER_POINT();

    FAIR_PVP_SKILL_TREE_SKILL_PER_POINT* clear();

    std::map<int, int> skill_point;
    std::map<int, int> static_basic_skill_point;
    std::vector<std::pair<int, int> > skill_per_point;
    int total_skill_point;
};

typedef std::map<PvPSkillTreeKey, std::map<int, PvPSkillTreeValue> >
    PvPSkillTreeMap;
typedef std::map<int, std::map<int, int> > PvPSkillPointMap;
typedef std::map<PvPSkillTreeKey,
                 FAIR_PVP_SKILL_TREE_SKILL_PER_POINT>
    FairPvpSkillPointMap;

class PvPSkillTreeParameterScript {
public:
    PvPSkillTreeParameterScript();
    ~PvPSkillTreeParameterScript();

    void clear();
    int getInitSkillTreeGrade(int pvp_grade) const;
    bool load_pvp_skill_tree_list();
    bool importPvPSkillTreeParameterScript(const char* filename);
    bool getPvPSkillTree(
        int charac_job,
        int first_grow_type,
        int second_grow_type,
        int pvp_grade,
        bool use_pvp_skill_tree,
        std::vector<std::pair<int, int> >** skill_tree) const;
    int getGivePvPSkillPoint(int charac_job,
                             int first_grow_type,
                             int second_grow_type,
                             int pvp_grade) const;
    int getPvPSkillPoint(int charac_job,
                         int first_grow_type,
                         int second_grow_type,
                         int pvp_grade,
                         bool subtract_static_skill_point) const;
    int getPvPSkillPointNext(int charac_job,
                             int first_grow_type,
                             int second_grow_type,
                             int pvp_grade) const;
    bool getGiveSkill(int charac_job,
                      int first_grow_type,
                      int second_grow_type,
                      std::vector<std::pair<int, int> >& skills) const;
    bool importPairPvPSkillPointScript(const char* filename);

    PvPSkillTreeMap skill_tree;
    PvPSkillPointMap skill_points;
    FairPvpSkillPointMap fair_skill_points;
    int max_pvp_skill_tree_level;
    int max_pvp_skill_point_grade;
    uint8_t version;
    uint8_t reserved_51[3];
    std::map<int, int> init_skill_tree;
};

static_assert(sizeof(PvPSkillTreeKey) == 0x03u,
              "PvPSkillTreeKey old i386 size must stay 0x03");
static_assert(sizeof(PvPSkillTreeValue) == 0x18u,
              "PvPSkillTreeValue old i386 size must stay 0x18");
static_assert(sizeof(FAIR_PVP_SKILL_TREE_SKILL_PER_POINT) == 0x40u,
              "FAIR_PVP_SKILL_TREE_SKILL_PER_POINT old i386 size must stay 0x40");
static_assert(sizeof(PvPSkillTreeParameterScript) == 0x6cu,
              "PvPSkillTreeParameterScript old i386 size must stay 0x6c");
static_assert(offsetof(PvPSkillTreeParameterScript, version) == 0x50u,
              "PvP skill-tree script version must stay at old +0x50");

#endif  // OLDSRV_OLD_DATA_PVF_PVP_MISSION_PVP_SKILL_TREE_PARAMETER_SCRIPT_H_
