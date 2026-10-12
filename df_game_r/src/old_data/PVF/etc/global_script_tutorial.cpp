#include "old_data/PVF/etc/global_script_tutorial.h"

#include "common/config.h"
#include "old_data/PVF/common/rdar_script.h"
#include "common/manager/global_tools.h"

// IDA_EA: 0x8AB981E
// IDA_NAME: _ZN12TutorialData10DialogDataC2Ev
// DEMANGLED: TutorialData::DialogData::DialogData(void)
TutorialData::DialogData::DialogData() : dialog() {}

// IDA_EA: 0x8AB985E
// IDA_NAME: _ZN12TutorialData14TutorialNoticeC2Ev
// DEMANGLED: TutorialData::TutorialNotice::TutorialNotice(void)
TutorialData::TutorialNotice::TutorialNotice() : animation_path() {}

// IDA_EA: 0x8AB7E4C
// IDA_NAME: _ZN12TutorialDataC2Ev
// DEMANGLED: TutorialData::TutorialData(void)
TutorialData::TutorialData()
    : dungeon_index_(0),
      dialog_data_(),
      avatar_indexes_(),
      equipment_indexes_(),
      skill_data_(),
      passive_skill_data_(),
      tutorial_notices_(),
      items_() {
    avatar_indexes_.clear();
}

// IDA_EA: 0x8AB80EE
// IDA_NAME: _ZN12TutorialDataD2Ev
// DEMANGLED: TutorialData::~TutorialData()
TutorialData::~TutorialData() {}

// IDA_EA: 0x8AB82C4
// IDA_NAME: _ZN12TutorialData15setDungeonIndexEj
// DEMANGLED: TutorialData::setDungeonIndex(unsigned int)
TutorialData* TutorialData::setDungeonIndex(unsigned int dungeon_index) {
    dungeon_index_ = dungeon_index;
    return this;
}

// IDA_EA: 0x8AB82D2
// IDA_NAME: _ZN12TutorialData15getDungeonIndexEv
// DEMANGLED: TutorialData::getDungeonIndex(void)
int TutorialData::getDungeonIndex() {
    return dungeon_index_;
}

// IDA_EA: 0x8AB82DC
// IDA_NAME: _ZN12TutorialData13addDialogDataEiiPKc
// DEMANGLED: TutorialData::addDialogData(int, int, char const*)
int TutorialData::addDialogData(int episode, int face_index, const char* dialog) {
    DialogData* value = new DialogData();
    value->face_index = face_index;
    value->dialog = dialog;
    dialog_data_[episode].push_back(value);
    return 0;
}

// IDA_EA: 0x8AB8486
// IDA_NAME: _ZN12TutorialData13getDialogSizeEj
// DEMANGLED: TutorialData::getDialogSize(unsigned int)
int TutorialData::getDialogSize(unsigned int episode) {
    std::map<int, std::vector<DialogData*> >::iterator it =
        dialog_data_.find(episode);
    return it == dialog_data_.end() ? 0 : it->second.size();
}

// IDA_EA: 0x8AB84FE
// IDA_NAME: _ZN12TutorialData16isCurrentEpisodeEjj
// DEMANGLED: TutorialData::isCurrentEpisode(unsigned int, unsigned int)
bool TutorialData::isCurrentEpisode(unsigned int dialog_index,
                                    unsigned int episode) {
    return getDialogSize(episode) > dialog_index;
}

// IDA_EA: 0x8AB852E
// IDA_NAME: _ZN12TutorialData12getDialogStrEjj
// DEMANGLED: TutorialData::getDialogStr(unsigned int, unsigned int)
const char* TutorialData::getDialogStr(unsigned int dialog_index,
                                       unsigned int episode) {
    std::map<int, std::vector<DialogData*> >::iterator it =
        dialog_data_.find(episode);
    if (it == dialog_data_.end() || it->second.size() <= dialog_index) {
        return 0;
    }
    return it->second.at(dialog_index)->dialog.c_str();
}

