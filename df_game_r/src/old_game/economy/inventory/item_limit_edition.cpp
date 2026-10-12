#include "old_game/economy/inventory/item_limit_edition.h"

#include "common/manager/global_data.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

#include <new>

namespace {

PacketGuard* AsOldsrvPacketGuard(PacketGuard& packet) {
    return reinterpret_cast<PacketGuard*>(&packet);
}

}  // namespace

// IDA_EA: 0x84E9732
// IDA_NAME: _ZN17CItemLimitEditionC2ERK28stItemLimitEditionItemInfo_t
// DEMANGLED: CItemLimitEdition::CItemLimitEdition(stItemLimitEditionItemInfo_t const&)
CItemLimitEdition::CItemLimitEdition(
    const stItemLimitEditionItemInfo_t& source)
    : info(source), current_sell_count(source.initial_current_sell_count) {}

// IDA_EA: 0x8374D48
// IDA_NAME: _ZN17CItemLimitEditionD2Ev
// DEMANGLED: CItemLimitEdition::~CItemLimitEdition()
CItemLimitEdition::~CItemLimitEdition() {}

// IDA_EA: 0x817A090
// IDA_NAME: _ZNK17CItemLimitEdition14getRelateditemEv
// DEMANGLED: CItemLimitEdition::getRelateditem(void)const
int CItemLimitEdition::getRelateditem() const {
    return static_cast<int>(info.related_item);
}

// IDA_EA: 0x817A09C
// IDA_NAME: _ZNK17CItemLimitEdition14getFeaturedIdxEv
// DEMANGLED: CItemLimitEdition::getFeaturedIdx(void)const
int CItemLimitEdition::getFeaturedIdx() const {
    return static_cast<int>(info.featured_index);
}

// IDA_EA: 0x822CA98
// IDA_NAME: _ZNK17CItemLimitEdition17getRelatedItemCntEv
// DEMANGLED: CItemLimitEdition::getRelatedItemCnt(void)const
int CItemLimitEdition::getRelatedItemCnt() const {
    return static_cast<int>(info.related_item_count);
}

// IDA_EA: 0x822CAA4
// IDA_NAME: _ZNK17CItemLimitEdition14getSellEndTimeEv
// DEMANGLED: CItemLimitEdition::getSellEndTime(void)const
int CItemLimitEdition::getSellEndTime() const {
    return static_cast<int>(info.sell_end_time);
}

// IDA_EA: 0x822CAB0
// IDA_NAME: _ZNK17CItemLimitEdition19getAvatarTypeSelectEv
// DEMANGLED: CItemLimitEdition::getAvatarTypeSelect(void)const
int CItemLimitEdition::getAvatarTypeSelect() const {
    return static_cast<int>(info.avatar_type_select);
}

// IDA_EA: 0x822CABC
// IDA_NAME: _ZNK17CItemLimitEdition13getCurSellNumEv
// DEMANGLED: CItemLimitEdition::getCurSellNum(void)const
int CItemLimitEdition::getCurSellNum() const {
    return static_cast<int>(current_sell_count);
}

// IDA_EA: 0x822CAC8
// IDA_NAME: _ZNK17CItemLimitEdition14isSellCompleteEv
// DEMANGLED: CItemLimitEdition::isSellComplete(void)const
bool CItemLimitEdition::isSellComplete() const {
    return info.total_sell_count != 0xFFFFFFFFu &&
           current_sell_count >= info.total_sell_count;
}

// IDA_EA: 0x822CAFA
// IDA_NAME: _ZNK17CItemLimitEdition12getPriceCeraEv
// DEMANGLED: CItemLimitEdition::getPriceCera(void)const
int CItemLimitEdition::getPriceCera() const {
    return static_cast<int>(info.price_cera);
}

// IDA_EA: 0x822CB06
// IDA_NAME: _ZNK17CItemLimitEdition12getPriceGoldEv
// DEMANGLED: CItemLimitEdition::getPriceGold(void)const
int CItemLimitEdition::getPriceGold() const {
    return static_cast<int>(info.price_gold);
}

// IDA_EA: 0x822CB12
// IDA_NAME: _ZNK17CItemLimitEdition16getPriceWinPointEv
// DEMANGLED: CItemLimitEdition::getPriceWinPoint(void)const
int CItemLimitEdition::getPriceWinPoint() const {
    return 0;
}

// IDA_EA: 0x822CB1C
// IDA_NAME: _ZNK17CItemLimitEdition16getCondCharacJobEv
// DEMANGLED: CItemLimitEdition::getCondCharacJob(void)const
int CItemLimitEdition::getCondCharacJob() const {
    return static_cast<int>(info.cond_charac_job);
}

// IDA_EA: 0x822CB28
// IDA_NAME: _ZNK17CItemLimitEdition23getCondCharacLevelBeginEv
// DEMANGLED: CItemLimitEdition::getCondCharacLevelBegin(void)const
int CItemLimitEdition::getCondCharacLevelBegin() const {
    return static_cast<int>(info.cond_charac_level_begin);
}

