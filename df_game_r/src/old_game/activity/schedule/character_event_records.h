#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_EVENT_RECORDS_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_EVENT_RECORDS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8697886 in the companion source.


#include <stdint.h>

#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/schedule/growth_creature_event.h"


// IDA unit: 04_character_select_create/CCharacterDayEvent.
class CCharacterDayEvent : public CEventBase {
public:
    CCharacterDayEvent();
    virtual ~CCharacterDayEvent();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int GetEventJob();
};

// IDA unit: 04_character_select_create/EventNewCharacterReward.
// The original object is a CEventBase subclass: the base owns the event flag
// and the two 16-bit wire parameters, while this record owns the decoded
// character job and experience ratio at +0x0c/+0x10.
class EventNewCharacterReward : public CEventBase {
public:
    EventNewCharacterReward();
    virtual ~EventNewCharacterReward();

    virtual void StartEvent();
    virtual void EndEvent();
    virtual void StartEvent(Word_Param param);

    int isEventCharacter(int charac_job);
    long double eventExpRatio();
    int eventCharacterJob();

private:
    int event_character_job;
    int exp_ratio;
};

namespace online_preliminary {

// IDA unit: 04_character_select_create/online_preliminary/CharacterInServer.
struct CharacterInServer {
    CharacterInServer(int server_group_value, uint32_t charac_no_value);

    bool operator<(const CharacterInServer& rhs) const;

    uint16_t server_group;
    uint16_t pad2;
    uint32_t charac_no;
};

}  // namespace online_preliminary

// IDA unit:
// 04_character_select_create/SIG_GIVE_GROWTH_CREATURE_EVENT/eventCharacInfo.
struct SIG_GIVE_GROWTH_CREATURE_EVENT {
    enum { kLoginRecordCount = 11 };

    struct eventCharacInfo {
        eventCharacInfo();

        uint16_t job;
        uint16_t pad2;
        uint32_t charac_no;
        uint32_t creature_id;
        uint8_t creature_name[0x1e];
    } entries[kLoginRecordCount];
};

static_assert(sizeof(SIG_GIVE_GROWTH_CREATURE_EVENT::eventCharacInfo) ==
                  0x2cu,
              "growth-creature login record must retain its old 44-byte ABI");
static_assert(sizeof(SIG_GIVE_GROWTH_CREATURE_EVENT) == 0x1e4u,
              "growth-creature login block must retain eleven IDA records");


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_EVENT_RECORDS_H_
