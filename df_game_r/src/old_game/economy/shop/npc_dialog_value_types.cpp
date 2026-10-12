#include "old_game/economy/shop/npc_dialog_value_types.h"

#include "infra/log/format/trace.h"


typedef char AssignNpc_Size_Must_Be_0x1c[
    (sizeof(AssignNpc) == 0x1c) ? 1 : -1];
typedef char NpcBuyLimitItem_Size_Must_Be_0x0c[
    (sizeof(NpcBuyLimitItem) == 0x0c) ? 1 : -1];
typedef char stNPCRelationship_t_Size_Must_Be_0x0c[
    (sizeof(stNPCRelationship_t) == 0x0c) ? 1 : -1];
typedef char stQuestShoptInitCost_Size_Must_Be_0x0c[
    (sizeof(stQuestShoptInitCost) == 0x0c) ? 1 : -1];
typedef char stNPCEventItemInfo_Size_Must_Be_0x08[
    (sizeof(stNPCEventItemInfo) == 0x08) ? 1 : -1];
typedef char CCeraShopBonusItemEvent_Size_Must_Be_0x10[
    (sizeof(CCeraShopBonusItemEvent) == 0x10) ? 1 : -1];
typedef char CSecretShopEvent_Size_Must_Be_0x0c[
    (sizeof(CSecretShopEvent) == 0x0c) ? 1 : -1];
typedef char CCeraShopGoods_Size_Must_Be_0x3c[
    (sizeof(CCeraShopGoods) == 0x3c) ? 1 : -1];
typedef char SECRET_SHOP_STATISTIC_DATA_Size_Must_Be_0x14[
    (sizeof(SECRET_SHOP_STATISTIC_DATA) == 0x14) ? 1 : -1];
typedef char stCeraShopBonusItemInfo_t_Size_Must_Be_0x0c[
    (sizeof(stCeraShopBonusItemInfo_t) == 0x0c) ? 1 : -1];
typedef char stCeraShopBonusInfo_t_Size_Must_Be_0x14[
    (sizeof(stCeraShopBonusInfo_t) == 0x14) ? 1 : -1];

// IDA_EA: 0x89E87EE
// IDA_NAME: _ZN9AssignNpc5clearEv
// DEMANGLED: AssignNpc::clear(void)
AssignNpc* AssignNpc::clear() {
    npc_id = 0;
    dungeon_id = -1;
    flag = 0;
    area = 0;
    x = 0;
    y = 0;
    z = 0;
    return this;
}

// IDA_EA: 0x89E8836
// IDA_NAME: _ZN9AssignNpcC2Ev
// DEMANGLED: AssignNpc::AssignNpc(void)
AssignNpc::AssignNpc() {
    clear();
}

// IDA_EA: 0x89F0D80
// IDA_NAME: _ZN9AssignNpcD2Ev
// DEMANGLED: AssignNpc::~AssignNpc()
AssignNpc::~AssignNpc() {}

// IDA_EA: 0x822B7CC
// IDA_NAME: _ZN15NpcBuyLimitItem5clearEv
// DEMANGLED: NpcBuyLimitItem::clear(void)
NpcBuyLimitItem* NpcBuyLimitItem::clear() {
    npc_id = 0;
    item_id = 0;
    buy_count = 0;
    return this;
}

// IDA_EA: 0x822B7EE
// IDA_NAME: _ZN15NpcBuyLimitItemC2Ev
// DEMANGLED: NpcBuyLimitItem::NpcBuyLimitItem(void)
NpcBuyLimitItem::NpcBuyLimitItem() {
    clear();
}

// IDA_EA: 0x848FA50
// IDA_NAME: _ZN19stNPCRelationship_tC2Ev
// DEMANGLED: stNPCRelationship_t::stNPCRelationship_t(void)
stNPCRelationship_t::stNPCRelationship_t()
    : npc_id(0),
      relationship(0),
      mood(0),
      reward_state(0),
      update_time(0) {}

// IDA_EA: 0x837109A
// IDA_NAME: _ZN20stQuestShoptInitCostC2Ev
// DEMANGLED: stQuestShoptInitCost::stQuestShoptInitCost(void)
stQuestShoptInitCost::stQuestShoptInitCost()
    : unknown_00(0),
      unknown_04(0),
      unknown_08(0) {}