// IDA_EA: 0x822CB34
// IDA_NAME: _ZNK17CItemLimitEdition21getCondCharacLevelEndEv
// DEMANGLED: CItemLimitEdition::getCondCharacLevelEnd(void)const
int CItemLimitEdition::getCondCharacLevelEnd() const {
    return static_cast<int>(info.cond_charac_level_end);
}

// IDA_EA: 0x822CB40
// IDA_NAME: _ZNK17CItemLimitEdition28getCondCharacCreateTimeBeginEv
// DEMANGLED: CItemLimitEdition::getCondCharacCreateTimeBegin(void)const
int CItemLimitEdition::getCondCharacCreateTimeBegin() const {
    return static_cast<int>(info.cond_charac_create_time_begin);
}

// IDA_EA: 0x822CB4C
// IDA_NAME: _ZNK17CItemLimitEdition26getCondCharacCreateTimeEndEv
// DEMANGLED: CItemLimitEdition::getCondCharacCreateTimeEnd(void)const
int CItemLimitEdition::getCondCharacCreateTimeEnd() const {
    return static_cast<int>(info.cond_charac_create_time_end);
}

// IDA_EA: 0x822CB58
// IDA_NAME: _ZNK17CItemLimitEdition25getCondAccCreateTimeBeginEv
// DEMANGLED: CItemLimitEdition::getCondAccCreateTimeBegin(void)const
int CItemLimitEdition::getCondAccCreateTimeBegin() const {
    return static_cast<int>(info.cond_acc_create_time_begin);
}

// IDA_EA: 0x822CB64
// IDA_NAME: _ZNK17CItemLimitEdition23getCondAccCreateTimeEndEv
// DEMANGLED: CItemLimitEdition::getCondAccCreateTimeEnd(void)const
int CItemLimitEdition::getCondAccCreateTimeEnd() const {
    return static_cast<int>(info.cond_acc_create_time_end);
}

// IDA_EA: 0x84E970E
// IDA_NAME: _ZN17CItemLimitEdition16updateCurSellNumEj
// DEMANGLED: CItemLimitEdition::updateCurSellNum(unsigned int)
CItemLimitEdition* CItemLimitEdition::updateCurSellNum(
    unsigned int cur_sell_num) {
    current_sell_count = cur_sell_num;
    return this;
}

// IDA_EA: 0x84E971C
// IDA_NAME: _ZNK17CItemLimitEdition8getIPGNOEv
// DEMANGLED: CItemLimitEdition::getIPGNO(void)const
int CItemLimitEdition::getIPGNO() const {
    return static_cast<int>(info.ipg_no);
}

// IDA_EA: 0x84E9726
// IDA_NAME: _ZNK17CItemLimitEdition15getTotalSellNumEv
// DEMANGLED: CItemLimitEdition::getTotalSellNum(void)const
int CItemLimitEdition::getTotalSellNum() const {
    return static_cast<int>(info.total_sell_count);
}

// IDA_EA: 0x85130E8
// IDA_NAME: _ZNK17CItemLimitEdition14makeInfoPacketER11PacketGuard
// DEMANGLED: CItemLimitEdition::makeInfoPacket(PacketGuard &)const
int CItemLimitEdition::makeInfoPacket(PacketGuard& packet) const {
    PacketGuard* out = AsOldsrvPacketGuard(packet);
    out->put_int(static_cast<int>(info.ipg_no));
    out->put_item_idx(info.related_item);
    out->put_int(static_cast<int>(info.related_item_count));
    out->put_byte(static_cast<int>(info.avatar_type_select));
    out->put_int(static_cast<int>(info.price_cera));
    out->put_int(static_cast<int>(info.price_gold));
    out->put_int(static_cast<int>(current_sell_count));
    out->put_int(static_cast<int>(info.total_sell_count));
    out->put_int(static_cast<int>(info.featured_index));

    int remain_sec = 0;
    CSystemTime* system_time = GlobalData::system_time();
    if (static_cast<int>(info.sell_end_time) > system_time->getCurSec()) {
        remain_sec =
            static_cast<int>(info.sell_end_time) - system_time->getCurSec();
    }
    out->put_int(remain_sec);

    out->put_int(static_cast<int>(info.raw_2c));
    out->put_byte(static_cast<int>(info.cond_charac_job));
    out->put_short(static_cast<int>(info.cond_charac_level_begin));
    out->put_short(static_cast<int>(info.cond_charac_level_end));
    out->put_int(static_cast<int>(info.cond_acc_create_time_begin));
    out->put_int(static_cast<int>(info.cond_acc_create_time_end));
    out->put_int(static_cast<int>(info.cond_charac_create_time_begin));
    return out->put_int(static_cast<int>(info.cond_charac_create_time_end));
}

// IDA_EA: 0x8374E0C
// IDA_NAME: _ZN20CItemLimitEditionMgrC2Ev
// DEMANGLED: CItemLimitEditionMgr::CItemLimitEditionMgr(void)
CItemLimitEditionMgr::CItemLimitEditionMgr() : items_() {}

