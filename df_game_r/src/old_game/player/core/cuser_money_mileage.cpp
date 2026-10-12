#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "old_data/db/auction_cashshop/db_mileage.h"
#include "old_data/db/inventory/db_save_inven.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>

// IDA_EA: 0x86504D8
// IDA_NAME: _ZN5CUser13SaveMoneyCoinEv
// DEMANGLED: CUser::SaveMoneyCoin(void)
int CUser::SaveMoneyCoin() {
    if (G_GameWorld()->GetChannelType() == 7) {
        return 0;
    }

    CStreamGuard guard(
        GlobalData::big_stream_pool()->Acquire("user.cpp", 6348), true);
    *guard << 35;
    *guard << this->GetUID();
    SIG_SAVE_INVENTORY* const payload_record =
        guard.GetInBuffer<SIG_SAVE_INVENTORY>();
    _GetSaveInvenBase(payload_record);
    payload_record->save_mode = 1;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}

// IDA_EA: 0x86505F6
// IDA_NAME: _ZN5CUser11SaveMileageEv
// DEMANGLED: CUser::SaveMileage(void)
void CUser::SaveMileage() {
    if (G_GameWorld()->GetChannelType() == 7) {
        return;
    }

    CStreamGuard guard(GlobalData::stream_pool()->Acquire("user.cpp", 6372),
                       true);
    *guard << 508;
    *guard << this->GetUID();
    SIG_MILEAGE_STRUCT* const payload =
        guard.GetInBuffer<SIG_MILEAGE_STRUCT>();
    payload->m_id = static_cast<uint32_t>(get_acc_id());
    payload->charac_no = static_cast<uint32_t>(getCurCharacNo());
    payload->miles = GetMileage();
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8650722
// IDA_NAME: _ZN5CUser11LoadMileageEv
// DEMANGLED: CUser::LoadMileage(void)
void CUser::LoadMileage() {
    CStreamGuard guard(GlobalData::stream_pool()->Acquire("user.cpp", 6386),
                       true);
    *guard << 509;
    *guard << this->GetUID();
    SIG_MILEAGE_STRUCT* const payload =
        guard.GetInBuffer<SIG_MILEAGE_STRUCT>();
    payload->m_id = static_cast<uint32_t>(get_acc_id());
    payload->charac_no = static_cast<uint32_t>(getCurCharacNo());
    payload->miles = 0;
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x865082A
// IDA_NAME: _ZN5CUser12SendCashDataEb
// DEMANGLED: CUser::SendCashData(bool)
void CUser::SendCashData(bool clear_values) {
    PacketGuard packet;
    packet.put_header(0, 53);
    packet.put_byte(1);
    if (clear_values) {
        packet.put_int(0);
        packet.put_int(0);
        packet.put_int(0);
    } else {
        packet.put_int(GetCera());
        packet.put_int(GetMileage());
        packet.put_int(GetCeraPoint());
    }
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8650954
// IDA_NAME: _ZN5CUser10AddMileageEi14eCoinAddReason
// DEMANGLED: CUser::AddMileage(int,eCoinAddReason)
void CUser::AddMileage(int amount, eCoinAddReason reason) {
    LogHistory("Mileage %s,%d,%d,%d,%d",
               "Add",
               amount,
               mileage_,
               amount + mileage_,
               reason);
    mileage_ += amount;
    SaveMileage();
}

// IDA_EA: 0x86509C6
// IDA_NAME: _ZN5CUser10SetMileageEi
// DEMANGLED: CUser::SetMileage(int)
CUser* CUser::SetMileage(int mileage) {
    LogHistory("Mileage %s,%d,%d,%d",
               "Set",
               mileage,
               mileage_,
               mileage);
    mileage_ = mileage;
    return this;
}

// IDA_EA: 0x8650A10
// IDA_NAME: _ZN5CUser10UseMileageEi
// DEMANGLED: CUser::UseMileage(int)
void CUser::UseMileage(int amount) {
    const int result = mileage_ - amount;
    LogHistory("Mileage %s,%d,%d,%d",
               "Use",
               amount,
               mileage_,
               result & ~(result >> 31));
    mileage_ = result >= 0 ? result : 0;
    SaveMileage();
}

// IDA_EA: 0x8650A94
// IDA_NAME: _ZN5CUser10GetMileageEv
// DEMANGLED: CUser::GetMileage(void)
int CUser::GetMileage() {
    return mileage_;
}
