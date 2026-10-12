#include "old_game/economy/mall/npc_shop_timer_runtime.h"

#include "common/manager/global_data.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/itemshop/item_shop_script.h"
#include "old_game/economy/mall/secret_shop_runtime.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x863CEAA
// IDA_NAME: _ZN25Timer_SecretShopStatisticC2Ev
// DEMANGLED: Timer_SecretShopStatistic::Timer_SecretShopStatistic(void)
Timer_SecretShopStatistic::Timer_SecretShopStatistic() : TimerSig() {}

// IDA_EA: 0x863EB8C / 0x863EBBC
// IDA_NAME: _ZN25Timer_SecretShopStatisticD2Ev / D0Ev
// DEMANGLED: Timer_SecretShopStatistic::~Timer_SecretShopStatistic()
Timer_SecretShopStatistic::~Timer_SecretShopStatistic() {}

// IDA_EA: 0x86397CE
// IDA_NAME: _ZN25Timer_SecretShopStatistic12dispatch_sigEiij
// DEMANGLED: Timer_SecretShopStatistic::dispatch_sig(int,int,unsigned int)
int Timer_SecretShopStatistic::dispatch_sig(int /*object_id*/,
                                            int /*param1*/,
                                            unsigned int /*param2*/) {
    GlobalData::s_secret_shop->SendSecretShopStatistic();
    RegistNextTimer();
    return 1;
}

// IDA_EA: 0x86397EE
// IDA_NAME: _ZN25Timer_SecretShopStatistic15RegistNextTimerEv
// DEMANGLED: Timer_SecretShopStatistic::RegistNextTimer(void)
int Timer_SecretShopStatistic::RegistNextTimer() {
    return G_TimerQueue()->InsertTimer(2, 0, 144, 60, 0, 0);
}

// IDA_EA: 0x863CFDE
// IDA_NAME: _ZN21Timer_OneADayItemShopC2Ev
// DEMANGLED: Timer_OneADayItemShop::Timer_OneADayItemShop(void)
Timer_OneADayItemShop::Timer_OneADayItemShop() : TimerSig() {}

// IDA_EA: 0x863E832 / 0x863E862
// IDA_NAME: _ZN21Timer_OneADayItemShopD2Ev / D0Ev
// DEMANGLED: Timer_OneADayItemShop::~Timer_OneADayItemShop()
Timer_OneADayItemShop::~Timer_OneADayItemShop() {}

// IDA_EA: 0x863A916
// IDA_NAME: _ZN21Timer_OneADayItemShop12dispatch_sigEiij
// DEMANGLED: Timer_OneADayItemShop::dispatch_sig(int,int,unsigned int)
int Timer_OneADayItemShop::dispatch_sig(int /*object_id*/,
                                        int /*param1*/,
                                        unsigned int /*param2*/) {
    G_CDataManager()->get_item_shop()->updateOneADayItemList();
    RegistNextTimer();
    return 1;
}

// IDA_EA: 0x863A93C
// IDA_NAME: _ZN21Timer_OneADayItemShop15RegistNextTimerEv
// DEMANGLED: Timer_OneADayItemShop::RegistNextTimer(void)
int Timer_OneADayItemShop::RegistNextTimer() {
    return G_TimerQueue()->InsertTimer(2, 0, 153, 60, 0, 0);
}
