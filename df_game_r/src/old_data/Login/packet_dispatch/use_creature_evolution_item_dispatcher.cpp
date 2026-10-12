#include "old_data/Login/packet_dispatch/use_creature_evolution_item_dispatcher.h"

#include "common/manager/data_manager.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/history_trace.h"
#include "infra/log/format/trace.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace user_creature {

// IDA_EA: 0x8338F36
// IDA_NAME: _ZN13user_creature12CCreatureMgr16GetCreatureIndexEv
// DEMANGLED: user_creature::CCreatureMgr::GetCreatureIndex(void)
int CCreatureMgr::GetCreatureIndex() {
    // The original 0x16-byte leaf is exactly
    // CCreature::GetCreatureId(this->active_creature_).  Its caller, CMD324,
    // already follows the old precondition that a creature is equipped.
    return active_creature_->GetCreatureId();
}

}  // namespace user_creature

// IDA_EA: 0x8597736
// IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItemC2Ev
// DEMANGLED: Dispatcher_UseCreatureEvolutionItem::Dispatcher_UseCreatureEvolutionItem(void)
Dispatcher_UseCreatureEvolutionItem::Dispatcher_UseCreatureEvolutionItem() {}

// IDA_EA: 0x82400EA
// IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItemD2Ev
// DEMANGLED: Dispatcher_UseCreatureEvolutionItem::~Dispatcher_UseCreatureEvolutionItem()
// IDA_EA: 0x824011A
// IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItemD0Ev
// DEMANGLED: Dispatcher_UseCreatureEvolutionItem::~Dispatcher_UseCreatureEvolutionItem()
Dispatcher_UseCreatureEvolutionItem::~Dispatcher_UseCreatureEvolutionItem() {}

// IDA_EA: 0x8228A76
// IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UseCreatureEvolutionItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UseCreatureEvolutionItem::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    if (user->get_state() != 3) {
        return 0;
    }

    uint8_t item_space = 0;
    int16_t item_slot = 0;
    int member_count = 0;
    if (packet.get_byte(item_space) != 1) {
        return LineFunc(
            58889,
            "virtual int Dispatcher_UseCreatureEvolutionItem::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_short(item_slot) != 1) {
        return LineFunc(
            58890,
            "virtual int Dispatcher_UseCreatureEvolutionItem::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_int(member_count) != 1) {
        return LineFunc(
            58891,
            "virtual int Dispatcher_UseCreatureEvolutionItem::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    const int result = process(user, item_space, item_slot, member_count);
    if (result != 0) {
        // CMD320 is one of the native 140828 commands whose error payload is
        // a u16.  The generic helper writes a u8 and would leave the client
        // handler one byte short.
        PacketGuard response;
        response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                            ENUM_CMDPACKET_USE_CREATURE_EVOLUTION_ITEM);
        response.put_byte(0);
        response.put_short(static_cast<uint16_t>(result));
        response.finalize(true);
        user->Send(response);
    }
    return 0;
}

// IDA_EA: 0x8228BCC
// IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItem7processEP5CUserhsi
// DEMANGLED: Dispatcher_UseCreatureEvolutionItem::process(CUser *,unsigned char,short,int)
int Dispatcher_UseCreatureEvolutionItem::process(CUser* user,
                                                  uint8_t item_space,
                                                  int16_t item_slot,
                                                  int member_count) {
    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(item_space));
    const Inven_Item* const inventory_item =
        user->getCurCharacInvenR()->GetInvenRef(inventory_type, item_slot);
    if (!inventory_item) {
        return 4;
    }

    CItem* const item_prototype =
        G_CDataManager()->find_item(inventory_item->item_id_);
    if (!item_prototype) {
        cMyTrace trace(
            "int Dispatcher_UseCreatureEvolutionItem::process(CUser*, unsigned char, short int, int)",
            58913,
            5);
        trace("Dispatcher_UseCreatureEvolutionItem not found item_id(%d), charac_no(%d)\n",
              inventory_item->item_id_, user->getCurCharacNo());
        return 21;
    }

    if (item_prototype->is_stackable() != 1) {
        cMyTrace trace(
            "int Dispatcher_UseCreatureEvolutionItem::process(CUser*, unsigned char, short int, int)",
            58919,
            5);
        trace("Dispatcher_UseCreatureEvolutionItem item_id(%d) is not stackable charac_no(%d)\n",
              inventory_item->item_id_, user->getCurCharacNo());
        return 23;
    }

    const CStackableItem* const stackable_item =
        static_cast<const CStackableItem*>(item_prototype);
    user_creature::CCreatureMgr* const creature_manager =
        user->getCurCharacInvenW()->GetCreatureMgrW();
    if (creature_manager->GetCreatureIndex() !=
        stackable_item->evolution_creature_id()) {
        cMyTrace trace(
            "int Dispatcher_UseCreatureEvolutionItem::process(CUser*, unsigned char, short int, int)",
            58927,
            5);
        trace("Dispatcher_UseCreatureEvolutionItem cur_creatureid(%d) != item_base_creature_id(%d) charac_no(%d)\n",
              creature_manager->GetCreatureIndex(),
              stackable_item->evolution_creature_id(),
              user->getCurCharacNo());
        return 7;
    }

    user_creature::CCreatureMgr* const write_creature_manager =
        static_cast<CInventory*>(user->getCurCharacInvenRefW())
            ->GetCreatureMgrW();
    cHistoryTrace history_trace = {};
    if (write_creature_manager->ItemEvolute(member_count)) {
        history_trace("ItemEvolute Succ evolution_creature_id(%d), charac_no(%d)",
                      member_count,
                      user->getCurCharacNo());
        return 0;
    }

    history_trace("ItemEvolute Fail evolution_creature_id(%d), charac_no(%d)",
                  member_count,
                  user->getCurCharacNo());
    return 1;
}

// IDA_EA: 0x8234302
// IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UseCreatureEvolutionItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UseCreatureEvolutionItem::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823430C
// IDA_NAME: _ZN35Dispatcher_UseCreatureEvolutionItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UseCreatureEvolutionItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UseCreatureEvolutionItem::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// The handler is deliberately unregistered until the root 606-slot
// PacketDispatcher construction/destruction transaction is fully restored.
