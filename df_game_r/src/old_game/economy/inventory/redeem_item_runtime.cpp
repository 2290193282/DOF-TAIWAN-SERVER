#include "old_game/economy/inventory/redeem_item_runtime.h"

#include <algorithm>
#include <cstring>
#include <iterator>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/equipment/item_upgrade.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


namespace {

WongWork::CItemUpgrade* GetItemUpgrade(CDataManager* data_manager) {
    return data_manager->get_item_upgrade();
}

}  // namespace

namespace Redeem_Item {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
Inven_Item& GetInvenItem(Inven_Item_Expand_RedeemInfo& info) {
    return *reinterpret_cast<Inven_Item*>(info.item_storage);
}

const Inven_Item& GetInvenItem(const Inven_Item_Expand_RedeemInfo& info) {
    return *reinterpret_cast<const Inven_Item*>(info.item_storage);
}
#pragma GCC diagnostic pop

// IDA_EA: 0x85F7040
// IDA_NAME: _ZN11Redeem_Item11CRedeemItemC2Ev
// DEMANGLED: Redeem_Item::CRedeemItem::CRedeemItem(void)
CRedeemItem::CRedeemItem()
    : charac_expand::CData(), redeem_items_() {}

// IDA_EA: 0x85F708E
// IDA_NAME: _ZN11Redeem_Item11CRedeemItemD2Ev
// DEMANGLED: Redeem_Item::CRedeemItem::~CRedeemItem()
// IDA_EA: 0x85F70F0
// IDA_NAME: _ZN11Redeem_Item11CRedeemItemD0Ev
// DEMANGLED: Redeem_Item::CRedeemItem::~CRedeemItem()
CRedeemItem::~CRedeemItem() {}

// IDA_EA: 0x85F710E
// IDA_NAME: _ZNK11Redeem_Item11CRedeemItem26_CheckVaildIndex_N_GetIterEi
// DEMANGLED: Redeem_Item::CRedeemItem::_CheckVaildIndex_N_GetIter(int)const
CRedeemItem::RedeemInfoList::const_iterator
CRedeemItem::_CheckVaildIndex_N_GetIter(int index) const {
    if (index < 0 || redeem_items_.empty() ||
        redeem_items_.size() < static_cast<unsigned int>(index + 1)) {
        return redeem_items_.end();
    }

    RedeemInfoList::const_iterator iter = redeem_items_.begin();
    std::advance(iter, index);
    return iter;
}

// IDA_EA: 0x85F71A8
// IDA_NAME: _ZN11Redeem_Item11CRedeemItem13AddRedeemListERK10Inven_Itemib
// DEMANGLED: Redeem_Item::CRedeemItem::AddRedeemList(Inven_Item const&,int,bool)
int CRedeemItem::AddRedeemList(const Inven_Item& item,
                               int value,
                               bool add_1000) {
    Inven_Item_Expand_RedeemInfo redeem_info;
    std::memcpy(redeem_info.item_storage, &item, sizeof(Inven_Item));
    redeem_info.expire_time = add_1000 ? value + 1000 : 1000;

    alter();
    if (redeem_items_.size() > 20u) {
        redeem_items_.pop_front();
    }
    redeem_items_.push_back(redeem_info);
    return 1;
}

// IDA_EA: 0x85F728A
// IDA_NAME: _ZN11Redeem_Item11CRedeemItem10RedeemItemEi
// DEMANGLED: Redeem_Item::CRedeemItem::RedeemItem(int)
Inven_Item_Expand_RedeemInfo CRedeemItem::RedeemItem(int index) {
    Inven_Item_Expand_RedeemInfo result;
    alter();

    const RedeemInfoList::const_iterator selected =
        _CheckVaildIndex_N_GetIter(index);
    RedeemInfoList::iterator iter = redeem_items_.begin();
    std::advance(iter,
                 std::distance(RedeemInfoList::const_iterator(iter),
                               selected));
    if (iter != redeem_items_.end()) {
        result = *iter;
        redeem_items_.erase(iter);
    }
    return result;
}

// IDA_EA: 0x85F7386
// IDA_NAME: _ZN11Redeem_Item11CRedeemItem12GetItemPriceEi
// DEMANGLED: Redeem_Item::CRedeemItem::GetItemPrice(int)
int CRedeemItem::GetItemPrice(int index) {
    const RedeemInfoList::const_iterator iter =
        _CheckVaildIndex_N_GetIter(index);
    if (iter == redeem_items_.end()) {
        return -1;
    }
    return iter->expire_time;
}

// IDA_EA: 0x85F73EE
// IDA_NAME: _ZN11Redeem_Item11CRedeemItem15Send_RedeemListEP5CUser
// DEMANGLED: Redeem_Item::CRedeemItem::Send_RedeemList(CUser *)
int CRedeemItem::Send_RedeemList(CUser* user) {
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_REDEEM_LIST);
    packet.put_byte(static_cast<int>(redeem_items_.size()));

    RedeemInfoList::iterator iter = redeem_items_.begin();
    unsigned int index = 0;
    while (index <= 20u && iter != redeem_items_.end()) {
        const Inven_Item& item = GetInvenItem(*iter);
        packet.put_int(iter->expire_time);
        packet.put_int(item.item_id_);
        packet.put_int(item.add_info_);
        packet.put_byte(item.GetItemAttr());
        packet.put_short(item.avatar_select_);
        packet.put_byte(item.seal_or_state_);
        if (G_GameWorld()->IsEnchantRevisionChannel()) {
            packet.put_int(0);
        } else {
            packet.put_int(item.extend_info_);
        }

        packet.put_byte(item.amplify_option_.getAbilityType());
        packet.put_short(item.amplify_option_.getAbilityValue());
        GetItemUpgrade(G_CDataManager())
            ->make3rdChroniclePacket(user, item, &packet);
        packet.put_packet(item);

        ++index;
        ++iter;
    }

