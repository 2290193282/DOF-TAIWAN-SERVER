#ifndef OLDSRV_OLD_GAME_PROGRESSION_QUEST_NPC_ITEM_DROP_RUNTIME_H_
#define OLDSRV_OLD_GAME_PROGRESSION_QUEST_NPC_ITEM_DROP_RUNTIME_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;
class QuestScript;
struct STMapScript;

// The 0828 client emits a body-less CMD588 from an action-script
// [NPC ITEM DROP] marker.  Quest and map PVF owners publish the two halves of
// that contract here without changing the old Quest/CMap object ABI.
namespace npc_item_drop {

void ClearQuestContracts();
void RegisterQuestContract(const QuestScript& script);
void ClearMapContracts();
void RegisterMapContract(const STMapScript& script);
void ResolveMapContracts();

}  // namespace npc_item_drop

class Dispatcher_EventNpcDropItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_EventNpcDropItem();
    virtual ~Dispatcher_EventNpcDropItem();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_EventNpcDropItem) == 0x04u,
              "CMD588 dispatcher must stay vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_PROGRESSION_QUEST_NPC_ITEM_DROP_RUNTIME_H_
