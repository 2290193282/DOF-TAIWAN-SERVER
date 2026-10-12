#ifndef OLDSRV_OLD_DATA_PVF_ETC_QUEST_MOVIE_INFO_H_
#define OLDSRV_OLD_DATA_PVF_ETC_QUEST_MOVIE_INFO_H_

#include <string>

// IDA unit: 08_quest_skill_growth/QuestMovieInfo.
// IDA refs: 0x89173F0, 0x8917458, 0x891748E, 0x89174D4, 0x891CBD0,
//           0x8921FBC.
//
// The PVF importer and the quest runtime share this value.  Its two strings
// are proven by importQuestMovieList to be [title] followed by [file path].
// Keep one canonical declaration; the original method bodies remain owned by
// quest_skill_standalone_value_types.cpp, matching their recovered IDA unit.
struct QuestMovieInfo {
    QuestMovieInfo();
    QuestMovieInfo(const QuestMovieInfo& other);
    ~QuestMovieInfo();

    QuestMovieInfo& operator=(QuestMovieInfo& other);
    bool operator==(const QuestMovieInfo& other);
    void clear();

    std::string title;      // +0x00
    std::string file_path;  // +0x04 in the old i386 libstdc++ ABI
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(std::string) == 0x04u,
              "old-server i386 libstdc++ string ABI must stay four bytes");
static_assert(sizeof(QuestMovieInfo) == 0x08u,
              "QuestMovieInfo must retain the 0627 i386 value ABI");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_ETC_QUEST_MOVIE_INFO_H_
