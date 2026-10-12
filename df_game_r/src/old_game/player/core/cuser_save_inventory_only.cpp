#include "old_game/player/core/cuser.h"
#include "common/config.h"
#include "old_game/player/core/charac_expand_data_mgr.h"

#include "old_data/db/inventory/db_save_inven.h"
#include "old_data/db/inventory/db_item_growth_state.h"
#include "old_game/economy/inventory/inventory.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/world/region/game_world.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <stdint.h>

static_assert(oldsrv::config::kOrdinaryEquipmentTypeFirst +
                      oldsrv::config::kOrdinaryEquipmentTypeCount ==
                  oldsrv::config::kEquippedAvatarAndOrdinaryItemCount,
              "save-only frame must end with the ordinary equipment range");
static_assert(oldsrv::config::kEquippedAvatarAndOrdinaryItemCount <=
                  oldsrv::config::kEquipmentTransferItemCount,
              "save-only equipment prefix must fit the physical table");





// IDA_EA: 0x86501C8
// IDA_NAME: _ZN5CUser17SaveInventoryOnlyEv
// DEMANGLED: CUser::SaveInventoryOnly(void)
int CUser::SaveInventoryOnly() {
    if (this->isLocked4DataLoad() != 0) {
        const int charac_no = this->getCurCharacNo();
        const char* const account_id = NumberToString(
            static_cast<unsigned int>(this->get_acc_id()), 0);
        cMyTrace trace("bool CUser::SaveInventoryOnly()", 6266, 5);
        trace("Can not save character until loading done(m_id: %s, charac_no: %d",
              account_id,
              charac_no);
        return 0;
    }
    if (!IsCharacInfoLoaded(static_cast<CACHE_SECTION>(1))) {
        cMyTrace trace("bool CUser::SaveInventoryOnly()", 6267, 5);
        trace("Can not save character before inventory section completion");
        return 0;
    }
    if (G_GameWorld()->GetChannelType() == 7) {
        return 0;
    }

    CStreamGuard guard(
        GlobalData::big_stream_pool()->Acquire("user.cpp", 6280), true);
    *guard << 35;
    *guard << this->GetUID();
    SIG_SAVE_INVENTORY* const payload_record =
        guard.GetInBuffer<SIG_SAVE_INVENTORY>();
    std::memset(payload_record, 0, sizeof(*payload_record));
    _GetSaveInvenBase(payload_record);
    payload_record->save_mode = 2;

    const CInventory* const inventory =
        reinterpret_cast<const CInventory*>(
            this->getCurCharacInvenR());
    // The save frame owns slots 0..22, while GetInvenStart(0) starts at the
    // ordinary-equipment subrange (slot 11).  Copy from the physical table
    // origin so the DB compressor's [11, 23) slice is the real ordinary set
    // and never reads beyond CInventory::equipment_items_.
    std::memcpy(payload_record->equipment_items,
                inventory->equipment_items_,
                sizeof(payload_record->equipment_items));
    std::memcpy(payload_record->stackable_items,
                inventory->GetInvenStart(1),
                sizeof(payload_record->stackable_items));

    const CCargo* const cargo = getCurCharacCargoR();
    payload_record->cargo_capacity =
        static_cast<uint32_t>(cargo->GetCapacity());
    std::memcpy(payload_record->cargo_items,
                cargo->get_cargo_start(),
                sizeof(payload_record->cargo_items));

    std::memcpy(payload_record->creature_region.creature_items,
                inventory->GetInvenStart(3),
                sizeof(payload_record->creature_region.creature_items));
    std::memcpy(payload_record->creature_region
                    .creature_and_equipment_tail.equipment_tail,
                inventory->equipment_items_ +
                    oldsrv::config::kCreatureArtifactEquipmentTypeFirst,
                sizeof(payload_record->creature_region
                           .creature_and_equipment_tail.equipment_tail));
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
    return GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard) == 1 ? 1 : 0;
}
