#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/player/core/cuser.h"
#include "common/manager/os_api.h"

#include <cstdlib>
#include <cstdio>
#include <map>
#include <string>

namespace WongWork {

typedef CDungeonClear::Base DungeonClearMap;

// IDA_EA: 0x8483FAE
// IDA_NAME: _ZN8WongWork13CDungeonClear14AradfromStringEP5CUserPKc
// DEMANGLED: WongWork::CDungeonClear::AradfromString(CUser *, char const*)
int CDungeonClear::AradfromString(CUser* user, const char* text) {
    std::string records(text);
    std::string::size_type begin = 0;
    while (begin < records.size()) {
        const std::string::size_type end = records.find(',', begin);
        const std::string record = records.substr(
            begin,
            end == std::string::npos ? std::string::npos : end - begin);
        if (!record.empty()) {
            const std::string::size_type separator = record.find('|');
            const int dungeon_index = std::atoi(record.c_str());
            const int parsed_difficulty = std::atoi(
                record.c_str() + separator + 1u);
            int difficulty = parsed_difficulty == 4 ? 3 : parsed_difficulty;

            CDungeon* const dungeon =
                G_CDataManager()->find_dungeon(dungeon_index);
            if (dungeon) {
                const unsigned int required_quest = static_cast<unsigned int>(
                    dungeon->getHeroesRequiredQuest());
                if (required_quest &&
                    user->getCurCharacQuestR()->isClearQuest(
                        static_cast<int>(required_quest))) {
                    difficulty = 3;
                }
            }

            const short stored_index = static_cast<short>(dungeon_index);
            DungeonClearMap::iterator found = DungeonClearMap::find(stored_index);
            if (found == DungeonClearMap::end()) {
                DungeonClearMap::insert(std::make_pair(
                    stored_index, static_cast<char>(difficulty)));
            } else if (found->second < difficulty) {
                found->second = static_cast<char>(difficulty);
            }
        }

        if (end == std::string::npos) {
            break;
        }
        begin = end + 1u;
    }
    return 1;
}

// IDA_EA: 0x84843FA
// IDA_NAME: _ZN8WongWork13CDungeonClear10fromStringEP5CUserPc
// DEMANGLED: WongWork::CDungeonClear::fromString(CUser *,char *)
int CDungeonClear::fromString(CUser* user, char* text) {
    return AradfromString(user, text);
}

// IDA_EA: 0x8695972
// IDA_NAME: _ZN8WongWork13CDungeonClear5clearEv
// DEMANGLED: WongWork::CDungeonClear::clear(void)
void CDungeonClear::clear() {
    DungeonClearMap::clear();
}

// IDA_EA: 0x848441C
// IDA_NAME: _ZNK8WongWork13CDungeonClear8toStringEPcj
// DEMANGLED: WongWork::CDungeonClear::toString(char *,unsigned int)const
int CDungeonClear::toString(char* out, unsigned int maxlen) const {
    const DungeonClearMap& records =
        static_cast<const DungeonClearMap&>(*this);
    if (records.empty()) {
        return 0;
    }

    DungeonClearMap::const_iterator it = records.begin();
    int written = std::snprintf(out, maxlen, "%d|%d",
                                static_cast<int>(it->first),
                                static_cast<int>(it->second));
    ++it;

    for (; it != records.end(); ++it) {
        const int added = std::snprintf(out + written, maxlen - written,
                                        ",%d|%d",
                                        static_cast<int>(it->first),
                                        static_cast<int>(it->second));
        if (added < 0) {
            return -1;
        }
        written += added;
    }
    return written;
}

// IDA_EA: 0x8484558
// IDA_NAME: _ZNK8WongWork13CDungeonClear8toStringERSsi
// DEMANGLED: WongWork::CDungeonClear::toString(std::string &,int)const
int CDungeonClear::toString(std::string& out, int maxlen) const {
    const DungeonClearMap& records =
        static_cast<const DungeonClearMap&>(*this);
    if (records.empty()) {
        return 0;
    }

    const unsigned int initial_maxlen = static_cast<unsigned int>(maxlen);
    unsigned int current_maxlen = initial_maxlen;
    out.resize(initial_maxlen);

    DungeonClearMap::const_iterator record = records.begin();
    int written = OS_API::snprintf(
        const_cast<char*>(out.c_str()),
        current_maxlen,
        "%d|%d",
        static_cast<int>(record->first),
        static_cast<int>(record->second));
    ++record;

    for (; record != records.end(); ++record) {
        int appended = 0;
        do {
            const unsigned int remaining =
                current_maxlen - static_cast<unsigned int>(written);
            appended = OS_API::snprintf(
                const_cast<char*>(out.c_str()) + written,
                remaining,
                ",%d|%d",
                static_cast<int>(record->first),
                static_cast<int>(record->second));
            if (appended < 0) {
                out.resize(current_maxlen + initial_maxlen);
                current_maxlen += initial_maxlen;
            }
        } while (appended < 0);

        written += appended;
    }
    return written;
}

}  // namespace WongWork
