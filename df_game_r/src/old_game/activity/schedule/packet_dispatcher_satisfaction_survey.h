#ifndef OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_SATISFACTION_SURVEY_H_
#define OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_SATISFACTION_SURVEY_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

class Dispatcher_SatisfactionSurvey
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_SatisfactionSurvey();
    virtual ~Dispatcher_SatisfactionSurvey();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user,
                        MSG_BASE& message,
                        ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SatisfactionSurvey) == 0x04u,
              "CMD563 dispatcher must remain vptr-only on target i386");
#endif

#endif  // OLDSRV_OLD_GAME_ACTIVITY_SCHEDULE_PACKET_DISPATCHER_SATISFACTION_SURVEY_H_
