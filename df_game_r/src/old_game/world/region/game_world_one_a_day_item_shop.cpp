#include "old_game/world/region/game_world.h"

#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/itemshop/item_shop_script.h"
#include "old_game/player/core/cuser.h"

namespace {

// 0x86CF858 walks GameWorld's canonical +0x134 user map.  This functor is
// passed to the existing typed ProcessAllUsers projection so it retains that
// original iteration order without introducing a second raw map facade.
class SendOneADayItemShopIndex {
public:
    explicit SendOneADayItemShopIndex(CItemShop* item_shop)
        : item_shop_(item_shop) {}

    void operator()(CUser* user) const {
        if (user->get_state() > 2) {
            user->SendOneADayItemShopIndex(item_shop_->GetOneADayItemList());
        }
    }

private:
    CItemShop* item_shop_;
};

}  // namespace

// IDA_EA: 0x86CF7F8
// IDA_NAME: _ZN9GameWorld20ResetOneADayItemShopEv
// DEMANGLED: GameWorld::ResetOneADayItemShop(void)
char GameWorld::ResetOneADayItemShop() {
    CEventBase* const event =
        GlobalData::s_event_manager->GetRepeatEvent(109);
    const char result = static_cast<char>(event->IsEventing(0) ^ 1);
    if (!result) {
        CItemShop* const item_shop = G_CDataManager()->get_item_shop();
        item_shop->updateOneADayItemList();
        ProcessAllUsers(SendOneADayItemShopIndex(item_shop));
    }
    return result;
}

// IDA_EA: 0x86CF904
// IDA_NAME: _ZN9GameWorld24testResetOneADayItemShopEv
// DEMANGLED: GameWorld::testResetOneADayItemShop(void)
char GameWorld::testResetOneADayItemShop() {
    CEventBase* const event =
        GlobalData::s_event_manager->GetRepeatEvent(109);
    const char result = static_cast<char>(event->IsEventing(0) ^ 1);
    if (!result) {
        CItemShop* const item_shop = G_CDataManager()->get_item_shop();
        item_shop->testupdateOneADayItemList();
        ProcessAllUsers(SendOneADayItemShopIndex(item_shop));
    }
    return result;
}
