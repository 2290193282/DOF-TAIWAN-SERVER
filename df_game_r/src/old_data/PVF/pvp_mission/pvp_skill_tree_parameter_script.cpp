#include "old_data/PVF/pvp_mission/pvp_skill_tree_parameter_script.h"

#include "old_data/PVF/data/datamanager/script_file_list.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"

#include <string>


extern char g_pvpSkillTreeScriptBaseDirectory[256];
extern STScriptFileList g_pvpSkillTreeScriptFileList;

// IDA_EA: 0x8A5E6D8
// IDA_NAME: _ZNK15PvPSkillTreeKeyltERKS_
// DEMANGLED: PvPSkillTreeKey::operator<(PvPSkillTreeKey const&)const
bool PvPSkillTreeKey::operator<(const PvPSkillTreeKey& rhs) const {
    if (charac_job < rhs.charac_job) {
        return true;
    }
    if (charac_job != rhs.charac_job) {
        return false;
    }
    if (first_grow_type < rhs.first_grow_type) {
        return true;
    }
    return first_grow_type == rhs.first_grow_type &&
           second_grow_type < rhs.second_grow_type;
}

// IDA_EA: 0x8A5E74E
// IDA_NAME: _ZN15PvPSkillTreeKeyC2Ev
// DEMANGLED: PvPSkillTreeKey::PvPSkillTreeKey(void)
PvPSkillTreeKey::PvPSkillTreeKey()
    : charac_job(0), first_grow_type(0), second_grow_type(0) {}

// IDA_EA: 0x8A5E768
// IDA_NAME: _ZN15PvPSkillTreeKeyC2Ehhh
// DEMANGLED: PvPSkillTreeKey::PvPSkillTreeKey(unsigned char,unsigned char,unsigned char)
PvPSkillTreeKey::PvPSkillTreeKey(uint8_t charac_job_value,
                                 uint8_t first_grow_type_value,
                                 uint8_t second_grow_type_value)
    : charac_job(charac_job_value),
      first_grow_type(first_grow_type_value),
      second_grow_type(second_grow_type_value) {}

// IDA_EA: 0x8A5E7A0
// IDA_NAME: _ZN17PvPSkillTreeValueC2Ev
// DEMANGLED: PvPSkillTreeValue::PvPSkillTreeValue(void)
PvPSkillTreeValue::PvPSkillTreeValue()
    : normal_skill_tree(), pvp_skill_tree() {}

// IDA_EA: 0x8A5EAD4
// IDA_NAME: _ZN17PvPSkillTreeValueC2ERKS_
// DEMANGLED: PvPSkillTreeValue::PvPSkillTreeValue(PvPSkillTreeValue const&)
PvPSkillTreeValue::PvPSkillTreeValue(const PvPSkillTreeValue& rhs)
    : normal_skill_tree(rhs.normal_skill_tree),
      pvp_skill_tree(rhs.pvp_skill_tree) {}

// IDA_EA: 0x83F0004
// IDA_NAME: _ZN17PvPSkillTreeValueD2Ev
// DEMANGLED: PvPSkillTreeValue::~PvPSkillTreeValue()
PvPSkillTreeValue::~PvPSkillTreeValue() {}

// IDA_EA: 0x8A5E7E6
// IDA_NAME: _ZN35FAIR_PVP_SKILL_TREE_SKILL_PER_POINTC2Ev
// DEMANGLED: FAIR_PVP_SKILL_TREE_SKILL_PER_POINT::FAIR_PVP_SKILL_TREE_SKILL_PER_POINT(void)
FAIR_PVP_SKILL_TREE_SKILL_PER_POINT::
FAIR_PVP_SKILL_TREE_SKILL_PER_POINT()
    : skill_point(),
      static_basic_skill_point(),
      skill_per_point(),
      total_skill_point(0) {
    clear();
}

// IDA_EA: 0x8A5E874
// IDA_NAME: _ZN35FAIR_PVP_SKILL_TREE_SKILL_PER_POINT5clearEv
// DEMANGLED: FAIR_PVP_SKILL_TREE_SKILL_PER_POINT::clear(void)
FAIR_PVP_SKILL_TREE_SKILL_PER_POINT*
FAIR_PVP_SKILL_TREE_SKILL_PER_POINT::clear() {
    skill_point.clear();
    static_basic_skill_point.clear();
    skill_per_point.clear();
    total_skill_point = 0;
    return this;
}

