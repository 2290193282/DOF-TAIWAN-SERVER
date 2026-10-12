#ifndef OLDSRV_GLOBAL_ITEM_LIMIT_EDITION_H_
#define OLDSRV_GLOBAL_ITEM_LIMIT_EDITION_H_

#include <stddef.h>
#include <stdint.h>

#include <ext/hash_map>

class PacketGuard;

// IDA unit: 99_misc_global/CItemLimitEdition.
// IDA_EA: ctor @0x84E9732, dtor @0x8374D48,
// makeInfoPacket @0x85130E8.
// The source row is copied verbatim into CItemLimitEdition, then word 6 is
// mirrored to the live current-sell counter at +0x48.
struct stItemLimitEditionItemInfo_t {
    uint32_t ipg_no;
    uint32_t related_item;
    uint32_t related_item_count;
    uint8_t avatar_type_select;
    uint8_t reserved_0d[3];
    uint32_t price_cera;
    uint32_t price_gold;
    uint32_t initial_current_sell_count;
    uint32_t total_sell_count;
    uint32_t featured_index;
    uint32_t raw_24;
    uint32_t sell_end_time;
    uint32_t raw_2c;
    uint32_t cond_charac_job;
    uint16_t cond_charac_level_begin;
    uint16_t cond_charac_level_end;
    uint32_t cond_acc_create_time_begin;
    uint32_t cond_acc_create_time_end;
    uint32_t cond_charac_create_time_begin;
    uint32_t cond_charac_create_time_end;
};

static_assert(sizeof(stItemLimitEditionItemInfo_t) == 0x48u,
              "stItemLimitEditionItemInfo_t IDA ABI size must stay 0x48");
static_assert(offsetof(stItemLimitEditionItemInfo_t,
                       cond_charac_level_begin) == 0x34u,
              "stItemLimitEditionItemInfo_t level offset must stay 0x34");

struct CItemLimitEdition {
    explicit CItemLimitEdition(const stItemLimitEditionItemInfo_t& info);
    ~CItemLimitEdition();

    int getRelateditem() const;
    int getFeaturedIdx() const;
    int getRelatedItemCnt() const;
    int getSellEndTime() const;
    int getAvatarTypeSelect() const;
    int getCurSellNum() const;
    bool isSellComplete() const;
    int getPriceCera() const;
    int getPriceGold() const;
    int getPriceWinPoint() const;
    int getCondCharacJob() const;
    int getCondCharacLevelBegin() const;
    int getCondCharacLevelEnd() const;
    int getCondCharacCreateTimeBegin() const;
    int getCondCharacCreateTimeEnd() const;
    int getCondAccCreateTimeBegin() const;
    int getCondAccCreateTimeEnd() const;
    CItemLimitEdition* updateCurSellNum(unsigned int cur_sell_num);
    int getIPGNO() const;
    int getTotalSellNum() const;
    int makeInfoPacket(PacketGuard& packet) const;

    stItemLimitEditionItemInfo_t info;
    uint32_t current_sell_count;
};

static_assert(sizeof(CItemLimitEdition) == 0x4Cu,
              "CItemLimitEdition IDA ABI size must stay 0x4C");
static_assert(offsetof(CItemLimitEdition, current_sell_count) == 0x48u,
              "CItemLimitEdition current_sell_count offset must stay 0x48");

// IDA unit: 99_misc_global/CItemLimitEditionMgr.
// IDA_EA: ctor @0x8374E0C, dtor @0x8374E20, clear @0x8374D4E,
// getItemInfo @0x817A0A8.
struct CItemLimitEditionMgr {
    CItemLimitEditionMgr();
    ~CItemLimitEditionMgr();

    CItemLimitEdition* getItemInfo(unsigned int ipg_no) const;
    int clear();
    int registItem(const stItemLimitEditionItemInfo_t& info);
    int removeItem(unsigned int ipg_no);
    int updateItem(unsigned int ipg_no, unsigned int cur_sell_num);
    bool empty() const;
    int makeItemLimitEditionInfo(PacketGuard& packet) const;
    int makeItemLimitEditionRemoveInfo(PacketGuard& packet) const;

private:
    typedef __gnu_cxx::hash_map<unsigned int, CItemLimitEdition*> ItemMap;
    ItemMap items_;
};

static_assert(sizeof(CItemLimitEditionMgr) == 0x14u,
              "CItemLimitEditionMgr IDA ABI size must stay 0x14");


using ::CItemLimitEdition;
using ::CItemLimitEditionMgr;
using ::stItemLimitEditionItemInfo_t;


#endif  // OLDSRV_GLOBAL_ITEM_LIMIT_EDITION_H_