// IDA_EA: 0x891B52E
// IDA_NAME: _ZN18stNPCEventItemInfoaSES_
// DEMANGLED: stNPCEventItemInfo::operator=(stNPCEventItemInfo)
stNPCEventItemInfo& stNPCEventItemInfo::operator=(stNPCEventItemInfo rhs) {
    item_id = rhs.item_id;
    count = rhs.count;
    return *this;
}

// IDA_EA: 0x893EC56
// IDA_NAME: _ZN18stNPCEventItemInfoD2Ev
// DEMANGLED: stNPCEventItemInfo::~stNPCEventItemInfo()
stNPCEventItemInfo::~stNPCEventItemInfo() {}

// IDA_EA: 0x810A2DC
// IDA_NAME: _ZN23CCeraShopBonusItemEventC2Ev
// DEMANGLED: CCeraShopBonusItemEvent::CCeraShopBonusItemEvent(void)
CCeraShopBonusItemEvent::CCeraShopBonusItemEvent()
    : CEventBase(),
      bonus_rate(0) {}

// IDA_EA: 0x810A2F8
// IDA_NAME: _ZN23CCeraShopBonusItemEventD2Ev
// DEMANGLED: CCeraShopBonusItemEvent::~CCeraShopBonusItemEvent()
// IDA_EA: 0x810A328
// IDA_NAME: _ZN23CCeraShopBonusItemEventD0Ev
// DEMANGLED: CCeraShopBonusItemEvent::~CCeraShopBonusItemEvent() deleting destructor
CCeraShopBonusItemEvent::~CCeraShopBonusItemEvent() {}

// IDA_EA: 0x810A346
// IDA_NAME: _ZN23CCeraShopBonusItemEvent10StartEventEv
// DEMANGLED: CCeraShopBonusItemEvent::StartEvent(void)
void CCeraShopBonusItemEvent::StartEvent() {}

// IDA_EA: 0x810A34C
// IDA_NAME: _ZN23CCeraShopBonusItemEvent10StartEventE10Word_Param
// DEMANGLED: CCeraShopBonusItemEvent::StartEvent(Word_Param)
void CCeraShopBonusItemEvent::StartEvent(Word_Param param) {
    SetEventFlag(true);
    const uint16_t rate = static_cast<uint16_t>(param);
    if (rate != 0 && rate <= 1000u) {
        bonus_rate = rate;
        cMyTrace trace("StartEvent", 32, 0);
        trace("Cera shop bonus item event Start");
        return;
    }

    cMyTrace trace("StartEvent", 26, 0);
    trace("Can not start Cera shop bonus item event. Bonus rate is too big(%d)",
          static_cast<int>(rate));
}

// IDA_EA: 0x810A406
// IDA_NAME: _ZN23CCeraShopBonusItemEvent8EndEventEv
// DEMANGLED: CCeraShopBonusItemEvent::EndEvent(void)
void CCeraShopBonusItemEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("EndEvent", 38, 0);
    trace("Cera shop bonus item event End");
}

// IDA_EA: 0x817A2E2
// IDA_NAME: _ZNK23CCeraShopBonusItemEvent12getBonusRateEv
// DEMANGLED: CCeraShopBonusItemEvent::getBonusRate(void)const
int CCeraShopBonusItemEvent::getBonusRate() const {
    return bonus_rate;
}

// IDA_EA: 0x81172C2
// IDA_NAME: _ZN16CSecretShopEventC2Ev
// DEMANGLED: CSecretShopEvent::CSecretShopEvent(void)
CSecretShopEvent::CSecretShopEvent()
    : CEventBase() {}

// IDA_EA: 0x81172DE
// IDA_NAME: _ZN16CSecretShopEventD2Ev
// DEMANGLED: CSecretShopEvent::~CSecretShopEvent()
// IDA_EA: 0x811730E
// IDA_NAME: _ZN16CSecretShopEventD0Ev
// DEMANGLED: CSecretShopEvent::~CSecretShopEvent() deleting destructor
CSecretShopEvent::~CSecretShopEvent() {}

// IDA_EA: 0x811732C
// IDA_NAME: _ZN16CSecretShopEvent10StartEventEv
// DEMANGLED: CSecretShopEvent::StartEvent(void)
void CSecretShopEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void CSecretShopEvent::StartEvent()", 22, 0);
    trace("CSecretShopEvent Event Start!");
}

// IDA_EA: 0x811737E
// IDA_NAME: _ZN16CSecretShopEvent10StartEventE10Word_Param
// DEMANGLED: CSecretShopEvent::StartEvent(Word_Param)
void CSecretShopEvent::StartEvent(Word_Param param) {
    (void)param;
    StartEvent();
}