// IDA_EA: 0x8A5E960
// IDA_NAME: _ZN35FAIR_PVP_SKILL_TREE_SKILL_PER_POINTC2ERKS_
// DEMANGLED: FAIR_PVP_SKILL_TREE_SKILL_PER_POINT::FAIR_PVP_SKILL_TREE_SKILL_PER_POINT(FAIR_PVP_SKILL_TREE_SKILL_PER_POINT const&)
FAIR_PVP_SKILL_TREE_SKILL_PER_POINT::
FAIR_PVP_SKILL_TREE_SKILL_PER_POINT(
    const FAIR_PVP_SKILL_TREE_SKILL_PER_POINT& rhs)
    : skill_point(rhs.skill_point),
      static_basic_skill_point(rhs.static_basic_skill_point),
      skill_per_point(rhs.skill_per_point),
      total_skill_point(rhs.total_skill_point) {}

// IDA_EA: 0x83CE2F8
// IDA_NAME: _ZN35FAIR_PVP_SKILL_TREE_SKILL_PER_POINTD2Ev
// DEMANGLED: FAIR_PVP_SKILL_TREE_SKILL_PER_POINT::~FAIR_PVP_SKILL_TREE_SKILL_PER_POINT()
FAIR_PVP_SKILL_TREE_SKILL_PER_POINT::
~FAIR_PVP_SKILL_TREE_SKILL_PER_POINT() {}

// IDA_EA: 0x8371E5A
// IDA_NAME: _ZN27PvPSkillTreeParameterScript5clearEv
// DEMANGLED: PvPSkillTreeParameterScript::clear(void)
void PvPSkillTreeParameterScript::clear() {
    skill_tree.clear();
    skill_points.clear();
    fair_skill_points.clear();
    max_pvp_skill_tree_level = 0;
    max_pvp_skill_point_grade = 0;
    version = 0;
    init_skill_tree.clear();
}

// IDA_EA: 0x8371EEE
// IDA_NAME: _ZN27PvPSkillTreeParameterScriptC2Ev
// DEMANGLED: PvPSkillTreeParameterScript::PvPSkillTreeParameterScript(void)
PvPSkillTreeParameterScript::PvPSkillTreeParameterScript()
    : skill_tree(),
      skill_points(),
      fair_skill_points(),
      init_skill_tree() {
    clear();
}

// IDA_EA: 0x837DA02
// IDA_NAME: _ZN27PvPSkillTreeParameterScriptD2Ev
// DEMANGLED: PvPSkillTreeParameterScript::~PvPSkillTreeParameterScript()
PvPSkillTreeParameterScript::~PvPSkillTreeParameterScript() {}

// IDA_EA: 0x84E913A
// IDA_NAME: _ZNK27PvPSkillTreeParameterScript21getInitSkillTreeGradeEi
// DEMANGLED: PvPSkillTreeParameterScript::getInitSkillTreeGrade(int)const
int PvPSkillTreeParameterScript::getInitSkillTreeGrade(
    int pvp_grade) const {
    std::map<int, int>::const_iterator it =
        init_skill_tree.find(pvp_grade);
    return it != init_skill_tree.end() ? it->second : 0;
}

// IDA_EA: 0x8A5CE28
// IDA_NAME: _ZN27PvPSkillTreeParameterScript24load_pvp_skill_tree_listEv
// DEMANGLED: PvPSkillTreeParameterScript::load_pvp_skill_tree_list(void)
bool PvPSkillTreeParameterScript::load_pvp_skill_tree_list() {
    for (STScriptFileList::iterator it = g_pvpSkillTreeScriptFileList.begin();
         it != g_pvpSkillTreeScriptFileList.end(); ++it) {
        const char* filename = it->second.c_str();
        if (!filename) {
            break;
        }

        if (importPvPSkillTreeParameterScript(filename) != 1) {
            LogManager::logFormat(
                1,
                "../RDARScriptPvPSkillTree.cpp",
                "load_pvp_skill_tree_list",
                0x45,
                "PvPSkillTreeParameterScript File Script Error: %s\n",
                filename);
            return false;
        }
    }

    return true;
}

