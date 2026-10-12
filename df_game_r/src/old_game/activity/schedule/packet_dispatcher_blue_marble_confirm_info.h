#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_BLUE_MARBLE_CONFIRM_INFO_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_BLUE_MARBLE_CONFIRM_INFO_H_

#include "old_game/activity/schedule/event_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD508.  This declaration deliberately has no registry side effect; the
// frozen root PacketDispatcher table stays unchanged until the full tree
// closes.
class Dispatcher_BlueMarbleConfirmInfo
    : public IPacketDispatcher<MSG_BLUE_MARBLE_CONFIRM_INFO,
                               stBlueMarbleConfirmInfo,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8597DE2
    // IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfoC2Ev
    // DEMANGLED: Dispatcher_BlueMarbleConfirmInfo::Dispatcher_BlueMarbleConfirmInfo(void)
    Dispatcher_BlueMarbleConfirmInfo();

    // IDA_EA: 0x8243C54 / 0x8243C84
    // IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfoD2Ev / D0Ev
    // DEMANGLED: Dispatcher_BlueMarbleConfirmInfo::~Dispatcher_BlueMarbleConfirmInfo()
    virtual ~Dispatcher_BlueMarbleConfirmInfo();

    // IDA_EA: 0x81E6242
    // IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfo4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81E62DA
    // IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfo7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E6904
    // IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfo4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E68C8
    // IDA_NAME: _ZN32Dispatcher_BlueMarbleConfirmInfo11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_BlueMarbleConfirmInfo) == 0x04u,
              "CMD508 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_BLUE_MARBLE_CONFIRM_INFO_H_
