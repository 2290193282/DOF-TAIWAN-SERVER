#include "old_game/player/core/cuser.h"

#include <map>
#include <utility>

#include "old_game/progression/quest/quest_skill_value_types.h"

// IDA_EA: 0x8654F74
// IDA_NAME: _ZN5CUser23customizingSkillCommandER18STSkillCommandData
// DEMANGLED: CUser::customizingSkillCommand(STSkillCommandData &)
int CUser::customizingSkillCommand(STSkillCommandData& command) {
    std::map<int, STSkillCommandData>* const commands = getSkillCommand();
    if (!commands) {
        return 0;
    }

    // The original normalizes client command code 8 to its persisted code 7
    // before either replacing or inserting the command record.
    for (int index = 0; index < static_cast<int>(command.byte_02); ++index) {
        if (command.raw_03[index] == 8u) {
            command.raw_03[index] = 7u;
        }
    }

    std::map<int, STSkillCommandData>::iterator found =
        commands->find(static_cast<int>(command.word_00));
    if (found != commands->end()) {
        found->second.word_00 = command.word_00;
        found->second.byte_02 = command.byte_02;
        for (int index = 0; index < static_cast<int>(command.byte_02);
             ++index) {
            found->second.raw_03[index] = command.raw_03[index];
        }
    } else {
        commands->insert(std::make_pair(static_cast<int>(command.word_00),
                                        command));
    }

    enableSaveSkill();
    return 1;
}
