#include "old_game/activity/schedule/hero_mission_runtime.h"

#include <boost/tokenizer.hpp>

#include <cstdlib>

// IDA_EA: 0x8164D68
// IDA_NAME: _ZN16HeroMissionValue14setStringTokenESs
// DEMANGLED: HeroMissionValue::setStringToken(std::string)
void HeroMissionValue::setStringToken(std::string token) {
    clear();

    // The original uses boost::char_separator(" ", 0, 0), then dereferences
    // five token iterators without validation.  Keep that exact space-only,
    // drop-empty grammar and its caller-owned malformed-input contract.
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
    const boost::char_separator<char> separator(" ");
    const Tokenizer tokens(token, separator);
    Tokenizer::const_iterator it = tokens.begin();

    mission_no = static_cast<HeroMissionCondition::MissionNo::T>(
        std::atoi(it->c_str()));
    ++it;
    progress = static_cast<uint16_t>(std::atoi(it->c_str()));
    ++it;
    state = static_cast<uint16_t>(std::atoi(it->c_str()));
    ++it;
    reward_count = static_cast<uint16_t>(std::atoi(it->c_str()));
    ++it;
    value = static_cast<uint16_t>(std::atoi(it->c_str()));
}
