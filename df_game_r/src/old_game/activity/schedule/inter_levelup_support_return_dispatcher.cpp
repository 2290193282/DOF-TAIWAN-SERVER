#include "old_game/activity/schedule/inter_levelup_support_return_dispatcher.h"

#include <cstring>
#include <stdint.h>

#include "old_data/db/event/db_save_event_levelup_support.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"

namespace {

uint16_t ReadU16(const char* data, int offset) {
    uint16_t value = 0;
    std::memcpy(&value, data + offset, sizeof(value));
    return value;
}

uint32_t ReadU32(const char* data, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, data + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84E8CE4
// IDA_NAME: _ZN29Inter_LoadEventLevelupSupport12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadEventLevelupSupport::dispatch_sig(CUser *,char *,int)
int Inter_LoadEventLevelupSupport::dispatch_sig(CUser* user,
                                                char* payload,
                                                int payload_size) {
    static const uint32_t kMaximumDatabaseRows = 10u;

    if (!user || !payload || payload_size < 4) {
        return 28143;
    }

    const uint32_t saved_count = ReadU32(payload, 0);
    if (saved_count > kMaximumDatabaseRows ||
        payload_size < static_cast<int>(4u + saved_count * 4u)) {
        return 0;
    }

    LevelupSupportEventManger* const manager =
        ARAD::Singleton<LevelupSupportEventManger>::Get();
    user->levelup_support_events_.clear();
    user->levelup_support_events_ = manager->GetDefaultEventInfo();

    const int charac_level = user->get_charac_level();
    for (std::vector<LevelupSupportEventManger::EventInfo>::iterator event =
             user->levelup_support_events_.begin();
         event != user->levelup_support_events_.end();
         ++event) {
        if (event->level <= charac_level) {
            event->state = 1;
        }
    }

    const uint32_t comparable_count = saved_count <
            static_cast<uint32_t>(user->levelup_support_events_.size())
        ? saved_count
        : static_cast<uint32_t>(user->levelup_support_events_.size());
    for (uint32_t index = 0; index < comparable_count; ++index) {
        LevelupSupportEventManger::EventInfo& event =
            user->levelup_support_events_[index];
        const uint16_t saved_level = ReadU16(payload, 4 + index * 4);
        if (event.level == saved_level) {
            event.state = ReadU16(payload, 6 + index * 4);
        }
    }

    // A freshly created character has no rows yet.  Persist every default
    // level/state pair exactly as the original receiver does before sending
    // the state packet.
    if (saved_count == 0) {
        for (std::vector<LevelupSupportEventManger::EventInfo>::const_iterator
                 event = user->levelup_support_events_.begin();
             event != user->levelup_support_events_.end();
             ++event) {
            DB_SaveEventLevelupSupport::makeRequest(
                user->GetUID(), user->getCurCharacNo(), event->level,
                event->state);
        }
    }

    LevelupSupportEventManger::SendCharacState(user, 0);
    return 0;
}

// IDA_EA: 0x84EEF10 / 0x84F572C / 0x84F575C
Inter_LoadEventLevelupSupport::Inter_LoadEventLevelupSupport()
    : ISigDispatcher() {}
Inter_LoadEventLevelupSupport::~Inter_LoadEventLevelupSupport() {}
