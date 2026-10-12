#include "old_game/economy/trade/private_store_inter_load_runtime.h"

#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x84CF55A
// IDA_NAME: _ZN22Inter_LoadPrivateStore12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadPrivateStore::dispatch_sig(CUser *,char *,int)
int Inter_LoadPrivateStore::dispatch_sig(CUser* user,
                                         char* payload,
                                         int /*payload_size*/) {
    if (user->get_state() > 2) {
        private_store::CPrivateStoreMgr* const manager =
            private_store::GetInstancePrivateStoreMgr();
        manager->LoadPrivateStoreResponse(
            user,
            reinterpret_cast<private_store::SIG_PRIVATE_STORE*>(payload));
    }
    return 0;
}

// IDA_EA: 0x84ED5B0
// IDA_NAME: _ZN22Inter_LoadPrivateStoreC2Ev
// DEMANGLED: Inter_LoadPrivateStore::Inter_LoadPrivateStore(void)
Inter_LoadPrivateStore::Inter_LoadPrivateStore()
    : ISigDispatcher() {}

// IDA_EA: 0x84F9DF2
// IDA_NAME: _ZN22Inter_LoadPrivateStoreD2Ev
// DEMANGLED: Inter_LoadPrivateStore::~Inter_LoadPrivateStore()
// IDA_EA: 0x84F9E22
// IDA_NAME: _ZN22Inter_LoadPrivateStoreD0Ev
// DEMANGLED: Inter_LoadPrivateStore::~Inter_LoadPrivateStore()
Inter_LoadPrivateStore::~Inter_LoadPrivateStore() {}
