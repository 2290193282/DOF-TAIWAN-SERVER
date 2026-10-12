#include "old_game/activity/schedule/hero_mission_runtime.h"

#include "common/config.h"
#include "old_game/player/core/cuser.h"

#include <boost/tokenizer.hpp>

// IDA_EA: 0x8166752
// IDA_NAME: _ZN16HeroMissionEvent12setEventDataEP5CUserPKc
// DEMANGLED: HeroMissionEvent::setEventData(CUser *,char const*)
bool HeroMissionEvent::setEventData(CUser* user, const char* token_data) {
    // 0x8166752 only installs state for a running event and a persisted
    // character.  It does not manufacture a record for charac_no 0.
    if (IsEventing(0) != 1 || user->get_charac_no(-1) == 0) {
        return false;
    }

    std::vector<HeroMissionValue> values;
    const std::string data(token_data);
    if (!data.empty()) {
        typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
        const boost::char_separator<char> separator(",");
        const Tokenizer tokens(data, separator);
        for (Tokenizer::const_iterator it = tokens.begin(); it != tokens.end();
             ++it) {
            HeroMissionValue value;
            value.setStringToken(*it);
            values.push_back(value);
        }
    } else {
        for (std::map<HeroMissionCondition::MissionNo::T,
                      BaseHeroMissionCondition*>::const_iterator it =
                 conditions_.begin();
             it != conditions_.end(); ++it) {
            HeroMissionValue value;
            value.clear();
            value.mission_no = it->first;

            // The maximum-level bootstrap writes all four original words for
            // mission 4 and only progress for mission 6.
            if (value.mission_no == HeroMissionCondition::MissionNo::LEVEL_UP &&
                user->get_charac_level() ==
                    oldsrv::config::kCharacterLevelMaximum) {
                value.progress = 1;
                value.state = 1;
                value.value = oldsrv::config::kCharacterLevelMaximum;
                value.reward_count = 1;
            }
            if (value.mission_no == HeroMissionCondition::MissionNo::TOTAL &&
                user->get_charac_level() ==
                    oldsrv::config::kCharacterLevelMaximum) {
                value.progress = 1;
            }
            values.push_back(value);
        }
    }

    const unsigned int charac_no = user->get_charac_no(-1);
    std::map<unsigned int, std::vector<HeroMissionValue> >::iterator existing =
        values_by_charac_.find(charac_no);
    if (existing != values_by_charac_.end()) {
        values_by_charac_.erase(existing);
    }
    values_by_charac_.insert(std::make_pair(charac_no, values));
    return true;
}

// IDA_EA: 0x8166D0E
// IDA_NAME: _ZN16HeroMissionEvent12setEventDataEP5CUserRKSs
// DEMANGLED: HeroMissionEvent::setEventData(CUser *,std::string const&)
bool HeroMissionEvent::setEventData(CUser* user, const std::string& token_data) {
    return setEventData(user, token_data.c_str());
}
