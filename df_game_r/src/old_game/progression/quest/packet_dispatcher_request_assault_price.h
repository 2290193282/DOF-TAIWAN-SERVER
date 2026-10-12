#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_ASSAULT_PRICE_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_ASSAULT_PRICE_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 08_quest_skill_growth/Dispatcher_RequestAssaultPrice (CMD447).
class Dispatcher_RequestAssaultPrice
    : public IPacketDispatcher<MsgRequestAssaultPrice,
                               stRequestAssaultPrice,
                               static_cast<ch_state>(3)> {
public:
    Dispatcher_RequestAssaultPrice();
    virtual ~Dispatcher_RequestAssaultPrice();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

private:
    int check_error(CUser* user, const MsgRequestAssaultPrice& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestAssaultPrice) == 0x04u,
              "CMD447 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_ASSAULT_PRICE_H_
