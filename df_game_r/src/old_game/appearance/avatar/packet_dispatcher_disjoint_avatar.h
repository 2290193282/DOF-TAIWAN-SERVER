#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_DISJOINT_AVATAR_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_DISJOINT_AVATAR_H_

#include <stdint.h>

#include <utility>
#include <vector>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CItem;
class CUser;
class PacketBuf;
struct stAvatarEmblemInfo_t;

// CMD205 directly owns the legacy avatar-disjoint transaction.  The packet
// body is read by dispatch_sig; its base-template read/process members are
// genuinely empty in the original binary.
class Dispatcher_DisJointAvatar
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85968C0
    Dispatcher_DisJointAvatar();
    // IDA_EA: 0x8241860 / 0x8241890
    virtual ~Dispatcher_DisJointAvatar();

    // IDA_EA: 0x8217C06
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    // IDA_EA: 0x8233D26
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    // IDA_EA: 0x8233D30
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x821857C
    int _getJewelTypeIndex(uint16_t emblem_type) const;
    // IDA_EA: 0x8218598
    int _selectJewelTypeIndex(const stAvatarEmblemInfo_t* emblem) const;
    // IDA_EA: 0x821873C
    int _getResultItems(CUser* user,
                        const CItem* avatar_item,
                        int jewel_type_index,
                        std::vector<std::pair<int, int> >& items) const;
    // IDA_EA: 0x8218996
    int SendDisjointAvatarInfo(const CItem* avatar_item);
    // IDA_EA: 0x8218A36
    int SendCreateEmblemInfo(
        const std::vector<std::pair<int, int> >& items);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_DisJointAvatar) == 0x04u,
              "CMD205 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_PACKET_DISPATCHER_DISJOINT_AVATAR_H_
