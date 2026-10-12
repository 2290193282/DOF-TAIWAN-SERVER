#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_STRIKER_DISPATCHER_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_STRIKER_DISPATCHER_H_

#include "old_data/Login/packet_dispatch/select_striker_types.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD488.  This handler either selects a fully loaded tag character or sends
// SIG_REQUEST_TAG_CHARAC_INFO to the DB queue; no PacketDispatcher root entry
// is registered until the global 606-slot lifecycle closes.
class Dispatcher_SelectStriker
    : public IPacketDispatcher<MSG_SELECT_STRIKER,
                               stSelectStriker,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8595900
    // IDA_NAME: _ZN24Dispatcher_SelectStrikerC2Ev
    // DEMANGLED: Dispatcher_SelectStriker::Dispatcher_SelectStriker(void)
    Dispatcher_SelectStriker();

    // IDA_EA: 0x8243EC4 / 0x8243EF4
    virtual ~Dispatcher_SelectStriker();

    // IDA_EA: 0x81E3D32 / 0x81E3DCA / 0x81E42EE
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E4066
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_SelectStriker) == 0x04u,
              "CMD488 handler must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_SELECT_STRIKER_DISPATCHER_H_
