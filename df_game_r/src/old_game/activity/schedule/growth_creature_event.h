#ifndef OLDSRV_OLD_GAME_EVENT_GROWTH_CREATURE_EVENT_H_
#define OLDSRV_OLD_GAME_EVENT_GROWTH_CREATURE_EVENT_H_

#include <stdint.h>
#include <vector>

#include "old_game/activity/reward/event_base.h"
#include "old_game/player/core/creature_guild_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

struct GrowthCreatureEvent {
    struct eventCharacInfo {
        // IDA_EA: 0x81B7458
        // IDA_NAME: _ZN19GrowthCreatureEvent15eventCharacInfoC2Ev
        // DEMANGLED: GrowthCreatureEvent::eventCharacInfo::eventCharacInfo(void)
        eventCharacInfo();

        // IDA_EA: 0x81B7E16
        // IDA_NAME: _ZN19GrowthCreatureEvent15eventCharacInfoD2Ev
        // DEMANGLED: GrowthCreatureEvent::eventCharacInfo::~eventCharacInfo()
        ~eventCharacInfo();

        uint16_t job;
        uint32_t charac_no;
        uint32_t creature_id;
        uint8_t creature_name[0x1e];
    };

    // IDA_EA: 0x86970A4
    // IDA_NAME: _ZN19GrowthCreatureEventC2Ev
    // DEMANGLED: GrowthCreatureEvent::GrowthCreatureEvent(void)
    GrowthCreatureEvent();

    // IDA_EA: 0x8697C16
    // IDA_NAME: _ZN19GrowthCreatureEventD2Ev
    // DEMANGLED: GrowthCreatureEvent::~GrowthCreatureEvent()
    ~GrowthCreatureEvent();

    std::vector<eventCharacInfo> event_characters;
};

class CUser;

// IDA unit: 06_userinfo_stat_inventory/GiveGrowCreatureEvent.  This is the
// base-only event registered as event 103 by the old event table; its state is
// retained in CUser::growth_creature_event_, not in this event object.
class GiveGrowCreatureEvent : public CEventBase {
public:
    // IDA_EA: 0x81B695C
    // IDA_NAME: _ZN21GiveGrowCreatureEventC2Ev
    GiveGrowCreatureEvent();

    // IDA_EA: 0x81B6978 / 0x81B69A8
    // IDA_NAME: _ZN21GiveGrowCreatureEventD2Ev / _ZN21GiveGrowCreatureEventD0Ev
    virtual ~GiveGrowCreatureEvent();

    // IDA_EA: 0x81B69C6 / 0x81B6A6A
    virtual void StartEvent();
    virtual void EndEvent();

    // IDA_EA: 0x81B6A18
    virtual void StartEvent(Word_Param param);

    // IDA_EA: 0x81B6ABC
    virtual void StartAction();

    // IDA_EA: 0x81B739E
    virtual int IsEventing(CUser* user) const;

    // IDA_EA: 0x81B6B70 / 0x81B6E7A / 0x81B6F76 / 0x81B7156
    // IDA_NAME: _ZN21GiveGrowCreatureEvent27SendGrowthCreatureEventInfoER5CUser
    //           _ZN21GiveGrowCreatureEvent14CheckEventInfoER5CUser
    //           _ZN21GiveGrowCreatureEvent12processEventER5CUser
    //           _ZN21GiveGrowCreatureEvent22isGrowCreatureMaxLevelER5CUser
    void SendGrowthCreatureEventInfo(CUser& user);
    int CheckEventInfo(CUser& user);
    int processEvent(CUser& user);
    int isGrowCreatureMaxLevel(CUser& user);

    // IDA_EA: 0x81B717A
    // IDA_NAME: _ZN21GiveGrowCreatureEvent16changeCharacNameEP5CUseriPci
    virtual void changeCharacName(CUser* user,
                                  int charac_no,
                                  char* name,
                                  int length);
};

// IDA unit:
// 11_event_runtime/Dispatcher_GrowthCreatureChangeInfinityCreature (CMD512).
class Dispatcher_GrowthCreatureChangeInfinityCreature
    : public IPacketDispatcher<MSG_BASE,
                               stGrowthCreature,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597EC2
    Dispatcher_GrowthCreatureChangeInfinityCreature();

    // IDA_EA: 0x8243A80 / 0x8243AB0
    virtual ~Dispatcher_GrowthCreatureChangeInfinityCreature();

    // IDA_EA: 0x81E85A2 / 0x81E85AC / 0x81E8768
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E863A / 0x81E86A6 / 0x81E86FC
    int process_event(CUser* user);
    int check_error(CUser* user, MSG_BASE& message);
    int check_error_event(CUser* user);
};

static_assert(sizeof(GrowthCreatureEvent::eventCharacInfo) == 0x2cu,
              "GrowthCreatureEvent::eventCharacInfo i386 layout drift");
static_assert(sizeof(GrowthCreatureEvent) == 0x0cu,
              "GrowthCreatureEvent i386 layout drift");

#if defined(__i386__)
static_assert(sizeof(GiveGrowCreatureEvent) == 0x0cu,
              "GiveGrowCreatureEvent must retain its base-only i386 ABI");
static_assert(sizeof(Dispatcher_GrowthCreatureChangeInfinityCreature) == 0x04u,
              "CMD512 handler must retain the old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_GROWTH_CREATURE_EVENT_H_
