#ifndef OLDSRV_PVF_DATA_CASH_SHOP_GIFT_ONLY_0828_H_
#define OLDSRV_PVF_DATA_CASH_SHOP_GIFT_ONLY_0828_H_

// 0828 added the [gift only] cash-shop list without extending the old
// STCeraShopItem/CCeraShopGoods ABI. Keep the policy in a startup-owned
// sidecar keyed by the same goods number used by the neighbouring lists.
namespace cash_shop_0828 {

void clear_gift_only_goods();
void register_gift_only_goods(unsigned int goods_no);
bool is_gift_only_goods(unsigned int goods_no);

}  // namespace cash_shop_0828

#endif  // OLDSRV_PVF_DATA_CASH_SHOP_GIFT_ONLY_0828_H_