    packet.finalize(true);
    user->Send(packet);
    return 1;
}

// IDA_EA: 0x85F7668
// IDA_NAME: _ZNK11Redeem_Item11CRedeemItem14GetRedeemInfoREi
// DEMANGLED: Redeem_Item::CRedeemItem::GetRedeemInfoR(int)const
Inven_Item_Expand_RedeemInfo CRedeemItem::GetRedeemInfoR(int index) const {
    Inven_Item_Expand_RedeemInfo result;
    const RedeemInfoList::const_iterator iter =
        _CheckVaildIndex_N_GetIter(index);
    if (iter != redeem_items_.end()) {
        result = *iter;
    }
    return result;
}

// IDA_EA: 0x85F76F0
// IDA_NAME: _ZN11Redeem_Item11CRedeemItem15isAddableFilterERK10Inven_Item
// DEMANGLED: Redeem_Item::CRedeemItem::isAddableFilter(Inven_Item const&)
bool CRedeemItem::isAddableFilter(const Inven_Item& item) {
    if (item.isEmpty() || _isFPItem(item)) {
        return false;
    }

    const uint32_t item_id = static_cast<uint32_t>(item.item_id_);
    switch (item_id) {
        case 3037:
        case 3313:
        case 4183:
        case 4350:
        case 4428:
        case 4439:
        case 4441:
        case 6515:
        case 7905:
        case 2680688:
            return false;
        default:
            break;
    }

    CItem* const item_data =
        G_CDataManager()->find_item(static_cast<int>(item_id));
    if (!item_data) {
        return false;
    }
    return (!item.isEquipableItemType() || item_data->get_rarity() ||
            item.GetUpgrade()) &&
           !item_data->IsBanRedeemItem();
}

// IDA_EA: 0x85F7830
// IDA_NAME: _ZN11Redeem_Item11CRedeemItem8loadDataEP5CUserPc
// DEMANGLED: Redeem_Item::CRedeemItem::loadData(CUser *,char *)
bool CRedeemItem::loadData(CUser* user, char* data) {
    (void)user;
    _reset();

    const SIG_REDEEMINFO_LIST_TO_DB* const payload =
        reinterpret_cast<const SIG_REDEEMINFO_LIST_TO_DB*>(data);
    for (uint32_t index = 0; index < payload->count; ++index) {
        if (GetInvenItem(payload->items[index]).item_id_) {
            redeem_items_.push_back(payload->items[index]);
        } else {
            alter();
        }
    }
    return 1;
}

// IDA_EA: 0x85F78BC
// IDA_NAME: _ZNK11Redeem_Item11CRedeemItem7getDataEPc
// DEMANGLED: Redeem_Item::CRedeemItem::getData(char *)const
void CRedeemItem::getData(char* data) const {
    SIG_REDEEMINFO_LIST_TO_DB* const payload =
        reinterpret_cast<SIG_REDEEMINFO_LIST_TO_DB*>(data);
    payload->count = static_cast<uint32_t>(redeem_items_.size());

    RedeemInfoList::const_iterator iter = redeem_items_.begin();
    unsigned int index = 0;
    while (index <= 20u && iter != redeem_items_.end()) {
        payload->items[index] = *iter;
        ++index;
        ++iter;
    }
}

// IDA_EA: 0x85F7998
// IDA_NAME: _ZN11Redeem_Item11CRedeemItem9_saveDataEP5CUser
// DEMANGLED: Redeem_Item::CRedeemItem::_saveData(CUser *)
int CRedeemItem::_saveData(CUser* user) {
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "RedeemItem.cpp", 264);
    CStreamGuard guard(stream, true);
    *guard << 500;
    *guard << user->GetUID();

    SIG_REDEEMINFO_LIST_TO_DB* const payload =
        guard->GetInBuffer<SIG_REDEEMINFO_LIST_TO_DB>();
    if (!payload) {
        return 0;
    }

    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
    payload->count = static_cast<uint32_t>(redeem_items_.size());
    unsigned int index = 0;
    for (RedeemInfoList::const_iterator iter = redeem_items_.begin();
         iter != redeem_items_.end(); ++iter, ++index) {
        payload->items[index] = *iter;
    }

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x85F7B6E
// IDA_NAME: _ZN11Redeem_Item11CRedeemItem6_resetEv
// DEMANGLED: Redeem_Item::CRedeemItem::_reset(void)
int CRedeemItem::_reset() {
    redeem_items_.clear();
    // std::list::clear @0x85F7F94 tail-calls _M_init @0x85F821C, whose
    // observed EAX is the list sentinel address (this+0x08).
    return static_cast<int>(reinterpret_cast<intptr_t>(&redeem_items_));
}

// IDA_EA: 0x85F7C04
// IDA_NAME: _ZNK11Redeem_Item11CRedeemItem9_isFPItemERK10Inven_Item
// DEMANGLED: Redeem_Item::CRedeemItem::_isFPItem(Inven_Item const&)const
bool CRedeemItem::_isFPItem(const Inven_Item& item) const {
    const uint32_t item_id = static_cast<uint32_t>(item.item_id_);
    return item_id > 0x1963u && item_id <= 0x1B57u;
}

}  // namespace Redeem_Item

// IDA_EA: 0x85F7BE0
// IDA_NAME: _ZNK5CItem15IsBanRedeemItemEv
// DEMANGLED: CItem::IsBanRedeemItem(void)const
int CItem::IsBanRedeemItem() const {
    return ban_redeem_item_;
}
