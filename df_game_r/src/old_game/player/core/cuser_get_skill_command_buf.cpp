#include "old_game/progression/quest/quest_skill_value_types.h"
#include "old_game/player/core/cuser.h"

#include <map>
#include <stdint.h>


typedef std::map<int, STSkillCommandData> SkillCommandMap;

// IDA_EA: 0x865459A
// IDA_NAME: _ZN5CUser21get_skill_command_bufEv
// DEMANGLED: CUser::get_skill_command_buf(void)
// Active reconstructed-tree ABI: _ZN5CUser21get_skill_command_bufEv.
void* CUser::get_skill_command_buf() {
    SkillCommandMap* const commands = this->getSkillCommand();
    if (!commands) {
        return 0;
    }

    int slot = 0;
    for (SkillCommandMap::iterator it = commands->begin();
         it != commands->end();
         ++it) {
        STSkillCommandData::STCommand command;
        uint8_t* const packed = command.raw;
        const STSkillCommandData& value = it->second;

        *reinterpret_cast<uint16_t*>(packed) = static_cast<uint16_t>(
            (value.word_00 & 0x3fffu) |
            (*reinterpret_cast<uint16_t*>(packed) & 0xc000u));
        packed[3] = static_cast<uint8_t>(
            ((value.byte_02 & 7u) << 5) | (packed[3] & 0x1fu));
        *reinterpret_cast<uint32_t*>(packed) =
            ((static_cast<uint32_t>(value.raw_03[0] & 7u)) << 14) |
            (*reinterpret_cast<uint32_t*>(packed) & 0xfffe3fffu);
        packed[2] = static_cast<uint8_t>(
            ((value.raw_03[1] & 7u) << 1) | (packed[2] & 0xf1u));
        packed[2] = static_cast<uint8_t>(
            ((value.raw_03[2] & 7u) << 4) | (packed[2] & 0x8fu));
        *reinterpret_cast<uint16_t*>(packed + 2) = static_cast<uint16_t>(
            ((value.raw_03[3] & 7u) << 7) |
            (*reinterpret_cast<uint16_t*>(packed + 2) & 0xfc7fu));
        packed[3] = static_cast<uint8_t>(
            ((value.raw_03[4] & 7u) << 2) | (packed[3] & 0xe3u));

        skill_commands_[slot] = command;
        if (++slot > 49) {
            break;
        }
    }
    return skill_commands_;
}
