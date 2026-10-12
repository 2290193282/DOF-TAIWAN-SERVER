#include "old_game/economy/mall/cerashop_purchase_bonus_runtime.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "common/manager/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/player/core/cuser.h"

namespace {

int ImportCashShopPurcahseBonusItemList(
    const char* filename,
    std::map<int, std::vector<ARAD::PurcahseBonus> >& bonus_items_by_goods) {
    if (!loadRDARScriptFile("", filename)) {
        return 10;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag != "[bonus]") {
            continue;
        }

        bool success = false;
        const int goods_no = ScanInt(&success);
        if (!success) {
            return 27;
        }

        std::vector<ARAD::PurcahseBonus> bonuses;
        for (;;) {
            const int required_purchase_count = ScanInt(&success);
            if (!success) {
                break;
            }

            const int bonus_item_index = ScanInt(&success);
            if (!success) {
                return 35;
            }

            const int bonus_item_count = ScanInt(&success);
            if (!success) {
                return 38;
            }

            ARAD::PurcahseBonus bonus;
            bonus.required_purchase_count_ =
                static_cast<uint8_t>(required_purchase_count);
            bonus.bonus_item_index_ = bonus_item_index;
            bonus.bonus_item_count_ =
                static_cast<uint16_t>(bonus_item_count);

            if (std::find(bonuses.begin(), bonuses.end(), bonus) !=
                bonuses.end()) {
                return 44;
            }
            bonuses.push_back(bonus);
        }

        if (bonus_items_by_goods.find(goods_no) !=
            bonus_items_by_goods.end()) {
            return 55;
        }
        bonus_items_by_goods.insert(std::make_pair(goods_no, bonuses));
    }

    return 0;
}

}  // namespace