// IDA_EA: 0x8AB85DE
// IDA_NAME: _ZN12TutorialData12getFaceIndexEjj
// DEMANGLED: TutorialData::getFaceIndex(unsigned int, unsigned int)
int TutorialData::getFaceIndex(unsigned int dialog_index, unsigned int episode) {
    std::map<int, std::vector<DialogData*> >::iterator it =
        dialog_data_.find(episode);
    if (it == dialog_data_.end() || it->second.size() <= dialog_index) {
        return 0;
    }
    return it->second.at(dialog_index)->face_index;
}

// IDA_EA: 0x8AB868A
// IDA_NAME: _ZN12TutorialData14addAvatarIndexEii
// DEMANGLED: TutorialData::addAvatarIndex(int, int)
int TutorialData::addAvatarIndex(int equipment_type, int avatar_index) {
    avatar_indexes_.insert(std::make_pair(equipment_type, avatar_index));
    return 0;
}

// IDA_EA: 0x8AB86E0
// IDA_NAME: _ZN12TutorialData17addEquipmentIndexEiii
// DEMANGLED: TutorialData::addEquipmentIndex(int, int, int)
int TutorialData::addEquipmentIndex(int equipment_type, int equipment_index,
                                    int job) {
    if (job <= 3) {
        equipment_indexes_[job].insert(
            std::make_pair(equipment_type, equipment_index));
    }
    return 0;
}

// IDA_EA: 0x8AB874E
// IDA_NAME: _ZN12TutorialData14getAvatarIndexE18ENUM_EQUIPMENTTYPE
// DEMANGLED: TutorialData::getAvatarIndex(ENUM_EQUIPMENTTYPE)
int TutorialData::getAvatarIndex(ENUM_EQUIPMENTTYPE equipment_type) {
    std::map<int, int>::iterator it = avatar_indexes_.find(equipment_type);
    return it == avatar_indexes_.end() ? -1 : it->second;
}

// IDA_EA: 0x8AB87BE
// IDA_NAME: _ZN12TutorialData17getEquipmentIndexE18ENUM_EQUIPMENTTYPEi
// DEMANGLED: TutorialData::getEquipmentIndex(ENUM_EQUIPMENTTYPE, int)
int TutorialData::getEquipmentIndex(ENUM_EQUIPMENTTYPE equipment_type,
                                    int job) {
    if (job > 3) {
        return -1;
    }
    std::map<int, int>::iterator it = equipment_indexes_[job].find(equipment_type);
    return it == equipment_indexes_[job].end() ? -1 : it->second;
}

// IDA_EA: 0x8AB885C
// IDA_NAME: _ZN12TutorialData16getConvertHotKeyEi
// DEMANGLED: TutorialData::getConvertHotKey(int)
int TutorialData::getConvertHotKey(int hot_key) {
    if (static_cast<unsigned int>(hot_key) > 12u) {
        return -1;
    }
    return hot_key > 5 ? hot_key + 192 : hot_key;
}

// IDA_EA: 0x8AB8888
// IDA_NAME: _ZN12TutorialData13addSkillIndexEiiiii
// DEMANGLED: TutorialData::addSkillIndex(int, int, int, int, int)
void TutorialData::addSkillIndex(int job, int hot_key, int skill_index,
                                 int skill_level, int skill_extra) {
    if (job <= 3) {
        SkillData* value = new SkillData;
        value->skill_index = skill_index;
        value->skill_level = skill_level;
        value->hot_key = skill_extra;
        const int key = getConvertHotKey(hot_key);
        if (key != -1) {
            skill_data_[job].insert(std::make_pair(key, value));
        }
    }
}

// IDA_EA: 0x8AB8944
// IDA_NAME: _ZN12TutorialData19addPassiveSkillDataEiii
// DEMANGLED: TutorialData::addPassiveSkillData(int, int, int)
void TutorialData::addPassiveSkillData(int job, int skill_index,
                                       int skill_level) {
    if (job <= 3) {
        SkillData* value = new SkillData;
        value->skill_index = skill_index;
        value->skill_level = skill_level;
        value->hot_key = -1;
        passive_skill_data_[job].push_back(value);
    }
}