// IDA_EA: 0x8117398
// IDA_NAME: _ZN16CSecretShopEvent8EndEventEv
// DEMANGLED: CSecretShopEvent::EndEvent(void)
void CSecretShopEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void CSecretShopEvent::EndEvent()", 32, 0);
    trace("CSecretShopEvent Event End!");
}

// IDA_EA: 0x8179FF6
// IDA_NAME: _ZNK14CCeraShopGoods10GetItemIdxEv
// DEMANGLED: CCeraShopGoods::GetItemIdx(void)const
int CCeraShopGoods::GetItemIdx() const {
    return item_idx;
}

// IDA_EA: 0x817A000
// IDA_NAME: _ZNK14CCeraShopGoods13GetStackCountEv
// DEMANGLED: CCeraShopGoods::GetStackCount(void)const
int CCeraShopGoods::GetStackCount() const {
    return stack_count;
}

// IDA_EA: 0x817A00C
// IDA_NAME: _ZNK14CCeraShopGoods12GetGoldPriceEv
// DEMANGLED: CCeraShopGoods::GetGoldPrice(void)const
int CCeraShopGoods::GetGoldPrice() const {
    return gold_price;
}

// IDA_EA: 0x817A018
// IDA_NAME: _ZNK14CCeraShopGoods11GetWinPointEv
// DEMANGLED: CCeraShopGoods::GetWinPoint(void)const
int CCeraShopGoods::GetWinPoint() const {
    return win_point;
}

// IDA_EA: 0x817A024
// IDA_NAME: _ZNK14CCeraShopGoods12GetCeraPriceEv
// DEMANGLED: CCeraShopGoods::GetCeraPrice(void)const
int CCeraShopGoods::GetCeraPrice() const {
    return cera_price;
}

// IDA_EA: 0x817A030
// IDA_NAME: _ZNK14CCeraShopGoods15GetMileagePriceEv
// DEMANGLED: CCeraShopGoods::GetMileagePrice(void)const
int CCeraShopGoods::GetMileagePrice() const {
    return mileage_price;
}

// IDA_EA: 0x817A03C
// IDA_NAME: _ZNK14CCeraShopGoods16GetSubGroupIndexEv
// DEMANGLED: CCeraShopGoods::GetSubGroupIndex(void)const
int CCeraShopGoods::GetSubGroupIndex() const {
    return sub_group_index;
}

// IDA_EA: 0x817A048
// IDA_NAME: _ZNK14CCeraShopGoods11GetGroupIdxEv
// DEMANGLED: CCeraShopGoods::GetGroupIdx(void)const
int CCeraShopGoods::GetGroupIdx() const {
    return group_idx;
}

// IDA_EA: 0x817A054
// IDA_NAME: _ZNK14CCeraShopGoods14GetFeaturedIdxEv
// DEMANGLED: CCeraShopGoods::GetFeaturedIdx(void)const
int CCeraShopGoods::GetFeaturedIdx() const {
    return featured_idx;
}

// IDA_EA: 0x817A060
// IDA_NAME: _ZNK14CCeraShopGoods10GetStickerEv
// DEMANGLED: CCeraShopGoods::GetSticker(void)const
int CCeraShopGoods::GetSticker() const {
    return sticker;
}

// IDA_EA: 0x817A06C
// IDA_NAME: _ZNK14CCeraShopGoods14GetRelatedItemEv
// DEMANGLED: CCeraShopGoods::GetRelatedItem(void)const
int CCeraShopGoods::GetRelatedItem() const {
    return related_item;
}

// IDA_EA: 0x817A078
// IDA_NAME: _ZNK14CCeraShopGoods19isGiftDisallowance_Ev
// DEMANGLED: CCeraShopGoods::isGiftDisallowance_(void)const
int CCeraShopGoods::isGiftDisallowance_() const {
    return gift_disallowance;
}

// IDA_EA: 0x817A084
// IDA_NAME: _ZNK14CCeraShopGoods24isCeraPointDisallowance_Ev
// DEMANGLED: CCeraShopGoods::isCeraPointDisallowance_(void)const
int CCeraShopGoods::isCeraPointDisallowance_() const {
    return cera_point_disallowance;
}