// IDA_EA: 0x8A5CF16
// IDA_NAME: _ZN27PvPSkillTreeParameterScript33importPvPSkillTreeParameterScriptEPKc
// DEMANGLED: PvPSkillTreeParameterScript::importPvPSkillTreeParameterScript(char const*)
bool PvPSkillTreeParameterScript::importPvPSkillTreeParameterScript(
    const char* filename) {
    if (loadRDARScriptFile(g_pvpSkillTreeScriptBaseDirectory, filename) != 1) {
        return false;
    }

    std::string tag;
    bool success = false;
    PvPSkillTreeKey key;
    int level = 0;

    while (ScanType(tag, true) == 1) {
        if (tag == "[level]") {
            level = ScanInt(&success);
            if (!success) {
                return false;
            }
        } else if (tag == "[job index]") {
            key.charac_job = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return false;
            }
        } else if (tag == "[grow type index]") {
            key.first_grow_type =
                static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return false;
            }
        } else if (tag == "[awakening type]") {
            key.second_grow_type =
                static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return false;
            }
        } else if (tag == "[skill]") {
            PvPSkillTreeValue value;
            while (1) {
                const int skill_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int skill_level = ScanInt(&success);
                if (!success) {
                    break;
                }

                value.normal_skill_tree.push_back(
                    std::make_pair(skill_index, skill_level));
                FairPvpSkillPointMap::iterator fair_it =
                    fair_skill_points.find(key);
                if (fair_it != fair_skill_points.end()) {
                    std::map<int, int>::iterator skill_it =
                        fair_it->second.skill_point.find(skill_index);
                    if (skill_it != fair_it->second.skill_point.end()) {
                        skill_it->second = skill_level;
                    } else {
                        fair_it->second.skill_point.insert(
                            std::make_pair(skill_index, skill_level));
                    }
                } else {
                    FAIR_PVP_SKILL_TREE_SKILL_PER_POINT fair_value;
                    fair_value.skill_point.insert(
                        std::make_pair(skill_index, skill_level));
                    fair_skill_points.insert(
                        std::make_pair(key, fair_value));
                }
            }

            PvPSkillTreeMap::iterator tree_it = skill_tree.find(key);
            if (tree_it != skill_tree.end()) {
                if (!tree_it->second.insert(
                         std::make_pair(level, value)).second) {
                    return false;
                }
            } else {
                std::map<int, PvPSkillTreeValue> levels;
                if (!levels.insert(std::make_pair(level, value)).second) {
                    return false;
                }
                skill_tree.insert(std::make_pair(key, levels));
            }
        } else if (tag == "[static basic skill]") {
            PvPSkillTreeMap::iterator tree_it = skill_tree.find(key);
            if (tree_it == skill_tree.end()) {
                return false;
            }

            std::map<int, PvPSkillTreeValue>::iterator level_it =
                tree_it->second.find(level);
            if (level_it == tree_it->second.end()) {
                PvPSkillTreeValue value;
                level_it = tree_it->second.insert(
                    std::make_pair(level, value)).first;
            }

            while (1) {
                const int skill_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int skill_level = ScanInt(&success);
                if (!success) {
                    break;
                }

                level_it->second.pvp_skill_tree.push_back(
                    std::make_pair(skill_index, skill_level));
                FairPvpSkillPointMap::iterator fair_it =
                    fair_skill_points.find(key);
                if (fair_it == fair_skill_points.end()) {
                    return false;
                }
                std::map<int, int>::iterator static_it =
                    fair_it->second.static_basic_skill_point.find(
                        skill_index);
                if (static_it !=
                    fair_it->second.static_basic_skill_point.end()) {
                    static_it->second = skill_level;
                } else {
                    fair_it->second.static_basic_skill_point.insert(
                        std::make_pair(skill_index, skill_level));
                }
            }
        }
    }

    return true;
}

