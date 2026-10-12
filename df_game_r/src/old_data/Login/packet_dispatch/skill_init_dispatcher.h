#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SKILL_INIT_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SKILL_INIT_DISPATCHER_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD492.  The full reset work is supplied by the already-restored SkillSlot
// lifecycle; this leaf owns its old state-0 dispatch specialization only.
class Dispatcher_SkillInit
    : public IPacketDispatcher<MSG_SKILL_INIT,
                               stSkillInit,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595890
    // IDA_NAME: _ZN20Dispatcher_SkillInitC2Ev
    // DEMANGLED: Dispatcher_SkillInit::Dispatcher_SkillInit(void)
    Dispatcher_SkillInit();

    // IDA_EA: 0x8243D3E / 0x8243D6E
    virtual ~Dispatcher_SkillInit();

    // IDA_EA: 0x81E5934 / 0x81E598A / 0x81E5AEC
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E5A2C / 0x81E5BDC
    int check_error(CUser* user, MSG_BASE& message);
    void process_skill_init(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SkillInit) == 0x04u,
              "CMD492 handler must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SKILL_INIT_DISPATCHER_H_
