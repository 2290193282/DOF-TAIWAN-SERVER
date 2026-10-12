#ifndef OLDSRV_OLD_GAME_EVENT_SERIA_ROOM_ANI_DECO_EVENT_H_
#define OLDSRV_OLD_GAME_EVENT_SERIA_ROOM_ANI_DECO_EVENT_H_

#include "old_game/activity/reward/event_base.h"

class CUser;

// IDA unit: 09_dungeon_pvp_party/SeriaRoomAniDecoEvent.
// The old static event table constructs this base-only event in slot 105
// (0x811B639..0x811B661). It owns the daily decoration award consumed by
// CMD513 and the character ETC-load return path.
class SeriaRoomAniDecoEvent : public CEventBase {
public:
    // IDA_EA: 0x827B490
    // IDA_NAME: _ZN21SeriaRoomAniDecoEventC2Ev
    SeriaRoomAniDecoEvent();

    // IDA_EA: 0x827B4AC / 0x827B4DC
    virtual ~SeriaRoomAniDecoEvent();

    // IDA_EA: 0x827B4FA / 0x827B59E
    virtual void StartEvent();
    virtual void EndEvent();

    // IDA_EA: 0x827B54C
    virtual void StartEvent(Word_Param param);

    // IDA_EA: 0x827B5F0 / 0x827B6A6
    virtual void dailyresetData();
    virtual void StartAction();

    // IDA_EA: 0x827BC4C
    virtual int IsEventing(CUser* user) const;

    // IDA_EA: 0x827B75A
    int SendAniDecoItem(CUser& user);
    // IDA_EA: 0x827B99A
    void SendEventInfo(CUser& user);
    // IDA_EA: 0x827BA48
    int CheckEventInfo(CUser& user);
    // IDA_EA: 0x827BA52
    int processEvent(CUser& user);
};

#if defined(__i386__)
static_assert(sizeof(SeriaRoomAniDecoEvent) == 0x0cu,
              "SeriaRoomAniDecoEvent must retain the old base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_SERIA_ROOM_ANI_DECO_EVENT_H_
