#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REFUND_SKILL_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REFUND_SKILL_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;

// IDA unit: 08_quest_skill_growth/Dispatcher_RefundSkill (CMD427).
class Dispatcher_RefundSkill
    : public IPacketDispatcher<MSG_REFUND_SKILL,
                               stRefundSkill,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_RefundSkill();
    virtual ~Dispatcher_RefundSkill();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

private:
    int check_error(CUser* user, const MSG_REFUND_SKILL& message) const;
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RefundSkill) == 0x04u,
              "CMD427 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REFUND_SKILL_H_