// IDA_EA: 0x8A5DB5A
// IDA_NAME: _ZNK27PvPSkillTreeParameterScript15getPvPSkillTreeEiiiibPPSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: PvPSkillTreeParameterScript::getPvPSkillTree(int,int,int,int,bool,std::vector<std::pair<int,int> >**)const
bool PvPSkillTreeParameterScript::getPvPSkillTree(
    int charac_job,
    int first_grow_type,
    int second_grow_type,
    int pvp_grade,
    bool use_pvp_skill_tree,
    std::vector<std::pair<int, int> >** result) const {
    const PvPSkillTreeKey key(static_cast<uint8_t>(charac_job),
                              static_cast<uint8_t>(first_grow_type),
                              static_cast<uint8_t>(second_grow_type));
    PvPSkillTreeMap::const_iterator tree_it = skill_tree.find(key);
    if (tree_it == skill_tree.end()) {
        return false;
    }
    std::map<int, PvPSkillTreeValue>::const_iterator level_it =
        tree_it->second.find(pvp_grade);
    if (level_it == tree_it->second.end()) {
        return false;
    }
    *result = use_pvp_skill_tree
                  ? const_cast<std::vector<std::pair<int, int> >*>(
                        &level_it->second.pvp_skill_tree)
                  : const_cast<std::vector<std::pair<int, int> >*>(
                        &level_it->second.normal_skill_tree);
    return true;
}

// IDA_EA: 0x8A5DC74
// IDA_NAME: _ZNK27PvPSkillTreeParameterScript20getGivePvPSkillPointEiiii
// DEMANGLED: PvPSkillTreeParameterScript::getGivePvPSkillPoint(int,int,int,int)const
int PvPSkillTreeParameterScript::getGivePvPSkillPoint(
    int charac_job,
    int first_grow_type,
    int second_grow_type,
    int pvp_grade) const {
    (void)second_grow_type;
    if (max_pvp_skill_point_grade < pvp_grade) {
        pvp_grade = max_pvp_skill_point_grade;
    }
    PvPSkillPointMap::const_iterator grade_it =
        skill_points.find(pvp_grade);
    if (grade_it == skill_points.end()) {
        return 0;
    }
    const int job_grow_key = first_grow_type + 5 * charac_job;
    std::map<int, int>::const_iterator point_it =
        grade_it->second.find(job_grow_key);
    return point_it != grade_it->second.end() ? point_it->second : 0;
}

// IDA_EA: 0x8A5DD62
// IDA_NAME: _ZNK27PvPSkillTreeParameterScript16getPvPSkillPointEiiiib
// DEMANGLED: PvPSkillTreeParameterScript::getPvPSkillPoint(int,int,int,int,bool)const
int PvPSkillTreeParameterScript::getPvPSkillPoint(
    int charac_job,
    int first_grow_type,
    int second_grow_type,
    int pvp_grade,
    bool subtract_static_skill_point) const {
    (void)second_grow_type;
    int skill_point = 0;
    if (max_pvp_skill_point_grade < pvp_grade) {
        pvp_grade = max_pvp_skill_point_grade;
    }
    PvPSkillPointMap::const_iterator grade_it =
        skill_points.find(pvp_grade);
    if (grade_it != skill_points.end()) {
        const int job_grow_key = first_grow_type + 5 * charac_job;
        std::map<int, int>::const_iterator point_it =
            grade_it->second.find(job_grow_key);
        if (point_it != grade_it->second.end()) {
            skill_point += point_it->second;
        }
    }

    const PvPSkillTreeKey key(static_cast<uint8_t>(charac_job),
                              static_cast<uint8_t>(first_grow_type),
                              0);
    FairPvpSkillPointMap::const_iterator fair_it =
        fair_skill_points.find(key);
    if (fair_it != fair_skill_points.end()) {
        if (subtract_static_skill_point) {
            if (skill_point) {
                skill_point -= fair_it->second.total_skill_point;
            }
        } else if (!skill_point) {
            skill_point = fair_it->second.total_skill_point;
        }
    }
    return skill_point;
}

