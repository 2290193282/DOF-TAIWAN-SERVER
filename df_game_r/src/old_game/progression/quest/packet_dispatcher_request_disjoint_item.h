#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_DISJOINT_ITEM_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_DISJOINT_ITEM_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 08_quest_skill_growth/Dispatcher_RequestDisjointItem (CMD239).
class Dispatcher_RequestDisjointItem
    : public IPacketDispatcher<MSG_REQUEST_DISJOINT_ITEM,
                               stRequestDisjointItemParam,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_RequestDisjointItem();
    virtual ~Dispatcher_RequestDisjointItem();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

private:
    int check_error(CUser* user, const MSG_REQUEST_DISJOINT_ITEM& message) const;
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestDisjointItem) == 0x04u,
              "CMD239 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_DISJOINT_ITEM_H_