// IDA_EA: 0x83280C4
// IDA_NAME: _ZNK14CCeraShopGoods13GetExpiryDateEv
// DEMANGLED: CCeraShopGoods::GetExpiryDate(void)const
int CCeraShopGoods::GetExpiryDate() const {
    return expiry_date;
}

// IDA_EA: 0x83280D0
// IDA_NAME: _ZNK14CCeraShopGoods14isStackBuyableEv
// DEMANGLED: CCeraShopGoods::isStackBuyable(void)const
int CCeraShopGoods::isStackBuyable() const {
    return stack_buyable;
}

// IDA_EA: 0x854C582
// IDA_NAME: _ZNK14CCeraShopGoods14GetBonusWeightEv
// DEMANGLED: CCeraShopGoods::GetBonusWeight(void)const
float CCeraShopGoods::GetBonusWeight() const {
    return bonus_weight;
}

// IDA_EA: 0x85FD45A
// IDA_NAME: _ZN26SECRET_SHOP_STATISTIC_DATAC2Ev
// DEMANGLED: SECRET_SHOP_STATISTIC_DATA::SECRET_SHOP_STATISTIC_DATA(void)
SECRET_SHOP_STATISTIC_DATA::SECRET_SHOP_STATISTIC_DATA()
    : item_id(0),
      count(0),
      raw_08(0),
      raw_0c(0),
      raw_10(0) {}

// IDA_EA: 0x89363DA
// IDA_NAME: _ZN25stCeraShopBonusItemInfo_tD2Ev
// DEMANGLED: stCeraShopBonusItemInfo_t::~stCeraShopBonusItemInfo_t()
stCeraShopBonusItemInfo_t::~stCeraShopBonusItemInfo_t() {}

// IDA_EA: 0x891BE5C
// IDA_NAME: _ZN21stCeraShopBonusInfo_tC2Ev
// DEMANGLED: stCeraShopBonusInfo_t::stCeraShopBonusInfo_t(void)
stCeraShopBonusInfo_t::stCeraShopBonusInfo_t() {}

// IDA_EA: 0x8525F3C
// IDA_NAME: _ZN21stCeraShopBonusInfo_tD2Ev
// DEMANGLED: stCeraShopBonusInfo_t::~stCeraShopBonusInfo_t()
stCeraShopBonusInfo_t::~stCeraShopBonusInfo_t() {}

// IDA_EA: 0x8924C00
// IDA_NAME: _ZN21stCeraShopBonusInfo_tC2ERKS_
// DEMANGLED: stCeraShopBonusInfo_t::stCeraShopBonusInfo_t(stCeraShopBonusInfo_t const&)
stCeraShopBonusInfo_t::stCeraShopBonusInfo_t(
    const stCeraShopBonusInfo_t& rhs)
    : raw_00(rhs.raw_00),
      raw_04(rhs.raw_04),
      bonus_items(rhs.bonus_items) {}

// IDA_EA: 0x8924CBA
// IDA_NAME: _ZN21stCeraShopBonusInfo_taSERKS_
// DEMANGLED: stCeraShopBonusInfo_t::operator=(stCeraShopBonusInfo_t const&)
stCeraShopBonusInfo_t& stCeraShopBonusInfo_t::operator=(
    const stCeraShopBonusInfo_t& rhs) {
    raw_00 = rhs.raw_00;
    raw_04 = rhs.raw_04;
    bonus_items = rhs.bonus_items;
    return *this;
}

namespace WongWork {

// IDA_EA: 0x8327EE6
// IDA_NAME: _ZN8WongWork9CCeraShop15setOriginalItemEi
// DEMANGLED: WongWork::CCeraShop::setOriginalItem(int)
CCeraShop* CCeraShop::setOriginalItem(int item_id) {
    original_item = item_id;
    return this;
}

// IDA_EA: 0x8327EF4
// IDA_NAME: _ZN8WongWork9CCeraShop15getOriginalItemEv
// DEMANGLED: WongWork::CCeraShop::getOriginalItem(void)
int CCeraShop::getOriginalItem() {
    if (original_item <= 0) {
        return 0;
    }
    return original_item;
}

// IDA_EA: 0x83280DC
// IDA_NAME: _ZN8WongWork9CCeraShop19Is2010ChuseokPacketEj
// DEMANGLED: WongWork::CCeraShop::Is2010ChuseokPacket(unsigned int)
bool CCeraShop::Is2010ChuseokPacket(unsigned int packet_id) {
    return packet_id > 0x2896D3u && packet_id <= 0x2896EFu;
}

}  // namespace WongWork
