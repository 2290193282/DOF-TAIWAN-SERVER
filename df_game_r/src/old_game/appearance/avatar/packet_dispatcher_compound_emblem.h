#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_COMPOUND_EMBLEM_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_COMPOUND_EMBLEM_H_

#include <stdint.h>

#include <utility>
#include <vector>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CItem;
class CUser;
class PacketBuf;
enum ENUM_ERROR : int;

// CMD260 owns a direct legacy transaction: `dispatch_sig` parses two emblem
// stackables and the base-template read/process entries are intentionally
// empty in the original binary.
class Dispatcher_CompoundEmblem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596D90
    Dispatcher_CompoundEmblem();
    // IDA_EA: 0x8240F3C / 0x8240F6C
    virtual ~Dispatcher_CompoundEmblem();

    // IDA_EA: 0x8220BB2
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x8233F6A / 0x8233F74
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x8220A60
    int _selectJewelTypeIndex(unsigned int count,
                              const uint16_t* types) const;
    // IDA_EA: 0x8221DDE
    char SendCreateEmblemInfo(
        const std::vector<std::pair<int, int> >& items);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_CompoundEmblem) == 0x04u,
              "CMD260 dispatcher must stay vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_COMPOUND_EMBLEM_H_
