#include "old_game/economy/mail/mail_trade_runtime.h"

#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

namespace private_store {

// IDA_EA: 0x85CB93C
// IDA_NAME: _ZN13private_store8IUserPosC2Ev
// DEMANGLED: private_store::IUserPos::IUserPos(void)
IUserPos::IUserPos() {}

// IDA_EA: 0x85CA3D6
// IDA_NAME: _ZN13private_store16CPrivateStoreMgrC2Ev
CPrivateStoreMgr::CPrivateStoreMgr()
    : IUserPos(),
      fallback_store(),
      store_pool(),
      sellers_098(),
      buyers_0b0(),
      zones_0c8() {
    // These clears are explicit stores in the old C2 after all map C2 calls.
    sellers_098.clear();
    buyers_0b0.clear();
    zones_0c8.clear();
}

// IDA_EA: 0x85CA4FE
// IDA_NAME: _ZN13private_store16CPrivateStoreMgrD2Ev
CPrivateStoreMgr::~CPrivateStoreMgr() {
    for (std::map<PrivateStoreZone,
                  std::list<CPrivateStore*>*>::iterator it = zones_0c8.begin();
         it != zones_0c8.end();
         ++it) {
        delete it->second;
    }
}

// IDA_EA: 0x85C4BE8
// IDA_NAME: _ZN13private_store26GetInstancePrivateStoreMgrEv
// DEMANGLED: private_store::GetInstancePrivateStoreMgr(void)
CPrivateStoreMgr* GetInstancePrivateStoreMgr() {
    static CPrivateStoreMgr instance;
    return &instance;
}

// IDA_EA: 0x85CA2DA
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr14FindStoreBuyerEP5CUser
// DEMANGLED: private_store::CPrivateStoreMgr::FindStoreBuyer(CUser *)
CPrivateStore* CPrivateStoreMgr::FindStoreBuyer(CUser* user) {
    if (!user) {
        return 0;
    }
    std::map<CUser*, CPrivateStore*>::iterator it = buyers_0b0.find(user);
    if (it == buyers_0b0.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x85CA358
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr15FindStoreSellerEP5CUser
// DEMANGLED: private_store::CPrivateStoreMgr::FindStoreSeller(CUser *)
CPrivateStore* CPrivateStoreMgr::FindStoreSeller(CUser* user) {
    if (!user) {
        return 0;
    }
    std::map<CUser*, CPrivateStore*>::iterator it = sellers_098.find(user);
    if (it == sellers_098.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x85CA874
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr15FreeStoreSellerEP5CUser
// DEMANGLED: private_store::CPrivateStoreMgr::FreeStoreSeller(CUser *)
int CPrivateStoreMgr::FreeStoreSeller(CUser* user) {
    std::map<CUser*, CPrivateStore*>::iterator it = sellers_098.find(user);
    if (it == sellers_098.end()) {
        return 0;
    }
    store_pool.Free(it->second);
    sellers_098.erase(it);
    return 1;
}

// IDA_EA: 0x85CA914
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr14FreeStoreBuyerEP5CUser
// DEMANGLED: private_store::CPrivateStoreMgr::FreeStoreBuyer(CUser *)
int CPrivateStoreMgr::FreeStoreBuyer(CUser* user) {
    std::map<CUser*, CPrivateStore*>::iterator it = buyers_0b0.find(user);
    if (it == buyers_0b0.end()) {
        return 0;
    }
    buyers_0b0.erase(it);
    return 1;
}

// IDA_EA: 0x85CA698
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr14GetStoreSellerEPPNS_7ISellerEP5CUser
int CPrivateStoreMgr::GetStoreSeller(ISeller** seller, CUser* user) {
    CPrivateStore* store = FindStoreSeller(user);
    if (!store) {
        store = store_pool.Acquire();
        if (!store) {
            fallback_store.SetSeller(user);
            *seller = static_cast<ISeller*>(&fallback_store);
            return 0;
        }
        store->Clear();
        store->SetParent(this);
        store->SetSeller(user);
        sellers_098[user] = store;
    }
    *seller = static_cast<ISeller*>(store);
    return 1;
}

// IDA_EA: 0x85CA760
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr13GetStoreBuyerEPPNS_6IBuyerEP5CUsert
int CPrivateStoreMgr::GetStoreBuyer(IBuyer** buyer,
                                    CUser* user,
                                    unsigned short seller_index) {
    return GetStoreBuyer(buyer,
                         user,
                         G_GameWorld()->find_from_world(seller_index));
}

// IDA_EA: 0x85CA7B8
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr13GetStoreBuyerEPPNS_6IBuyerEP5CUserS5_
int CPrivateStoreMgr::GetStoreBuyer(IBuyer** buyer,
                                    CUser* user,
                                    CUser* seller) {
    CPrivateStore* store = FindStoreBuyer(user);
    if (!store) {
        store = FindStoreSeller(seller);
        if (!store) {
            fallback_store.SetBuyer(user);
            *buyer = static_cast<IBuyer*>(&fallback_store);
            return 0;
        }
        buyers_0b0[user] = store;
    }
    *buyer = static_cast<IBuyer*>(store);
    store->SetBuyer(user);
    return 1;
}

// IDA_EA: 0x85CA994
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr4ExitEP5CUser
// DEMANGLED: private_store::CPrivateStoreMgr::Exit(CUser *)
int CPrivateStoreMgr::Exit(CUser* user) {
    if (!user) {
        return 0;
    }

    CPrivateStore* buyer_store = FindStoreBuyer(user);
    if (buyer_store) {
        IBuyer* buyer_iface = 0;
        GetStoreBuyer(&buyer_iface, user, static_cast<CUser*>(0));
        buyer_iface->ExitPrivateStore();
        FreeStoreBuyer(user);
    }

    CPrivateStore* seller_store = FindStoreSeller(user);
    if (seller_store) {
        ISeller* seller_iface = 0;
        GetStoreSeller(&seller_iface, user);
        seller_iface->RemovePrivateStore(true);
        FreeStoreSeller(user);
    }
    return 1;
}

// IDA_EA: 0x85CAAAA
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr11HandleErrorEP5CUseriii
int CPrivateStoreMgr::HandleError(CUser* user,
                                  int packet_class,
                                  int packet_id,
                                  int error_code) {
    CPacketBufPrivate packet;
    packet.MakeErrorReturnPacket(packet_class, packet_id, error_code);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x85CAD5E
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr18IsBusyPrivateStoreEP5CUser
// DEMANGLED: private_store::CPrivateStoreMgr::IsBusyPrivateStore(CUser *)
int CPrivateStoreMgr::IsBusyPrivateStore(CUser* user) {
    CPrivateStoreMgr* instance = GetInstancePrivateStoreMgr();
    CPrivateStore* store = instance->FindStoreSeller(user);
    if (!store) {
        return 0;
    }

    const int state = store->GetState();
    if (state != 1 && state != 2) {
        return 0;
    }
    if (store->isDollCreated()) {
        return store->GetState() == 1;
    }
    return 1;
}

// IDA_EA: 0x86D23A4
// IDA_NAME: _ZN13private_store16CPrivateStoreMgr19GetPrivateStoreSizeEv
// DEMANGLED: private_store::CPrivateStoreMgr::GetPrivateStoreSize(void)
int CPrivateStoreMgr::GetPrivateStoreSize() {
    return static_cast<int>(sellers_098.size());
}

}  // namespace private_store
