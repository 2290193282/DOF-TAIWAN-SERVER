#include "old_game/economy/mail/mail_trade_runtime.h"

#include <algorithm>
#include <climits>

#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

namespace private_store {

// IDA_EA: 0x85CD22A
// IDA_NAME: _ZN13private_store9COperatorIiE7ProductEiiRi
// DEMANGLED: private_store::COperator<int>::Product(int,int,int &)
int COperator<int>::Product(int left, int right, int& result) {
    result = 0;
    for (int index = 0; index < right; ++index) {
        result += left;
        if (result <= 0) return 0;
    }
    return 1;
}

// IDA_EA: 0x85C76F0
// IDA_NAME: _ZN13private_store13CPrivateStore19CheckTotalItemPriceEv
// DEMANGLED: private_store::CPrivateStore::CheckTotalItemPrice(void)
int CPrivateStore::CheckTotalItemPrice() {
    int total_price = 0;
    for (int slot = 0; slot <= 13; ++slot) {
        PrivateStoreItem& store_item = items[slot];
        if (store_item.state != 2) continue;
        if (!store_item.price) return 0;

        CItem* const item = G_CDataManager()->find_item(store_item.item.getKey());
        if (!item) return 0;
        if (item->is_stackable()) {
            if (!COperator<int>::Product(
                    store_item.price, store_item.item.get_add_info(),
                    total_price)) {
                return 0;
            }
        } else {
            const long long next =
                static_cast<long long>(total_price) + store_item.price;
            if (next > INT_MAX || next <= 0) return 0;
            total_price = static_cast<int>(next);
        }
    }
    return total_price > 0;
}

// IDA_EA: 0x85C7ADC
// IDA_NAME: _ZN13private_store13CPrivateStore9SendUsersEP11PacketGuardP5CUserS4_
// DEMANGLED: private_store::CPrivateStore::SendUsers(PacketGuard *,CUser *,CUser *)
int CPrivateStore::SendUsers(PacketGuard* packet,
                             CUser* excluded_user,
                             CUser* extra_user) {
    int sent = 0;
    for (std::list<CUser*>::iterator it = buyers.begin(); it != buyers.end();
         ++it) {
        CUser* const user = *it;
        if (user && user != excluded_user) {
            user->Send(*packet);
            ++sent;
        }
    }
    if (extra_user) {
        extra_user->Send(*packet);
        ++sent;
    }
    return sent;
}

// IDA_EA: 0x85C7BBC
// IDA_NAME: _ZN13private_store13CPrivateStore17SubtractItemCountEPK5CItemRNS_16PrivateStoreItemEi
// DEMANGLED: private_store::CPrivateStore::SubtractItemCount(CItem const*,private_store::PrivateStoreItem &,int)
PrivateStoreItem* CPrivateStore::SubtractItemCount(const CItem* item,
                                                   PrivateStoreItem& store_item,
                                                   int count) {
    if (!item->is_stackable() || store_item.item.get_add_info() == count) {
        return store_item.Reset();
    }
    store_item.item.set_add_info(store_item.item.get_add_info() - count);
    return &store_item;
}

// IDA_EA: 0x85C7C0A
// IDA_NAME: _ZN13private_store13CPrivateStore14IsPossibleDealEijj
// DEMANGLED: private_store::CPrivateStore::IsPossibleDeal(int,unsigned int,unsigned int)
int CPrivateStore::IsPossibleDeal(int money) {
    if (seller->CheckMoney(money) == 1) {
        if (buyer->getCurCharacInvenR()->get_money() >= money) return 1;
        error_handler.Do(10);
        return 0;
    }

    error_handler.Do(22);
    CPacketBufPrivate packet;
    packet.MakeErrorReturnPacket(0, 82, 22);
    seller->Send(packet);
    return 0;
}

// IDA_EA: 0x85C7D04
// IDA_NAME: _ZN13private_store13CPrivateStore12GetItemPriceEPK5CItemRNS_16PrivateStoreItemEiRiRjS7_
// DEMANGLED: private_store::CPrivateStore::GetItemPrice(CItem const*,private_store::PrivateStoreItem &,int,int &,unsigned int &,unsigned int &)
int CPrivateStore::GetItemPrice(const CItem* item,
                                PrivateStoreItem& store_item,
                                int count,
                                int& money,
                                unsigned int& first_value,
                                unsigned int& second_value) {
    money = store_item.price;
    first_value = 0;
    second_value = 0;

    if (item->is_stackable()) {
        if (count <= 0) {
            error_handler.Do(1);
            return 0;
        }
        if (store_item.item.get_add_info() < count) {
            error_handler.Do(17);
            return 0;
        }
        if (!COperator<int>::Product(money, count, money)) {
            error_handler.Do(1);
            return 0;
        }
        first_value *= static_cast<unsigned int>(count);
        second_value *= static_cast<unsigned int>(count);
    } else if (item->vtable_->is_avatar_item(item) != 1 &&
               store_item.item.get_add_info() != count) {
        error_handler.Do(1);
        return 0;
    }

    if (money <= 0) {
        error_handler.Do(1);
        return 0;
    }
    return 1;
}

// IDA_EA: 0x85C91B6
// IDA_NAME: _ZN13private_store13CPrivateStore16IsExistBuyerListEP5CUser
// DEMANGLED: private_store::CPrivateStore::IsExistBuyerList(CUser *)
bool CPrivateStore::IsExistBuyerList(CUser* user) {
    return std::find(buyers.begin(), buyers.end(), user) != buyers.end();
}

// IDA_EA: 0x85C9B8C
// IDA_NAME: _ZN13private_store13CPrivateStore20GetCommissionPercentEv
// DEMANGLED: private_store::CPrivateStore::GetCommissionPercent(void)
int CPrivateStore::GetCommissionPercent() {
    return G_GameWorld()->GetChannelType() == 3 ? 3 : 4;
}

}  // namespace private_store