// IDA_EA: 0x8AB89A6
// IDA_NAME: _ZN12TutorialData19getPassiveSkillDataEi
// DEMANGLED: TutorialData::getPassiveSkillData(int)
std::vector<TutorialData::SkillData*> TutorialData::getPassiveSkillData(int job) {
    return job < 4 ? passive_skill_data_[job] : std::vector<SkillData*>();
}

// IDA_EA: 0x8AB89FA
// IDA_NAME: _ZN12TutorialData12getSkillDataEii
// DEMANGLED: TutorialData::getSkillData(int, int)
TutorialData::SkillData* TutorialData::getSkillData(int hot_key, int job) {
    std::map<int, SkillData*>::iterator it = skill_data_[job].find(hot_key);
    return it == skill_data_[job].end() ? 0 : it->second;
}

// IDA_EA: 0x8AB8A86
// IDA_NAME: _ZN12TutorialData17addTutorialNoticeEiiiPKc
// DEMANGLED: TutorialData::addTutorialNotice(int, int, int, char const*)
int TutorialData::addTutorialNotice(int episode, int begin_time, int end_time,
                                    const char* animation_path) {
    TutorialNotice* value = new TutorialNotice();
    value->notice_index = episode;
    value->animation_path = animation_path;
    value->begin_time = begin_time;
    value->end_time = end_time;
    tutorial_notices_[episode].push_back(value);
    return 0;
}

// IDA_EA: 0x8AB8C4C
// IDA_NAME: _ZN12TutorialData16getNoticeAniPathEi
// DEMANGLED: TutorialData::getNoticeAniPath(int)
std::vector<TutorialData::TutorialNotice*> TutorialData::getNoticeAniPath(
    int episode) {
    std::map<int, std::vector<TutorialNotice*> >::iterator it =
        tutorial_notices_.find(episode);
    return it == tutorial_notices_.end() ? std::vector<TutorialNotice*>()
                                         : it->second;
}

// IDA_EA: 0x8AB8D1C
// IDA_NAME: _ZN12TutorialData7addItemEii
// DEMANGLED: TutorialData::addItem(int, int)
int TutorialData::addItem(int item_index, int item_count) {
    ItemData* value = new ItemData;
    value->item_index = item_index;
    value->item_count = item_count;
    items_.push_back(value);
    return 0;
}

// IDA_EA: 0x8AB8D5C
// IDA_NAME: _ZN12TutorialData7getItemEv
// DEMANGLED: TutorialData::getItem(void)
std::vector<TutorialData::ItemData*> TutorialData::getItem() {
    return items_;
}

// IDA_EA: 0x8AB8D8A
// IDA_NAME: _ZN20GlobalScriptTutorialC2Ev
// DEMANGLED: GlobalScriptTutorial::GlobalScriptTutorial(void)
GlobalScriptTutorial::GlobalScriptTutorial() : data_() {
    clearScript();
}

// IDA_EA: 0x8AB8E0A
// IDA_NAME: _ZN20GlobalScriptTutorialD2Ev
// DEMANGLED: GlobalScriptTutorial::~GlobalScriptTutorial()
GlobalScriptTutorial::~GlobalScriptTutorial() {
    clearScript();
}

// IDA_EA: 0x8AB8E4A
// IDA_NAME: _ZN20GlobalScriptTutorial10loadScriptEv
// DEMANGLED: GlobalScriptTutorial::loadScript(void)
int GlobalScriptTutorial::loadScript() {
    if (!loadRDARScriptFile("Etc/GlobalTutorialInfo/", "CharacterList.lst")) {
        return 0;
    }
    std::vector<std::string> file_list;
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[list]") {
            std::string filename;
            while (ScanStr(&filename) == 1) {
                file_list.push_back(filename);
            }
        }
    }
    unloadRDARScriptFile();

    for (std::vector<std::string>::const_iterator it = file_list.begin();
         it != file_list.end(); ++it) {
        if (loadScript("Etc/GlobalTutorialInfo/", it->c_str()) != 1) {
            return 0;
        }
    }
    return 1;
}

