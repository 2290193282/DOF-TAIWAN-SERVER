#ifndef OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_SET_PLAY_RESULT_H_
#define OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_SET_PLAY_RESULT_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;
class CParty;
struct GameResultSet;

// IDA unit: 13_database_dispatch/DisPatcher_SetPlayResult (CMD049).
// This direct owner consumes a variable-length state-5 report in dispatch_sig;
// the template message/result slots are intentionally MSG_BASE/ParamBase.
class DisPatcher_SetPlayResult
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595580
    DisPatcher_SetPlayResult();
    // IDA_EA: 0x823303E / 0x823306E
    virtual ~DisPatcher_SetPlayResult();

    // IDA_EA: 0x81F4CE6
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x81F5DB6
    static int SendResult(CParty* party, GameResultSet& result);
    // IDA_EA: 0x823302A / 0x8233034
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SetPlayResult) == 0x04u,
              "CMD049 dispatcher must retain the base-only i386 ABI");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_SET_PLAY_RESULT_H_
