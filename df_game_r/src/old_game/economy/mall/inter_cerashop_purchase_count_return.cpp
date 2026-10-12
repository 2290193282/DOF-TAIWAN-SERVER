#include "old_game/economy/mall/inter_cerashop_purchase_count_return.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "old_data/db/auction_cashshop/cera_shop_purchase_count_payload.h"
#include "old_game/economy/shop/npc_runtime.h"

// IDA_EA: 0x84E7BFE
// IDA_NAME: _ZN27Inter_CeraShopPurchaseCount12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CeraShopPurchaseCount::dispatch_sig(CUser *,char *,int)
int Inter_CeraShopPurchaseCount::dispatch_sig(CUser* user,
                                              char* payload,
                                              int payload_size) {
    if (!payload) {
        return 27287;
    }
    if (!user ||
        payload_size < static_cast<int>(sizeof(SIG_CERASHOP_PURCHASE_COUNT)) ||
        !GlobalData::s_pItemVendingMachine) {
        return 0;
    }

    SIG_CERASHOP_PURCHASE_COUNT response;
    std::memcpy(&response, payload, sizeof(response));
    GlobalData::s_pItemVendingMachine->ProcessPurcahseBonusItem(
        user,
        static_cast<unsigned long>(response.item_index),
        response.server_id,
        response.purchase_count);
    return 0;
}

// IDA_EA: 0x84EED6C
// IDA_NAME: _ZN27Inter_CeraShopPurchaseCountC2Ev
Inter_CeraShopPurchaseCount::Inter_CeraShopPurchaseCount()
    : ISigDispatcher() {}

// IDA_EA: 0x84F5A9C / 0x84F5ACC
Inter_CeraShopPurchaseCount::~Inter_CeraShopPurchaseCount() {}
