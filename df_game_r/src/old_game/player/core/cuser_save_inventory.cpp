#include "common/manager/global_data.h"
#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_data/db/inventory/db_save_inven.h"
#include "old_data/db/inventory/db_item_growth_state.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/charac_expand_data_mgr.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <stdint.h>

static_assert(oldsrv::config::kOrdinaryEquipmentTypeFirst +
                      oldsrv::config::kOrdinaryEquipmentTypeCount ==
                  oldsrv::config::kEquippedAvatarAndOrdinaryItemCount,
              "save frame must end with the complete ordinary equipment range");
static_assert(oldsrv::config::kEquippedAvatarAndOrdinaryItemCount <=
                  oldsrv::config::kEquipmentTransferItemCount,
              "save frame equipment prefix must fit the physical table");





// IDA_EA: 0x864FE52
// IDA_NAME: _ZN5CUser13SaveInventoryEv
// DEMANGLED: CUser::SaveInventory(void)
// Active reconstructed-tree ABI: _ZN5CUser13SaveInventoryEv.
int CUser::SaveInventory() {
    if (static_cast<unsigned char>(this->isLocked4DataLoad())) {
        const int charac_no = this->getCurCharacNo();
        const char* const account_id = NumberToString(
            static_cast<unsigned int>(this->get_acc_id()), 0);
        cMyTrace trace("bool CUser::SaveInventory()", 6170, 5);
        trace("Can not save character until loading done(m_id: %s, charac_no: %d",
              account_id,
              charac_no);
        return 0;
    }
    if (!IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1))) {
        cMyTrace trace("bool CUser::SaveInventory()", 6171, 5);
        trace("Can not save character before inventory section completion");
        return 0;
    }

    if (G_GameWorld()->GetChannelType() == 7) {
        return 0;
    }

    CStreamGuard guard(GlobalData::big_stream_pool()->Acquire("user.cpp", 6184),
                       true);
    *guard << 35;
    *guard << this->GetUID();

    SIG_SAVE_INVENTORY* const payload_record =
        guard.GetInBuffer<SIG_SAVE_INVENTORY>();
    std::memset(payload_record, 0, sizeof(*payload_record));
    _GetSaveInvenBase(payload_record);
    payload_record->save_mode = 0;

    const CInventory* const inventory =
        static_cast<const CInventory*>(this->getCurCharacInvenR());
    // Work on all 27 physical equipment slots first.  The KC/FP filters scan
    // that complete table; passing the 23-record save prefix to them makes
    // slots 23..26 alias stackable_items[0..3].
    Inven_Item equipment_snapshot[
        oldsrv::config::kEquipmentTransferItemCount];
    std::memcpy(equipment_snapshot,
                inventory->equipment_items_,
                sizeof(equipment_snapshot));
    std::memcpy(payload_record->stackable_items,
                inventory->GetInvenStart(1),
                sizeof(payload_record->stackable_items));

    CInventory::RemoveKCItem(equipment_snapshot,
                             payload_record->stackable_items);
    if (this->get_state() == 10) {
        CInventory::RemoveFPItem(equipment_snapshot,
                                 payload_record->stackable_items);
    }

    // SIG_SAVE_INVENTORY stores the complete avatar+ordinary equipment
    // prefix (slots 0..22).  GetInvenStart(0) deliberately returns the
    // ordinary-equipment subrange at slot 11; copying the 23-record payload
    // from there reads seven records past CInventory::equipment_items_ and
    // persists adjacent pointers as equipment data.
    std::memcpy(payload_record->equipment_items,
                equipment_snapshot,
                sizeof(payload_record->equipment_items));
    std::memcpy(payload_record->creature_region.creature_items,
                inventory->GetInvenStart(3),
                sizeof(payload_record->creature_region.creature_items));
    std::memcpy(payload_record->creature_region
                    .creature_and_equipment_tail.equipment_tail,
                equipment_snapshot +
                    oldsrv::config::kCreatureArtifactEquipmentTypeFirst,
                sizeof(payload_record->creature_region
                           .creature_and_equipment_tail.equipment_tail));

    const CCargo* const cargo = getCurCharacCargoR();
    payload_record->cargo_capacity =
        static_cast<uint32_t>(cargo->GetCapacity());
    std::memcpy(payload_record->cargo_items,
                cargo->get_cargo_start(),
                sizeof(payload_record->cargo_items));

    charac_expand_data_.update(this, static_cast<ENUM_CHARAC_EXPAND_TYPE>(9));
    this->removeRentalItemInfoFromInvenBuf(payload_record, 1);
    this->removeRentalItemInfoFromInvenBuf(payload_record, 2);
    item_growth_db::LegacySaveProofTrailer* const proof =
        reinterpret_cast<item_growth_db::LegacySaveProofTrailer*>(
            guard.GetInBufferUntyped(
                sizeof(item_growth_db::LegacySaveProofTrailer)));
    if (!proof) return 0;
    item_growth_db::InitLegacySaveProofTrailer(
        proof,
        this->GetServerGroup(),
        payload_record->account_id,
        payload_record->character_no,
        item_growth_db::kSagaProofInventory);
    // Callers that sequence a durable follow-up behind this snapshot need the
    // real DB_Q acceptance result.  Returning success unconditionally lets a
    // later request overtake a snapshot that was never queued.
    return GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard) == 1 ? 1 : 0;
}
