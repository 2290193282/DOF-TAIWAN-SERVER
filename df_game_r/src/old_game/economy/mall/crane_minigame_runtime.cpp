#include "old_game/economy/mall/crane_minigame_runtime.h"

#include "infra/log/format/trace.h"
#include "old_data/PVF/npc/need_material_data_manager.h"
#include "old_game/activity/schedule/event_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x80CA15C
// IDA_NAME: _Z12get_rand_inti
// DEMANGLED: get_rand_int(int)
unsigned int get_rand_int(int limit);

// IDA_EA: 0x80EBC40
// IDA_NAME: _ZN20CraneMinigameManagerC2Ev
// DEMANGLED: CraneMinigameManager::CraneMinigameManager(void)
CraneMinigameManager::CraneMinigameManager()
    : CraneMinigameScript(), material_rotation_index_(0) {}

// IDA_EA: 0x80EBC5A
// IDA_NAME: _ZN20CraneMinigameManagerD2Ev
// DEMANGLED: CraneMinigameManager::~CraneMinigameManager()
CraneMinigameManager::~CraneMinigameManager() {}

// IDA_EA: 0x80EBC6E
// IDA_NAME: _ZN20CraneMinigameManager4initEv
// DEMANGLED: CraneMinigameManager::init(void)
int CraneMinigameManager::init() {
    send_mail.reset();
    if (importCraneMinigameScript(this, "Etc/craneMinigameItem.etc") == 1) {
        return 1;
    }
    cMyTrace trace("bool CraneMinigameManager::init()", 29, 0);
    trace("Error : craneMinigameItem.etc load failed!!");
    return 0;
}

// IDA_EA: 0x80EBCE0
// IDA_NAME: _ZN20CraneMinigameManager13isPickSuccessEsi
// DEMANGLED: CraneMinigameManager::isPickSuccess(short,int)
int CraneMinigameManager::isPickSuccess(int16_t item_index, int item_id) {
    CraneItemInfo* const item = getItem(item_index);
    if (!item || item->item_id != item_id) return 0;
    return item->pick_ratio >= static_cast<int>(get_rand_int(100));
}

// IDA_EA: 0x80EBD44
// IDA_NAME: _ZN20CraneMinigameManager10getItemCntEi
// DEMANGLED: CraneMinigameManager::getItemCnt(int)
int CraneMinigameManager::getItemCnt(int item_index) {
    CraneItemInfo* const item = getItem(static_cast<int16_t>(item_index));
    return item ? item->item_count : 0;
}

// IDA_EA: 0x80EBD7A
// IDA_NAME: _ZN20CraneMinigameManager8sendItemEP5CUseri
// DEMANGLED: CraneMinigameManager::sendItem(CUser *,int)
int CraneMinigameManager::sendItem(CUser* user, int item_index) {
    CraneItemInfo* const item = getItem(static_cast<int16_t>(item_index));
    if (!item || item->item_count == -1) return 21;

    send_mail.set_item(item->item_id, item->item_count);
    EventClassify::CEventActionMng action_manager;
    return action_manager.process_action_send_mail(user, send_mail, true);
}

// IDA_EA: 0x80EBE06
// IDA_NAME: _ZN20CraneMinigameManager16startUseMaterialEP5CUseri
// DEMANGLED: CraneMinigameManager::startUseMaterial(CUser *,int)
int CraneMinigameManager::startUseMaterial(CUser* user, int item_slot) {
    const Inven_Item material =
        user->getCurCharacInvenR()->GetInvenSlot(1, item_slot);
    if (material.getKey() != material_item_id) return 17;
    if (material.get_add_info() < material_count) return 22;

    if (user->getCurCharacInvenW()->delete_item(
            static_cast<INVEN_TYPE>(1), item_slot, material_count,
            static_cast<eItemDelReason>(52), true) == 1) {
        return 0;
    }
    return 22;
}

// IDA_EA: 0x80EBEBC
// IDA_NAME: _ZN20CraneMinigameManager27updateCraneItemNeedMaterialEi
// DEMANGLED: CraneMinigameManager::updateCraneItemNeedMaterial(int)
int CraneMinigameManager::updateCraneItemNeedMaterial(int rotation_seed) {
    material_rotation_index_ = static_cast<uint8_t>(
        static_cast<unsigned int>(rotation_seed) % need_materials.size());
    const NeedMaterialDataManager::NeedMaterialPair& material =
        need_materials.at(material_rotation_index_);
    return NeedMaterialDataManager::getInstance()->setNeedMaterialInfo(
        material_item_id, material);
}

// IDA_EA: 0x80EBF2E
// IDA_NAME: _ZN20CraneMinigameManager25SendCraneItemNeedMaterialEP5CUser
// DEMANGLED: CraneMinigameManager::SendCraneItemNeedMaterial(CUser *)
void CraneMinigameManager::SendCraneItemNeedMaterial(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, 423);
    packet.put_byte(material_rotation_index_);
    packet.finalize(true);
    user->Send(packet);
}
