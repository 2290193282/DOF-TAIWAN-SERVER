#ifndef OLDSRV_OLD_GAME_NPC_INTER_CERASHOP_PURCHASE_COUNT_RETURN_H_
#define OLDSRV_OLD_GAME_NPC_INTER_CERASHOP_PURCHASE_COUNT_RETURN_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// IDA unit: 07_npc_shop_dialog/Inter_CeraShopPurchaseCount.
// This is a base-only InterDispatcher receiver for the DB purchase-count
// response that drives the CeraShop cumulative-purchase bonus chain.
class Inter_CeraShopPurchaseCount : public ISigDispatcher {
public:
    // IDA_EA: 0x84EED6C
    Inter_CeraShopPurchaseCount();

    // IDA_EA: 0x84F5A9C / 0x84F5ACC
    virtual ~Inter_CeraShopPurchaseCount();

    // IDA_EA: 0x84E7BFE
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_CeraShopPurchaseCount) == 0x04u,
              "Inter_CeraShopPurchaseCount must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_INTER_CERASHOP_PURCHASE_COUNT_RETURN_H_