// IDA_EA: 0x8374E20
// IDA_NAME: _ZN20CItemLimitEditionMgrD2Ev
// DEMANGLED: CItemLimitEditionMgr::~CItemLimitEditionMgr()
CItemLimitEditionMgr::~CItemLimitEditionMgr() {
    clear();
}

// IDA_EA: 0x817A0A8
// IDA_NAME: _ZNK20CItemLimitEditionMgr11getItemInfoEj
// DEMANGLED: CItemLimitEditionMgr::getItemInfo(unsigned int)const
CItemLimitEdition* CItemLimitEditionMgr::getItemInfo(
    unsigned int ipg_no) const {
    ItemMap::const_iterator found = items_.find(ipg_no);
    if (found == items_.end()) {
        return 0;
    }
    return found->second;
}

// IDA_EA: 0x8374D4E
// IDA_NAME: _ZN20CItemLimitEditionMgr5clearEv
// DEMANGLED: CItemLimitEditionMgr::clear(void)
int CItemLimitEditionMgr::clear() {
    for (ItemMap::iterator it = items_.begin(); it != items_.end(); ++it) {
        CItemLimitEdition* item = it->second;
        if (item != 0) {
            item->~CItemLimitEdition();
            ::operator delete(item);
        }
    }
    items_.clear();
    return 0;
}

// IDA_EA: 0x84E975E
// IDA_NAME: _ZN20CItemLimitEditionMgr10registItemERK28stItemLimitEditionItemInfo_t
// DEMANGLED: CItemLimitEditionMgr::registItem(stItemLimitEditionItemInfo_t const&)
int CItemLimitEditionMgr::registItem(
    const stItemLimitEditionItemInfo_t& info) {
    CItemLimitEdition* item =
        new (::operator new(sizeof(CItemLimitEdition)))
            CItemLimitEdition(info);
    const unsigned int ipg_no = static_cast<unsigned int>(item->getIPGNO());

    ItemMap::iterator found = items_.find(ipg_no);
    if (found != items_.end()) {
        CItemLimitEdition* old_item = found->second;
        if (old_item != 0) {
            old_item->~CItemLimitEdition();
            ::operator delete(old_item);
        }
        items_.erase(found);
    }

    return items_.insert(ItemMap::value_type(ipg_no, item)).second ? 1 : 0;
}

// IDA_EA: 0x84E9876
// IDA_NAME: _ZN20CItemLimitEditionMgr10removeItemEj
// DEMANGLED: CItemLimitEditionMgr::removeItem(unsigned int)
int CItemLimitEditionMgr::removeItem(unsigned int ipg_no) {
    ItemMap::iterator found = items_.find(ipg_no);
    if (found == items_.end()) {
        return 0;
    }

    CItemLimitEdition* item = found->second;
    if (item != 0) {
        item->~CItemLimitEdition();
        ::operator delete(item);
    }
    items_.erase(found);
    return 1;
}

// IDA_EA: 0x84E9904
// IDA_NAME: _ZN20CItemLimitEditionMgr10updateItemEjj
// DEMANGLED: CItemLimitEditionMgr::updateItem(unsigned int,unsigned int)
int CItemLimitEditionMgr::updateItem(unsigned int ipg_no,
                                     unsigned int cur_sell_num) {
    ItemMap::iterator found = items_.find(ipg_no);
    if (found == items_.end()) {
        return 0;
    }
    found->second->updateCurSellNum(cur_sell_num);
    return 1;
}

// IDA_EA: 0x84E997C
// IDA_NAME: _ZNK20CItemLimitEditionMgr5emptyEv
// DEMANGLED: CItemLimitEditionMgr::empty(void)const
bool CItemLimitEditionMgr::empty() const {
    return items_.empty();
}

// IDA_EA: 0x85132CA
// IDA_NAME: _ZNK20CItemLimitEditionMgr24makeItemLimitEditionInfoER11PacketGuard
// DEMANGLED: CItemLimitEditionMgr::makeItemLimitEditionInfo(PacketGuard &)const
int CItemLimitEditionMgr::makeItemLimitEditionInfo(PacketGuard& packet) const {
    PacketGuard* out = AsOldsrvPacketGuard(packet);
    int index = out->get_index();
    out->put_byte(0);
    int count = 0;
    for (ItemMap::const_iterator it = items_.begin(); it != items_.end();
         ++it) {
        it->second->makeInfoPacket(packet);
        ++count;
    }
    return out->put_byte(index, count);
}

// IDA_EA: 0x8513382
// IDA_NAME: _ZNK20CItemLimitEditionMgr30makeItemLimitEditionRemoveInfoER11PacketGuard
// DEMANGLED: CItemLimitEditionMgr::makeItemLimitEditionRemoveInfo(PacketGuard &)const
int CItemLimitEditionMgr::makeItemLimitEditionRemoveInfo(
    PacketGuard& packet) const {
    PacketGuard* out = AsOldsrvPacketGuard(packet);
    int index = out->get_index();
    out->put_byte(0);
    int count = 0;
    for (ItemMap::const_iterator it = items_.begin(); it != items_.end();
         ++it) {
        out->put_int(it->second->getIPGNO());
        ++count;
    }
    return out->put_byte(index, count);
}
