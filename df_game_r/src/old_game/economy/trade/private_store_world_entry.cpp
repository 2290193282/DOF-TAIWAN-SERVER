#include "old_game/economy/mail/mail_trade_runtime.h"

#include "common/manager/global_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x85CAA8A
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr5EnterEP5CUser
// DEMANGLED: private_store::CPrivateStoreMgr::Enter(CUser *)
int private_store::CPrivateStoreMgr::Enter(CUser* user) {
    LoadPrivateStore(user);
    return 1;
}

// IDA_EA: 0x85CAB24
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr16LoadPrivateStoreEP5CUser
// DEMANGLED: private_store::CPrivateStoreMgr::LoadPrivateStore(CUser *)
void private_store::CPrivateStoreMgr::LoadPrivateStore(CUser* user) {
    CStreamGuard guard(
        GlobalData::stream_pool()->Acquire("PrivateStore.cpp", 3257), true);
    *guard << 120;
    *guard << user->GetUID();
    *guard << user->get_charac_no(-1);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x85CAC16
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr24LoadPrivateStoreResponseEP5CUserPNS_17SIG_PRIVATE_STOREE
// DEMANGLED: private_store::CPrivateStoreMgr::LoadPrivateStoreResponse(CUser *,private_store::SIG_PRIVATE_STORE *)
void private_store::CPrivateStoreMgr::LoadPrivateStoreResponse(
    CUser* user,
    SIG_PRIVATE_STORE* payload) {
    if (payload->use_doll == 1) {
        ISeller* seller_interface = 0;
        GetStoreSeller(&seller_interface, user);

        // ISeller is CPrivateStore's primary, zero-offset base.  The original
        // calls the concrete nonvirtual recovery/persistence sequence here.
        CPrivateStore* const store =
            static_cast<CPrivateStore*>(seller_interface);
        store->ResetItems();
        store->CopyItems(payload->item, 14);
        store->RecallPrivateStoreItemToMail();
        store->SavePrivateStore();
        store->Clear();
        FreeStoreSeller(user);
    } else if (user->IsPermissionPrivateStore()) {
        CPacketBufPrivate packet;
        const int item_count = packet.MakeNotipacketPrivateStoreInfo(payload);
        user->Send(packet);
        if (item_count) {
            ISeller* seller_interface = 0;
            GetStoreSeller(&seller_interface, user);
            seller_interface->RecreatePrivateStore(payload);
        }
    }
}
