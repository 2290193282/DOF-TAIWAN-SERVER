#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SET_PARTY_INFO_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SET_PARTY_INFO_H_

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/social/party/party_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

class DisPatcher_SetPartyInfo
    : public IPacketDispatcher<MSG_PARTY_SET_INFO,
                               stSetPartyInfo,
                               static_cast<ch_state>(3)> {
public:
    DisPatcher_SetPartyInfo();
    virtual ~DisPatcher_SetPartyInfo();

    int check_error(CUser* user, MSG_BASE& message);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
};

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_PACKET_DISPATCHER_SET_PARTY_INFO_H_
