#include "old_game/economy/mail/mail_trade_runtime.h"

#include <algorithm>
#include <cstring>

#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace private_store {

// IDA_EA: 0x85CA116
// IDA_NAME: _ZN13private_store13CPrivateStore16SavePrivateStoreEv
// DEMANGLED: private_store::CPrivateStore::SavePrivateStore(void)
void CPrivateStore::SavePrivateStore() {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("PrivateStore.cpp", 3014), true);
    guard << 119;
    guard << seller->GetUID();

    SIG_PRIVATE_STORE* const payload = guard.GetInBuffer<SIG_PRIVATE_STORE>();
    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no = seller->get_charac_no(-1);
    std::copy(items.begin(), items.begin() + 14, payload->item);
    payload->use_doll = isDollCreated() ? 1 : 0;

    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x85C7800
// IDA_NAME: _ZN13private_store13CPrivateStore15CompleteDisplayEPKct
// DEMANGLED: private_store::CPrivateStore::CompleteDisplay(char const*,unsigned short)
int CPrivateStore::CompleteDisplay(const char* display_name, uint16_t doll_slot) {
    error_handler.Set(seller, 1, ENUM_CMDPACKET_COMPLETE_DISPLAY);
    if (!seller) return 0;
    if (!manager) {
        error_handler.Do(58);
        return 0;
    }
    if (MoveItems() != 1) return 0;

    SavePrivateStore();
    seller->UpdateData();
    seller->send_itemspace(0);
    seller->send_itemspace(1);
    seller->getCurCharacInvenR()->SendItemLockListInven();
    seller->getCurCharacInvenR()->SendItemLockListAvatar();

    if (isDollCreated() != 1 && doll_slot != 0xffffu) return 0;
    if (isDollCreated()) {
        const int use_result =
            seller->getCurCharacInvenW()->use_item(doll_slot, 1, 1);
        seller->SendUpdateItem(static_cast<CUser::eSendTarget>(1),
                               static_cast<ENUM_ITEMSPACE>(0), doll_slot);
        if (use_result) {
            cMyTrace trace(
                "virtual bool private_store::CPrivateStore::CompleteDisplay(const char*, short unsigned int)",
                1729,
                5);
            trace("CPrivateStore::CompleteDisplay() ERROR!!! : use_item(%d)",
                  doll_slot);
            return 0;
        }
    }

    subject = display_name;
    CPacketBufPrivate packet;
    packet.MakeNotipacketCompleteDisplay(GetSellerIndex(), subject,
                                         isDollCreated() != 0);
    CBroadcastPrivate::Send(position.zone.village, position.zone.area, packet);
    packet.MakeCmdpacketCompleteDisplay(isDollCreated() != 0);
    seller->Send(packet);
    state = 2;
    return 1;
}

}  // namespace private_store
