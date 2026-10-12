#include "old_game/pvp/battlefield/battle_field.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/data_manager.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

#include <stdarg.h>

// IDA_EA: 0x8678166
// IDA_NAME: _Z18fnEnumDungeonClearscPc
// DEMANGLED: fnEnumDungeonClear(short,char,char *)
bool fnEnumDungeonClear(short dungeon_index,
                        char difficulty,
                        char* callback_arguments) {
    PacketGuard* const packet =
        *reinterpret_cast<PacketGuard**>(callback_arguments);
    packet->put_short(dungeon_index);
    packet->put_byte(difficulty);
    return true;
}

namespace WongWork {

// IDA_EA: 0x84846E2
// IDA_NAME: _ZNK8WongWork13CDungeonClear16enumDungeonClearEP5CUserPFbscPcEz
// DEMANGLED: WongWork::CDungeonClear::enumDungeonClear(CUser *,bool (*)(short,char,char *),...)const
char CDungeonClear::enumDungeonClear(CUser* user,
                                     bool (*callback)(short, char, char*),
                                     ...) const {
    va_list arguments;
    va_start(arguments, callback);

    // The original forwards the unconsumed x86 va_list itself on every
    // callback, rather than taking the PacketGuard pointer out of it.  The
    // serializer callback therefore dereferences this stack argument slot.
#if defined(__i386__)
    char* const callback_arguments = arguments;
#else
    char* const callback_arguments = reinterpret_cast<char*>(&arguments);
#endif

    for (Base::const_iterator it = Base::begin(); it != Base::end(); ++it) {
        const short dungeon_index = it->first;
        char difficulty = it->second;

        CDungeon* const dungeon =
            G_CDataManager()->find_dungeon(static_cast<int>(dungeon_index));
        if (dungeon && dungeon->getHeroesRequiredQuest() != 0 &&
            user->getCurCharacQuestR()->clear_quest.isClearedQuest(
                static_cast<unsigned int>(dungeon->getHeroesRequiredQuest()))) {
            difficulty = 4;
        }

        if (!callback(dungeon_index, difficulty, callback_arguments)) {
            va_end(arguments);
            return 1;
        }
    }

    va_end(arguments);
    return 0;
}

// IDA_EA: 0x869595E
// IDA_NAME: _ZNK8WongWork13CDungeonClear19getDungeonClearSizeEv
// DEMANGLED: WongWork::CDungeonClear::getDungeonClearSize(void)const
int CDungeonClear::getDungeonClearSize() const {
    return static_cast<int>(Base::size());
}

}  // namespace WongWork