// IDA_EA: 0x8AB8FFE
// IDA_NAME: _ZN20GlobalScriptTutorial10loadScriptEPKcS1_
// DEMANGLED: GlobalScriptTutorial::loadScript(char const*, char const*)
int GlobalScriptTutorial::loadScript(const char* directory, const char* filename) {
    if (!loadRDARScriptFile(directory, filename)) {
        return 0;
    }
    ENUM_CHARACTERJOB character_job = static_cast<ENUM_CHARACTERJOB>(
        oldsrv::config::kCharacterJobInvalid);
    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[dialog]") LoadDialogTag(character_job);
        else if (tag == "[avatar]") LoadAvatarTag(character_job);
        else if (tag == "[equipment]") LoadEquipmentTag(character_job);
        else if (tag == "[dungeonindex]") LoadDungeonIndex(character_job);
        else if (tag == "[skill]") LoadSkillTag(character_job);
        else if (tag == "[passive skill]") LoadPassiveSkilltag(character_job);
        else if (tag == "[tutorial notice]") LoadTutorialNotice(character_job);
        else if (tag == "[item]") LoadItem(character_job);
        else character_job = static_cast<ENUM_CHARACTERJOB>(
            ::getCharacterJob(tag.c_str()));
    }
    unloadRDARScriptFile();
    return 1;
}

// IDA_EA: 0x8AB9226
// IDA_NAME: _ZN20GlobalScriptTutorial13LoadDialogTagER17ENUM_CHARACTERJOB
// DEMANGLED: GlobalScriptTutorial::LoadDialogTag(ENUM_CHARACTERJOB &)
void GlobalScriptTutorial::LoadDialogTag(ENUM_CHARACTERJOB& job) {
    if (job != oldsrv::config::kCharacterJobInvalid) {
        bool ok = false;
        while (1) {
            const int episode = ScanInt(&ok); if (!ok) break;
            const int face = ScanInt(&ok); if (!ok) break;
            std::string dialog; if (ScanStr(&dialog) != 1) break;
            data_[job].addDialogData(episode, face, dialog.c_str());
        }
    }
}

// IDA_EA: 0x8AB930A
// IDA_NAME: _ZN20GlobalScriptTutorial13LoadAvatarTagER17ENUM_CHARACTERJOB
// DEMANGLED: GlobalScriptTutorial::LoadAvatarTag(ENUM_CHARACTERJOB &)
int GlobalScriptTutorial::LoadAvatarTag(ENUM_CHARACTERJOB& job) {
    if (job != oldsrv::config::kCharacterJobInvalid) { bool ok = false; while (1) { int a = ScanInt(&ok); if (!ok) break; int b = ScanInt(&ok); if (!ok) break; data_[job].addAvatarIndex(a, b); } }
    return 0;
}

// IDA_EA: 0x8AB937A
// IDA_NAME: _ZN20GlobalScriptTutorial16LoadEquipmentTagER17ENUM_CHARACTERJOB
// DEMANGLED: GlobalScriptTutorial::LoadEquipmentTag(ENUM_CHARACTERJOB &)
int GlobalScriptTutorial::LoadEquipmentTag(ENUM_CHARACTERJOB& job) {
    if (job != oldsrv::config::kCharacterJobInvalid) { bool ok = false; while (1) { int a = ScanInt(&ok); if (!ok) break; int b = ScanInt(&ok); if (!ok) break; int c = ScanInt(&ok); if (!ok) break; data_[job].addEquipmentIndex(b, c, a); } }
    return 0;
}

// IDA_EA: 0x8AB9408
// IDA_NAME: _ZN20GlobalScriptTutorial12LoadSkillTagER17ENUM_CHARACTERJOB
// DEMANGLED: GlobalScriptTutorial::LoadSkillTag(ENUM_CHARACTERJOB &)
int GlobalScriptTutorial::LoadSkillTag(ENUM_CHARACTERJOB& job) {
    if (job != oldsrv::config::kCharacterJobInvalid) { bool ok = false; while (1) { int a = ScanInt(&ok); if (!ok) break; int b = ScanInt(&ok); if (!ok) break; int c = ScanInt(&ok); if (!ok) break; int d = ScanInt(&ok); if (!ok) break; int e = ScanInt(&ok); if (!ok) break; data_[job].addSkillIndex(a, b, c, d, e); } }
    return 0;
}

