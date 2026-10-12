#include "old_game/progression/quest/quest_skill_standalone_value_types.h"

// IDA_EA: 0x89173F0
// IDA_NAME: _ZN14QuestMovieInfoC2Ev
// DEMANGLED: QuestMovieInfo::QuestMovieInfo(void)
QuestMovieInfo::QuestMovieInfo()
    : title(),
      file_path() {
    clear();
}

// IDA_EA: 0x8921FBC
// IDA_NAME: _ZN14QuestMovieInfoC2ERKS_
// DEMANGLED: QuestMovieInfo::QuestMovieInfo(QuestMovieInfo const&)
QuestMovieInfo::QuestMovieInfo(const QuestMovieInfo& other)
    : title(other.title),
      file_path(other.file_path) {}

// IDA_EA: 0x891CBD0
// IDA_NAME: _ZN14QuestMovieInfoD2Ev
// DEMANGLED: QuestMovieInfo::~QuestMovieInfo()
QuestMovieInfo::~QuestMovieInfo() {}

// IDA_EA: 0x8917458
// IDA_NAME: _ZN14QuestMovieInfoaSERS_
// DEMANGLED: QuestMovieInfo::operator=(QuestMovieInfo&)
QuestMovieInfo& QuestMovieInfo::operator=(QuestMovieInfo& other) {
    title = other.title;
    file_path = other.file_path;
    return *this;
}

// IDA_EA: 0x891748E
// IDA_NAME: _ZN14QuestMovieInfoeqERKS_
// DEMANGLED: QuestMovieInfo::operator==(QuestMovieInfo const&)
bool QuestMovieInfo::operator==(const QuestMovieInfo& other) {
    return title == other.title && file_path == other.file_path;
}

// IDA_EA: 0x89174D4
// IDA_NAME: _ZN14QuestMovieInfo5clearEv
// DEMANGLED: QuestMovieInfo::clear(void)
void QuestMovieInfo::clear() {
    title.clear();
    file_path.clear();
}

// IDA_EA: 0x89C3612
// IDA_NAME: _ZN17CNRepeatQuestListC2Ev
// DEMANGLED: CNRepeatQuestList::CNRepeatQuestList(void)
CNRepeatQuestList::CNRepeatQuestList()
    : quest_indices() {
    clear();
}

// IDA_EA: 0x89DC4CE
// IDA_NAME: _ZN17CNRepeatQuestListD2Ev
// DEMANGLED: CNRepeatQuestList::~CNRepeatQuestList()
CNRepeatQuestList::~CNRepeatQuestList() {}

// IDA_EA: 0x89C35FE
// IDA_NAME: _ZN17CNRepeatQuestList5clearEv
// DEMANGLED: CNRepeatQuestList::clear(void)
void CNRepeatQuestList::clear() {
    quest_indices.clear();
}

// IDA_EA: 0x84868E4
// IDA_NAME: _ZN5SkillC2Ev
// DEMANGLED: Skill::Skill(void)
Skill::Skill()
    : IData() {
    Reset();
}

// IDA_EA: 0x8486950
// IDA_NAME: _ZN5SkillD2Ev
// DEMANGLED: Skill::~Skill()
Skill::~Skill() {}

// IDA_EA: 0x84868B8
// IDA_NAME: _ZN5Skill5ResetEv
// DEMANGLED: Skill::Reset(void)
Skill* Skill::Reset() {
    value_00 = 0;
    value_04 = 0;
    value_08 = 0;
    merge_count_0c = 0;
    return this;
}

// IDA_EA: 0x8486902
// IDA_NAME: _ZN5SkillpLERKS_
// DEMANGLED: Skill::operator+=(Skill const&)
Skill& Skill::operator+=(const Skill& other) {
    value_00 += other.value_00;
    value_04 += other.value_04;
    value_08 += other.value_08;
    ++merge_count_0c;
    return *this;
}