// IDA_EA: 0x8A5DF0E
// IDA_NAME: _ZNK27PvPSkillTreeParameterScript20getPvPSkillPointNextEiiii
// DEMANGLED: PvPSkillTreeParameterScript::getPvPSkillPointNext(int,int,int,int)const
int PvPSkillTreeParameterScript::getPvPSkillPointNext(
    int charac_job,
    int first_grow_type,
    int second_grow_type,
    int pvp_grade) const {
    (void)second_grow_type;
    if (max_pvp_skill_point_grade < pvp_grade) {
        pvp_grade = max_pvp_skill_point_grade;
    }
    const int job_grow_key = first_grow_type + 5 * charac_job;

    int current_skill_point = 0;
    PvPSkillPointMap::const_iterator current_grade =
        skill_points.find(pvp_grade);
    if (current_grade == skill_points.end()) {
        return 0;
    }
    std::map<int, int>::const_iterator current_point =
        current_grade->second.find(job_grow_key);
    if (current_point != current_grade->second.end()) {
        current_skill_point = current_point->second;
    }

    int previous_skill_point = 0;
    const int previous_grade_value = pvp_grade - 1;
    PvPSkillPointMap::const_iterator previous_grade =
        skill_points.find(previous_grade_value);
    if (previous_grade != skill_points.end()) {
        std::map<int, int>::const_iterator previous_point =
            previous_grade->second.find(job_grow_key);
        if (previous_point != previous_grade->second.end()) {
            previous_skill_point = previous_point->second;
        }
    }

    int static_skill_point = 0;
    const PvPSkillTreeKey key(static_cast<uint8_t>(charac_job),
                              static_cast<uint8_t>(first_grow_type),
                              0);
    FairPvpSkillPointMap::const_iterator fair_it =
        fair_skill_points.find(key);
    if (fair_it != fair_skill_points.end()) {
        static_skill_point = fair_it->second.total_skill_point;
    }
    return current_skill_point - previous_skill_point - static_skill_point;
}

// IDA_EA: 0x8A5E178
// IDA_NAME: _ZNK27PvPSkillTreeParameterScript12getGiveSkillEiiiRSt6vectorISt4pairIiiESaIS2_EE
// DEMANGLED: PvPSkillTreeParameterScript::getGiveSkill(int,int,int,std::vector<std::pair<int,int> >&)const
bool PvPSkillTreeParameterScript::getGiveSkill(
    int charac_job,
    int first_grow_type,
    int second_grow_type,
    std::vector<std::pair<int, int> >& skills) const {
    (void)second_grow_type;
    const PvPSkillTreeKey key(static_cast<uint8_t>(charac_job),
                              static_cast<uint8_t>(first_grow_type),
                              0);
    FairPvpSkillPointMap::const_iterator it = fair_skill_points.find(key);
    if (it == fair_skill_points.end()) {
        return false;
    }
    skills = it->second.skill_per_point;
    return true;
}

// IDA_EA: 0x8A5E226
// IDA_NAME: _ZN27PvPSkillTreeParameterScript29importPairPvPSkillPointScriptEPKc
// DEMANGLED: PvPSkillTreeParameterScript::importPairPvPSkillPointScript(char const*)
bool PvPSkillTreeParameterScript::importPairPvPSkillPointScript(
    const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return false;
    }

    std::string tag;
    bool success = false;
    std::map<int, int> points_for_grade;
    int index = 0;
    int grade = 0;
    const int point_count_per_grade = 55;

    while (ScanType(tag, true) == 1) {
        if (tag == "[skill point]") {
            while (1) {
                const int point = static_cast<int>(ScanFloat(&success));
                if (!success) {
                    break;
                }

                points_for_grade.insert(std::make_pair(index++, point));
                if (static_cast<int>(points_for_grade.size()) ==
                    point_count_per_grade) {
                    skill_points.insert(
                        std::make_pair(grade++, points_for_grade));
                    points_for_grade.clear();
                    index = 0;
                }
            }

            if (static_cast<int>(points_for_grade.size()) !=
                    point_count_per_grade &&
                !points_for_grade.empty()) {
                return false;
            }
        } else if (tag == "[version]") {
            version = static_cast<uint8_t>(ScanInt(&success));
            if (!success) {
                return false;
            }
        } else if (tag == "[init skill tree]") {
            while (1) {
                const int pvp_grade = ScanInt(&success);
                if (!success) {
                    break;
                }
                const int init_grade = ScanInt(&success);
                if (!success) {
                    return false;
                }

                if (!init_skill_tree.insert(
                         std::make_pair(pvp_grade, init_grade)).second) {
                    return false;
                }
            }
        }
    }

    return true;
}
