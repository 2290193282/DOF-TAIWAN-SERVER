#include "old_data/PVF/itemshop/datamanager/skill_shop_script.h"

#include "common/config.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

#include <utility>

int getGrowTypeIndex(const char* text) __asm__("_Z16getGrowTypeIndexPKc");


// IDA_EA: 0x8913F48
// IDA_NAME: _Z24importCommonSkillIndexesPKcRSt6vectorIiSaIiEERSt3mapI17ENUM_CHARACTERJOBSt3setIiSt4lessIiES2_ES8_IS6_ESaISt4pairIKS6_SA_EEE
// DEMANGLED: importCommonSkillIndexes(char const*,std::vector<int,std::allocator<int>> &,std::map<ENUM_CHARACTERJOB,std::set<int,std::less<int>,std::allocator<int>>,std::less<ENUM_CHARACTERJOB>,std::allocator<std::pair<ENUM_CHARACTERJOB const,std::set<int,std::less<int>,std::allocator<int>>>>> &)
int importCommonSkillIndexes(
    const char* filename,
    std::vector<int>& common_skill_indexes,
    std::map<ENUM_CHARACTERJOB, std::set<int> >& except_skill_info) {
    if (!loadRDARScriptFile("", filename)) {
        LogManager::logFormat(0,
                              "../RDARScriptEtcParameter.cpp",
                              "importCommonSkillIndexes",
                              0x329A,
                              "ERROR : %s ",
                              filename);
        return 0;
    }

    std::string tag;
    bool success = false;
    common_skill_indexes.clear();
    except_skill_info.clear();

    while (ScanType(tag, true) == 1) {
        if (tag == "[common skill]") {
            while (1) {
                const int skill_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                common_skill_indexes.push_back(skill_index);
            }
        } else if (tag == "[except skill info]") {
            std::set<int> except_indexes;
            while (ScanType(tag, true) == 1 &&
                   tag != "[/except skill info]") {
                ENUM_CHARACTERJOB character_job =
                    static_cast<ENUM_CHARACTERJOB>(
                        oldsrv::config::kCharacterJobCommon);
                except_indexes.clear();

                std::string job_name;
                if (ScanStr(&job_name) != 1) {
                    return 0;
                }

                character_job = static_cast<ENUM_CHARACTERJOB>(
                    ::getCharacterJob(job_name.c_str()));
                while (1) {
                    const int skill_index = ScanInt(&success);
                    if (!success) {
                        break;
                    }
                    except_indexes.insert(skill_index);
                }

                except_skill_info.insert(
                    std::make_pair(character_job, except_indexes));
            }
        } else {
            (void)(tag == "[/common skill]");
        }
    }

    return 1;
}

// IDA_EA: 0x891427F
// IDA_NAME: _Z19importSkillShopTreePKcRSt3mapI17ENUM_CHARACTERJOBSsSt4lessIS2_ESaISt4pairIKS2_SsEEE
// DEMANGLED: importSkillShopTree(char const*,std::map<ENUM_CHARACTERJOB,std::string,std::less<ENUM_CHARACTERJOB>,std::allocator<std::pair<ENUM_CHARACTERJOB const,std::string>>> &)
int importSkillShopTree(const char* filename,
                        std::map<ENUM_CHARACTERJOB, std::string>&
                            skill_shop_tree) {
    if (!loadRDARScriptFile("", filename)) {
        LogManager::logFormat(0,
                              "../RDARScriptEtcParameter.cpp",
                              "importSkillShopTree",
                              0x32EB,
                              "ERROR : %s ",
                              filename);
        return 0;
    }

    std::string tag;
    std::string job_name;
    std::string tree_name;
    skill_shop_tree.clear();

    while (ScanType(tag, true) == 1) {
        if (tag == "[skill tree]") {
            while (ScanStr(&job_name) == 1) {
                const ENUM_CHARACTERJOB character_job =
                    static_cast<ENUM_CHARACTERJOB>(
                        ::getCharacterJob(job_name.c_str()));
                if (ScanStr(&tree_name) != 1) {
                    break;
                }
                skill_shop_tree.insert(
                    std::make_pair(character_job, tree_name));
            }
        } else if (tag == "[/skill tree]") {
            break;
        }
    }

    return 1;
}

// IDA_EA: 0x8914DD1
// IDA_NAME: _Z24importCancelSkillIndicesPKcRSt3mapISt4pairI17ENUM_CHARACTERJOBiESt3setIiSt4lessIiESaIiEES6_IS4_ESaIS2_IKS4_S9_EEE
// DEMANGLED: importCancelSkillIndices(char const*,std::map<std::pair<ENUM_CHARACTERJOB,int>,std::set<int,std::less<int>,std::allocator<int>>,std::less<std::pair<ENUM_CHARACTERJOB,int>>,std::allocator<std::pair<std::pair<ENUM_CHARACTERJOB,int> const,std::set<int,std::less<int>,std::allocator<int>>>>> &)
int importCancelSkillIndices(
    const char* filename,
    std::map<std::pair<ENUM_CHARACTERJOB, int>, std::set<int> >&
        cancel_skill_indices) {
    if (!loadRDARScriptFile("", filename)) {
        LogManager::logFormat(0,
                              "../RDARScriptEtcParameter.cpp",
                              "importCancelSkillIndices",
                              0x3372,
                              "ERROR : %s ",
                              filename);
        return 0;
    }

    std::string tag;
    std::set<int> skill_indices;
    ENUM_CHARACTERJOB character_job = static_cast<ENUM_CHARACTERJOB>(
        oldsrv::config::kCharacterJobCommon);
    int grow_type_index = -1;
    bool success = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[character job]") {
            std::string job_name;
            if (ScanStr(&job_name) != 1) {
                return 0;
            }

            character_job = static_cast<ENUM_CHARACTERJOB>(
                ::getCharacterJob(job_name.c_str()));

            std::string grow_type_name;
            if (ScanStr(&grow_type_name) != 1) {
                return 0;
            }

            grow_type_index = ::getGrowTypeIndex(grow_type_name.c_str());

            while (1) {
                const int skill_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                skill_indices.insert(skill_index);
            }

            cancel_skill_indices.insert(
                std::make_pair(std::make_pair(character_job, grow_type_index),
                               skill_indices));

            character_job = static_cast<ENUM_CHARACTERJOB>(
                oldsrv::config::kCharacterJobCommon);
            grow_type_index = -1;
            skill_indices.clear();
        } else {
            (void)(tag == "[cancel skill]" || tag == "[/cancel skill]" ||
                   tag == "[/character job]");
        }
    }

    return 1;
}
