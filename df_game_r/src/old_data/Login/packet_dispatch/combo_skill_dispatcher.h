#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_COMBO_SKILL_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_COMBO_SKILL_DISPATCHER_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD509.  The request only changes the combo-command map used by job 9;
// its skill-slot owner and the six configured combo indexes are recovered
// separately and remain the authority for this packet.
class Dispatcher_ComboSkill
    : public IPacketDispatcher<MSG_COMBO_SKILL,
                               stComboSkill,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597E52
    // IDA_NAME: _ZN21Dispatcher_ComboSkillC2Ev
    // DEMANGLED: Dispatcher_ComboSkill::Dispatcher_ComboSkill(void)
    Dispatcher_ComboSkill();

    // IDA_EA: 0x8260A40 / 0x8260A70
    virtual ~Dispatcher_ComboSkill();

    // IDA_EA: 0x825EF78 / 0x825F2FC / 0x825F444
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x825F292 / 0x825F3CE
    int checkComboSkillIndex(short skill_index) const;
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ComboSkill) == 0x04u,
              "CMD509 handler must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_COMBO_SKILL_DISPATCHER_H_
