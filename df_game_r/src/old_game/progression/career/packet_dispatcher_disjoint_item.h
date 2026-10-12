#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DISJOINT_ITEM_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DISJOINT_ITEM_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;
class CItem;
struct Inven_Item;

// IDA unit: 13_database_dispatch/DisPatcher_DisJointItem (CMD28).  Unlike a
// normal typed template this command owns dispatch_sig and is also the shared
// service called by CMD239's portable-machine request.
class DisPatcher_DisJointItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    DisPatcher_DisJointItem();
    virtual ~DisPatcher_DisJointItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    static int _disjoint(CUser* user,
                         int item_slot,
                         int item_space,
                         int command,
                         CUser* machine_owner,
                         uint16_t material_slot);

private:
    static int check_disjointable_item(const Inven_Item& item,
                                       CUser* machine_owner,
                                       CItem*& item_data);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_DisJointItem) == 0x04u,
              "CMD28 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_DISJOINT_ITEM_H_
