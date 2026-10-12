#ifndef OLDSRV_DATAMANAGER_GLOBAL_SCRIPT_TUTORIAL_H_
#define OLDSRV_DATAMANAGER_GLOBAL_SCRIPT_TUTORIAL_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "common/config.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/equipment/upgrade_table.h"

// IDA unit: 99_misc_global/TutorialData.
class TutorialData {
public:
    struct DialogData {
        DialogData();
        std::string dialog;
        int face_index;
    };

    struct SkillData {
        int skill_index;
        int skill_level;
        int hot_key;
    };

    struct TutorialNotice {
        TutorialNotice();
        int notice_index;
        std::string animation_path;
        int begin_time;
        int end_time;
    };

    struct ItemData {
        int item_index;
        int item_count;
    };

    TutorialData();
    ~TutorialData();

    TutorialData* setDungeonIndex(unsigned int dungeon_index);
    int getDungeonIndex();
    int addDialogData(int episode, int face_index, const char* dialog);
    int getDialogSize(unsigned int episode);
    bool isCurrentEpisode(unsigned int dialog_index, unsigned int episode);
    const char* getDialogStr(unsigned int dialog_index, unsigned int episode);
    int getFaceIndex(unsigned int dialog_index, unsigned int episode);
    int addAvatarIndex(int equipment_type, int avatar_index);
    int addEquipmentIndex(int equipment_type, int equipment_index, int job);
    int getAvatarIndex(ENUM_EQUIPMENTTYPE equipment_type);
    int getEquipmentIndex(ENUM_EQUIPMENTTYPE equipment_type, int job);
    int getConvertHotKey(int hot_key);
    void addSkillIndex(int job, int hot_key, int skill_index, int skill_level,
                       int skill_extra);
    void addPassiveSkillData(int job, int skill_index, int skill_level);
    std::vector<SkillData*> getPassiveSkillData(int job);
    SkillData* getSkillData(int hot_key, int job);
    int addTutorialNotice(int episode, int begin_time, int end_time,
                          const char* animation_path);
    std::vector<TutorialNotice*> getNoticeAniPath(int episode);
    int addItem(int item_index, int item_count);
    std::vector<ItemData*> getItem();

private:
    unsigned int dungeon_index_;
    std::map<int, std::vector<DialogData*> > dialog_data_;
    std::map<int, int> avatar_indexes_;
    std::map<int, int> equipment_indexes_[4];
    std::map<int, SkillData*> skill_data_[4];
    std::vector<SkillData*> passive_skill_data_[4];
    std::map<int, std::vector<TutorialNotice*> > tutorial_notices_;
    std::vector<ItemData*> items_;
};

static_assert(sizeof(TutorialData::DialogData) == 0x08u,
              "TutorialData::DialogData IDA ABI size must stay 0x08");
static_assert(sizeof(TutorialData::SkillData) == 0x0cu,
              "TutorialData::SkillData IDA ABI size must stay 0x0c");
static_assert(sizeof(TutorialData::TutorialNotice) == 0x10u,
              "TutorialData::TutorialNotice IDA ABI size must stay 0x10");
static_assert(sizeof(TutorialData::ItemData) == 0x08u,
              "TutorialData::ItemData IDA ABI size must stay 0x08");
static_assert(sizeof(TutorialData) == 0x148u,
              "TutorialData IDA ABI size must stay 0x148");

// IDA unit: 12_pvf_datamanager_scripts/GlobalScriptTutorial.
class GlobalScriptTutorial {
public:
    GlobalScriptTutorial();
    ~GlobalScriptTutorial();

    int loadScript();
    int loadScript(const char* directory, const char* filename);
    void LoadDialogTag(ENUM_CHARACTERJOB& character_job);
    int LoadAvatarTag(ENUM_CHARACTERJOB& character_job);
    int LoadEquipmentTag(ENUM_CHARACTERJOB& character_job);
    int LoadSkillTag(ENUM_CHARACTERJOB& character_job);
    int LoadPassiveSkilltag(ENUM_CHARACTERJOB& character_job);
    int LoadDungeonIndex(ENUM_CHARACTERJOB& character_job);
    void LoadTutorialNotice(ENUM_CHARACTERJOB& character_job);
    int LoadItem(ENUM_CHARACTERJOB& character_job);
    void clearScript();
    TutorialData* getData(int character_job);
    static bool IsGlobalTutorialDungeon(unsigned int dungeon_index);

private:
    TutorialData data_[oldsrv::config::kCharacterJobCount];
};

static_assert(sizeof(GlobalScriptTutorial) ==
                  sizeof(TutorialData) * oldsrv::config::kCharacterJobCount,
              "GlobalScriptTutorial must follow the configured job count");

#endif  // OLDSRV_DATAMANAGER_GLOBAL_SCRIPT_TUTORIAL_H_
