#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_CALL_PARTY_MEMBER_REALTIME_INFO_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_CALL_PARTY_MEMBER_REALTIME_INFO_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

// IDA unit: 09_dungeon_pvp_party/Dispatcher_CallPartyMemberRealtimeInfo.
class Dispatcher_CallPartyMemberRealtimeInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)> {
public:
    Dispatcher_CallPartyMemberRealtimeInfo();
    virtual ~Dispatcher_CallPartyMemberRealtimeInfo();

    int check_error(CUser* user, MSG_BASE& message);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CallPartyMemberRealtimeInfo) == 0x04u,
              "CMD169 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_CALL_PARTY_MEMBER_REALTIME_INFO_H_
