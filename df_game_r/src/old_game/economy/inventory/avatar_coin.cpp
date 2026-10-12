#include "old_game/economy/inventory/avatar_coin.h"

#include "old_data/PVF/data/datamanager/avatar_roulette_server.h"
#include "old_data/db/inventory/db_save_inven.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


typedef char AvatarCoin_Size_Must_Be_0x04[
    (sizeof(AvatarCoin) == 0x04) ? 1 : -1];

// IDA_EA: 0x817FED4
// IDA_NAME: _ZN10AvatarCoinC2Ev
// DEMANGLED: AvatarCoin::AvatarCoin(void)
AvatarCoin::AvatarCoin()
    : coin_count(0) {}

// IDA_EA: 0x817FEE2
// IDA_NAME: _ZN10AvatarCoin3SetEj
// DEMANGLED: AvatarCoin::Set(unsigned int)
AvatarCoin* AvatarCoin::Set(uint32_t coin_count_value) {
    coin_count = coin_count_value;
    return this;
}

// IDA_EA: 0x817FEF0
// IDA_NAME: _ZNK10AvatarCoin3GetEv
// DEMANGLED: AvatarCoin::Get(void)const
int AvatarCoin::Get() const {
    return static_cast<int>(coin_count);
}

// IDA_EA: 0x817FEFA
// IDA_NAME: _ZN10AvatarCoin3AddEj
// DEMANGLED: AvatarCoin::Add(unsigned int)
int AvatarCoin::Add(uint32_t coin_count_value) {
    coin_count += coin_count_value;
    return 1;
}

// IDA_EA: 0x817FF14
// IDA_NAME: _ZN10AvatarCoin3UseEv
// DEMANGLED: AvatarCoin::Use(void)
int AvatarCoin::Use() {
    if (!IsUsable()) {
        return 0;
    }

    --coin_count;
    return 1;
}

// IDA_EA: 0x817FF44
// IDA_NAME: _ZNK10AvatarCoin8IsUsableEv
// DEMANGLED: AvatarCoin::IsUsable(void)const
bool AvatarCoin::IsUsable() const {
    return coin_count != 0;
}

// IDA_EA: 0x81800D6
// IDA_NAME: _ZN10AvatarCoin8SaveToDBEP5CUser
// DEMANGLED: AvatarCoin::SaveToDB(CUser *)
int AvatarCoin::SaveToDB(CUser* user) {
    CStreamGuard guard(GlobalData::big_stream_pool()->Acquire(
                           "localjapan/Arad_AvatarRoulette.cpp", 388),
                       true);
    *guard << 35;
    *guard << user->GetUID();

    SIG_SAVE_INVENTORY* const payload =
        guard.GetInBuffer<SIG_SAVE_INVENTORY>();
    user->_GetSaveInvenBase(payload);
    payload->save_mode = 1;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x81801D0
// IDA_NAME: _ZN10AvatarCoin10BuyProcessEP5CUserR10Inven_Item
// DEMANGLED: AvatarCoin::BuyProcess(CUser *,Inven_Item &)
int AvatarCoin::BuyProcess(CUser* user, Inven_Item& item) {
    if (!GlobalData::s_pAvatarRoulette->isAvatarPottery(
            static_cast<unsigned long>(item.item_id_))) {
        return 0;
    }

    const unsigned int amount = static_cast<unsigned int>(item.add_info_);
    user->getCurCharacInvenW()->avatar_coin_.Add(amount);
    if (SaveToDB(user) != 1) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_AvatarRoulette.cpp",
            "static bool AvatarCoin::BuyProcess(CUser*, Inven_Item&)",
            413,
            "SaveAvatarCoin failed");
    }
    HistoryLog::AddLog(user, amount);
    SendSyncPacket(user);
    return 1;
}

// IDA_EA: 0x818028A
// IDA_NAME: _ZN10AvatarCoin10GetPackageEP5CUserib
// DEMANGLED: AvatarCoin::GetPackage(CUser *,int,bool)
int AvatarCoin::GetPackage(CUser* user, int coin_count, bool is_received) {
    if (is_received) {
        return 1;
    }

    user->getCurCharacInvenW()->avatar_coin_.Add(
        static_cast<unsigned int>(coin_count));
    if (SaveToDB(user) != 1) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_AvatarRoulette.cpp",
            "static bool AvatarCoin::GetPackage(CUser*, int, bool)",
            432,
            "SaveAvatarCoin failed");
    }
    HistoryLog::AddLog(user, static_cast<unsigned int>(coin_count));
    SendSyncPacket(user);
    return 1;
}