// IDA_EA: 0x8AB94D4
// IDA_NAME: _ZN20GlobalScriptTutorial19LoadPassiveSkilltagER17ENUM_CHARACTERJOB
// DEMANGLED: GlobalScriptTutorial::LoadPassiveSkilltag(ENUM_CHARACTERJOB &)
int GlobalScriptTutorial::LoadPassiveSkilltag(ENUM_CHARACTERJOB& job) {
    if (job != oldsrv::config::kCharacterJobInvalid) { bool ok = false; while (1) { int a = ScanInt(&ok); if (!ok) break; int b = ScanInt(&ok); if (!ok) break; int c = ScanInt(&ok); if (!ok) break; data_[job].addPassiveSkillData(a, b, c); } }
    return 0;
}

// IDA_EA: 0x8AB9562
// IDA_NAME: _ZN20GlobalScriptTutorial16LoadDungeonIndexER17ENUM_CHARACTERJOB
// DEMANGLED: GlobalScriptTutorial::LoadDungeonIndex(ENUM_CHARACTERJOB &)
int GlobalScriptTutorial::LoadDungeonIndex(ENUM_CHARACTERJOB& job) {
    if (job != oldsrv::config::kCharacterJobInvalid) { bool ok = false; while (1) { int index = ScanInt(&ok); if (!ok) break; data_[job].setDungeonIndex(index); } }
    return 0;
}

// IDA_EA: 0x8AB95AC
// IDA_NAME: _ZN20GlobalScriptTutorial18LoadTutorialNoticeER17ENUM_CHARACTERJOB
// DEMANGLED: GlobalScriptTutorial::LoadTutorialNotice(ENUM_CHARACTERJOB &)
void GlobalScriptTutorial::LoadTutorialNotice(ENUM_CHARACTERJOB& job) {
    if (job != oldsrv::config::kCharacterJobInvalid) { bool ok = false; while (1) { int a = ScanInt(&ok); if (!ok) break; int b = ScanInt(&ok); if (!ok) break; int c = ScanInt(&ok); if (!ok) break; std::string path; if (ScanStr(&path) != 1) break; data_[job].addTutorialNotice(a, b, c, path.c_str()); } }
}

// IDA_EA: 0x8AB96A8
// IDA_NAME: _ZN20GlobalScriptTutorial8LoadItemER17ENUM_CHARACTERJOB
// DEMANGLED: GlobalScriptTutorial::LoadItem(ENUM_CHARACTERJOB &)
int GlobalScriptTutorial::LoadItem(ENUM_CHARACTERJOB& job) {
    if (job != oldsrv::config::kCharacterJobInvalid) { bool ok = false; while (1) { int a = ScanInt(&ok); if (!ok) break; int b = ScanInt(&ok); if (!ok) break; data_[job].addItem(a, b); } }
    return 0;
}

// IDA_EA: 0x8AB9718
// IDA_NAME: _ZN20GlobalScriptTutorial11clearScriptEv
// DEMANGLED: GlobalScriptTutorial::clearScript(void)
void GlobalScriptTutorial::clearScript() {}

// IDA_EA: 0x8AB971E
// IDA_NAME: _ZN20GlobalScriptTutorial7getDataEi
// DEMANGLED: GlobalScriptTutorial::getData(int)
TutorialData* GlobalScriptTutorial::getData(int character_job) {
    if (character_job < 0 ||
        character_job >= oldsrv::config::kCharacterJobCount ||
        !data_[character_job].getDungeonIndex()) return 0;
    return &data_[character_job];
}

// IDA_EA: 0x8AB976A
// IDA_NAME: _ZN20GlobalScriptTutorial23IsGlobalTutorialDungeonEj
// DEMANGLED: GlobalScriptTutorial::IsGlobalTutorialDungeon(unsigned int)
bool GlobalScriptTutorial::IsGlobalTutorialDungeon(unsigned int dungeon_index) {
    return dungeon_index == 3506u;
}
