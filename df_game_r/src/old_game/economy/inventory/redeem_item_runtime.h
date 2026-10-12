#ifndef OLDSRV_OLD_GAME_INVENTORY_REDEEM_ITEM_RUNTIME_H_
#define OLDSRV_OLD_GAME_INVENTORY_REDEEM_ITEM_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>

#include <list>
#include <type_traits>

#include "old_game/player/core/charac_expand_cdata.h"


class CUser;
struct Inven_Item;

namespace Redeem_Item {

// IDA_EA: 0x848F876.  The old value object placement-constructs the canonical
// 0x3d-byte Inven_Item but has no destructor identity of its own.  Keeping the
// item as constructor-only storage preserves that lifetime and the exact
// expiration-value offset at +0x40.
struct Inven_Item_Expand_RedeemInfo {
    Inven_Item_Expand_RedeemInfo();

    uint8_t item_storage[0x3d];
    uint8_t ida_off_03d;
    uint8_t ida_off_03e;
    uint8_t ida_off_03f;
    int32_t expire_time;
};

// The C2 at 0x848F876 placement-constructs a canonical Inven_Item in this
// member.  These accessors expose that existing typed subobject without
// changing the record ABI or creating another item lifetime.
Inven_Item& GetInvenItem(Inven_Item_Expand_RedeemInfo& info);
const Inven_Item& GetInvenItem(const Inven_Item_Expand_RedeemInfo& info);

// IDA unit: 99_misc_global/Redeem_Item/CRedeemItem.
class CRedeemItem : public charac_expand::CData {
public:
    CRedeemItem();
    virtual ~CRedeemItem();

    int AddRedeemList(const Inven_Item& item, int value, bool add_1000);
    Inven_Item_Expand_RedeemInfo RedeemItem(int index);
    int GetItemPrice(int index);
    int Send_RedeemList(CUser* user);
    Inven_Item_Expand_RedeemInfo GetRedeemInfoR(int index) const;
    bool isAddableFilter(const Inven_Item& item);

    virtual bool loadData(CUser* user, char* data);
    virtual void getData(char* data) const;

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    friend class ::CUser;

    typedef std::list<Inven_Item_Expand_RedeemInfo> RedeemInfoList;

    RedeemInfoList::const_iterator _CheckVaildIndex_N_GetIter(
        int index) const;
    bool _isFPItem(const Inven_Item& item) const;

    RedeemInfoList redeem_items_;  // IDA +0x08
};

}  // namespace Redeem_Item

// IDA_EA: 0x848F894.  This is both the DB-load record and the save payload
// selected by Stream/CStreamGuard at 0x85F81B2/0x85F7F7E.
struct SIG_REDEEMINFO_LIST_TO_DB {
    SIG_REDEEMINFO_LIST_TO_DB();

    uint32_t charac_no;
    uint32_t count;
    Redeem_Item::Inven_Item_Expand_RedeemInfo items[21];
};

static_assert(sizeof(Redeem_Item::Inven_Item_Expand_RedeemInfo) == 0x44u,
              "redeem item value must match IDA");
static_assert(offsetof(Redeem_Item::Inven_Item_Expand_RedeemInfo,
                       expire_time) == 0x40u,
              "redeem expiration offset must match IDA");
static_assert(
    std::is_trivially_destructible<
        Redeem_Item::Inven_Item_Expand_RedeemInfo>::value,
    "redeem item value must not synthesize an old-server destructor");
static_assert(sizeof(SIG_REDEEMINFO_LIST_TO_DB) == 0x59cu,
              "redeem DB payload must match IDA");
static_assert(offsetof(SIG_REDEEMINFO_LIST_TO_DB, items) == 0x08u,
              "redeem DB item-array offset must match IDA");
static_assert(std::is_trivially_destructible<
                  SIG_REDEEMINFO_LIST_TO_DB>::value,
              "redeem DB payload must not synthesize an old-server destructor");

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Redeem_Item::CRedeemItem) == 0x10u,
              "CRedeemItem IDA ABI size must stay 0x10");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_REDEEM_ITEM_RUNTIME_H_
