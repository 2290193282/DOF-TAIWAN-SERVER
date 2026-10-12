#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_BUY_SKILL_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_BUY_SKILL_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 08_quest_skill_growth/DisPatcher_BuySkill (CMD31).
class DisPatcher_BuySkill
    : public IPacketDispatcher<MSG_BUY_SKILL,
                               stOnBuySkillParam,
                               static_cast<ch_state>(3)> {
public:
    DisPatcher_BuySkill();
    virtual ~DisPatcher_BuySkill();

    virtual int check_state(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

private:
    int check_error(CUser* user, const MSG_BUY_SKILL& message) const;
    int master_new_skill_process(CUser* user,
                                 unsigned char skill_tree,
                                 buySkillInfo& info) const;
    int refund_skill_process(CUser* user,
                             unsigned char skill_tree,
                             buySkillInfo& info) const;
    int try_master_new_skills_process(CUser* user,
                                      unsigned char skill_tree,
                                      buySkillInfo& info) const;
    int try_refund_skills_process(CUser* user,
                                  unsigned char skill_tree,
                                  buySkillInfo& info) const;
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_BuySkill) == 0x04u,
              "CMD31 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_BUY_SKILL_H_