namespace ARAD {

// IDA_EA: 0x81AC710
// IDA_NAME: _ZN4ARAD13PurcahseBonusC2Ev
// DEMANGLED: ARAD::PurcahseBonus::PurcahseBonus(void)
PurcahseBonus::PurcahseBonus()
    : required_purchase_count_(0),
      alignment_01_{0, 0, 0},
      bonus_item_index_(0),
      bonus_item_count_(0) {}

// IDA_EA: 0x88B72B4
// IDA_NAME: _ZN4ARAD13PurcahseBonusD2Ev
// DEMANGLED: ARAD::PurcahseBonus::~PurcahseBonus()
PurcahseBonus::~PurcahseBonus() {}

// IDA_EA: 0x88B66CE
// IDA_NAME: _ZNK4ARAD13PurcahseBonuseqERKS0_
// DEMANGLED: ARAD::PurcahseBonus::operator==(ARAD::PurcahseBonus const&)const
bool PurcahseBonus::operator==(const PurcahseBonus& rhs) const {
    return required_purchase_count_ == rhs.required_purchase_count_;
}

// IDA_EA: 0x854C5B6
// IDA_NAME: _ZN4ARAD26CeraShopPurcahseCountBonusC2Ev
// DEMANGLED: ARAD::CeraShopPurcahseCountBonus::CeraShopPurcahseCountBonus(void)
CeraShopPurcahseCountBonus::CeraShopPurcahseCountBonus()
    : bonus_items_by_goods_() {}

// IDA_EA: 0x854C5CA
// IDA_NAME: _ZN4ARAD26CeraShopPurcahseCountBonusD2Ev
// DEMANGLED: ARAD::CeraShopPurcahseCountBonus::~CeraShopPurcahseCountBonus()
CeraShopPurcahseCountBonus::~CeraShopPurcahseCountBonus() {}

// IDA_EA: 0x81ABF14
// IDA_NAME: _ZN4ARAD26CeraShopPurcahseCountBonus10LoadScriptEv
// DEMANGLED: ARAD::CeraShopPurcahseCountBonus::LoadScript(void)
int CeraShopPurcahseCountBonus::LoadScript() {
    return ImportCashShopPurcahseBonusItemList(
               "CashShop/PurcahseBonusItem.shp", bonus_items_by_goods_) <= 0;
}

// IDA_EA: 0x81ABF44
// IDA_NAME: _ZN4ARAD26CeraShopPurcahseCountBonus13GiveBonusItemEP5CUsermit
// DEMANGLED: ARAD::CeraShopPurcahseCountBonus::GiveBonusItem(CUser *,unsigned long,int,unsigned short)
int CeraShopPurcahseCountBonus::GiveBonusItem(CUser* user,
                                              unsigned long purchase_index,
                                              int goods_no,
                                              unsigned short purchase_count) {
    (void)purchase_index;

    if (!CheckBonusItem(goods_no)) {
        return 1;
    }

    const std::map<int, std::vector<PurcahseBonus> >::const_iterator found =
        bonus_items_by_goods_.find(goods_no);
    PurcahseBonus bonus;
    if (found == bonus_items_by_goods_.end() ||
        !_findBonusItem(found->second, bonus, purchase_count)) {
        return 1;
    }

    _processBonusItem(user, goods_no, bonus);
    return 0;
}

// IDA_EA: 0x81ABFF0
// IDA_NAME: _ZN4ARAD26CeraShopPurcahseCountBonus14CheckBonusItemEi
// DEMANGLED: ARAD::CeraShopPurcahseCountBonus::CheckBonusItem(int)
int CeraShopPurcahseCountBonus::CheckBonusItem(int goods_no) {
    return bonus_items_by_goods_.find(goods_no) != bonus_items_by_goods_.end();
}

// IDA_EA: 0x81AC04C
// IDA_NAME: _ZN4ARAD26CeraShopPurcahseCountBonus14_findBonusItemERKSt6vectorINS_13PurcahseBonusESaIS2_EERS2_t
// DEMANGLED: ARAD::CeraShopPurcahseCountBonus::_findBonusItem(std::vector<ARAD::PurcahseBonus> const&,ARAD::PurcahseBonus&,unsigned short)
int CeraShopPurcahseCountBonus::_findBonusItem(
    const std::vector<PurcahseBonus>& bonuses,
    PurcahseBonus& bonus,
    unsigned short purchase_count) {
    bool found = false;
    for (std::vector<PurcahseBonus>::const_iterator it = bonuses.begin();
         it != bonuses.end();
         ++it) {
        if (it->required_purchase_count_ <= purchase_count) {
            bonus = *it;
            found = true;
        }
    }
    return found ? 1 : 0;
}

// IDA_EA: 0x81AC13E
// IDA_NAME: _ZN4ARAD26CeraShopPurcahseCountBonus17_processBonusItemEP5CUseriRKNS_13PurcahseBonusE
// DEMANGLED: ARAD::CeraShopPurcahseCountBonus::_processBonusItem(CUser *,int,ARAD::PurcahseBonus const&)
void CeraShopPurcahseCountBonus::_processBonusItem(
    CUser* user,
    int goods_no,
    const PurcahseBonus& bonus) {
    if (!user) {
        return;
    }

    if (bonus.bonus_item_index_ == 1) {
        CInventory* const inventory_read = user->getCurCharacInvenR();
        CInventory* const inventory_write = user->getCurCharacInvenW();
        if (!inventory_read || !inventory_write) {
            return;
        }

        inventory_write->SetPayCoin(
            static_cast<unsigned int>(inventory_read->GetPayCoin()) +
            bonus.bonus_item_count_);
        user->history_log().PayCoinAdd(
            user->getCurCharacInvenR()->GetPayCoin(),
            bonus.bonus_item_count_,
            static_cast<eCoinAddReason>(5));
        user->SaveMoneyCoin();
        user->SendCoin();
        return;
    }

    if (bonus.bonus_item_index_ == 5) {
        CCeraShopGoods* const goods = G_CDataManager()->FindGoods(goods_no);
        if (!goods) {
            return;
        }

        const int mileage = static_cast<int>(std::floor(
            static_cast<double>(goods->GetCeraPrice()) *
            (static_cast<double>(bonus.bonus_item_count_) / 1000.0)));
        user->AddMileage(mileage, static_cast<eCoinAddReason>(5));
        user->SendCashData(false);
        return;
    }

    CItem* const item = G_CDataManager()->find_item(bonus.bonus_item_index_);
    if (!item || !item->vtable_) {
        return;
    }

    Inven_Item bonus_item;
    item->vtable_->make_item(item, bonus_item);
    bonus_item.item_id_ = bonus.bonus_item_index_;
    bonus_item.add_info_ = bonus.bonus_item_count_;

    if (item->vtable_->is_avatar_item(item) ||
        item->vtable_->is_creature_item(item)) {
        return;
    }

    CInventory* const inventory = user->getCurCharacInvenW();
    if (!inventory) {
        return;
    }

    const int slot = inventory->insertItemIntoInventory(
        bonus_item, static_cast<eItemAddReason>(31), true, false);
    if (slot < 0) {
        const char* const mail_text =
            g_scriptStringManager_.findString(4, "game_server_msg_104", 0);
        WongWork::CMailBoxHelper::ReqDBSendNewMail(
            user,
            bonus_item,
            0u,
            static_cast<unsigned int>(user->getCurCharacNo()),
            true,
            mail_text,
            static_cast<int>(std::strlen(mail_text)));
        return;
    }

    user->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_ITEMSPACE>(0),
                             slot);
    user->SaveInventory();
}

}  // namespace ARAD

// IDA_EA: 0x8179EAE
// IDA_NAME: _ZN18ItemVendingMachine23LoadPurcahseBonusScriptEv
// DEMANGLED: ItemVendingMachine::LoadPurcahseBonusScript(void)
int ItemVendingMachine::LoadPurcahseBonusScript() {
    return purchase_count_bonus_.LoadScript();
}

// IDA_EA: 0x8179ED8
// IDA_NAME: _ZN18ItemVendingMachine22CheckPurcahseBonusItemEm
// DEMANGLED: ItemVendingMachine::CheckPurcahseBonusItem(unsigned long)
int ItemVendingMachine::CheckPurcahseBonusItem(unsigned long goods_no) {
    return purchase_count_bonus_.CheckBonusItem(static_cast<int>(goods_no));
}

// IDA_EA: 0x8179F08
// IDA_NAME: _ZN18ItemVendingMachine24ProcessPurcahseBonusItemEP5CUsermit
// DEMANGLED: ItemVendingMachine::ProcessPurcahseBonusItem(CUser *,unsigned long,int,unsigned short)
int ItemVendingMachine::ProcessPurcahseBonusItem(
    CUser* user,
    unsigned long purchase_index,
    int goods_no,
    unsigned short purchase_count) {
    return purchase_count_bonus_.GiveBonusItem(
        user, purchase_index, goods_no, purchase_count);
}
