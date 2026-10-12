#include "old_game/appearance/pet/inter_creature_item_returns.h"

#include "old_data/db/creature/compound_creature_marker.h"
#include "old_data/db/creature/creature_item_payload.h"
#include "old_game/appearance/pet/creature.h"
#include "old_game/appearance/pet/packet_dispatcher_compound_creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

namespace {

template <typename Dispatcher>
int DispatchCreatureItemResult(const Dispatcher& dispatcher,
                               CUser* user,
                               char* payload,
                               int mismatch_result) {
    SIG_CREATURE_ITEM* const signal =
        reinterpret_cast<SIG_CREATURE_ITEM*>(payload);
    if (!user->getCurCharacR() ||
        user->getCurCharacNo() != static_cast<int>(signal->detail.charac_no)) {
        return mismatch_result;
    }
    user_creature::CCreatureMgr* const creatures =
        user->getCurCharacInvenW()->GetCreatureMgrW();
    // IDA 0x84D027E / 0x84D02EE calls the creature-manager result
    // handler for its side effects, then returns zero to InterDispatcher.
    // The manager's one means that the creature operation succeeded; it is
    // not an inter-dispatch error code.
    dispatcher.Dispatch(creatures, signal);
    return 0;
}

struct InsertCreatureItemResultDispatcher {
    int Dispatch(user_creature::CCreatureMgr* creatures,
                 SIG_CREATURE_ITEM* signal) const {
        return creatures->InsertCreatureItemResult(signal);
    }
};

struct UpdateCreatureItemResultDispatcher {
    int Dispatch(user_creature::CCreatureMgr* creatures,
                 SIG_CREATURE_ITEM* signal) const {
        return creatures->UpdateCreatureItemResult(signal);
    }
};

}  // namespace

// IDA_EA: 0x84D027E
// IDA_NAME: _ZN30Inter_InsertCreatureItemResult12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_InsertCreatureItemResult::dispatch_sig(CUser *,char *,int)
int Inter_InsertCreatureItemResult::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int /*payload_size*/) {
    SIG_CREATURE_ITEM* const signal =
        reinterpret_cast<SIG_CREATURE_ITEM*>(payload);
    if (signal &&
        signal->update_kind == compound_creature::kInsertUpdateKind) {
        (void)HandleCompoundCreatureInsertResult(user, signal);
        return 0;
    }
    return DispatchCreatureItemResult(InsertCreatureItemResultDispatcher(),
                                      user,
                                      payload,
                                      11845);
}

// IDA_EA: 0x84ED674
// IDA_NAME: _ZN30Inter_InsertCreatureItemResultC2Ev
// DEMANGLED: Inter_InsertCreatureItemResult::Inter_InsertCreatureItemResult(void)
Inter_InsertCreatureItemResult::Inter_InsertCreatureItemResult()
    : ISigDispatcher() {}

// IDA_EA: 0x84F9BD0 / 0x84F9C00
// IDA_NAME: _ZN30Inter_InsertCreatureItemResultD2Ev / _ZN30Inter_InsertCreatureItemResultD0Ev
// DEMANGLED: Inter_InsertCreatureItemResult::~Inter_InsertCreatureItemResult()
Inter_InsertCreatureItemResult::~Inter_InsertCreatureItemResult() {}

// IDA_EA: 0x84D02EE
// IDA_NAME: _ZN30Inter_UpdateCreatureItemResult12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_UpdateCreatureItemResult::dispatch_sig(CUser *,char *,int)
int Inter_UpdateCreatureItemResult::dispatch_sig(CUser* user,
                                                 char* payload,
                                                 int /*payload_size*/) {
    return DispatchCreatureItemResult(UpdateCreatureItemResultDispatcher(),
                                      user,
                                      payload,
                                      11869);
}

// IDA_EA: 0x84ED690
// IDA_NAME: _ZN30Inter_UpdateCreatureItemResultC2Ev
// DEMANGLED: Inter_UpdateCreatureItemResult::Inter_UpdateCreatureItemResult(void)
Inter_UpdateCreatureItemResult::Inter_UpdateCreatureItemResult()
    : ISigDispatcher() {}

// IDA_EA: 0x84F9B82 / 0x84F9BB2
// IDA_NAME: _ZN30Inter_UpdateCreatureItemResultD2Ev / _ZN30Inter_UpdateCreatureItemResultD0Ev
// DEMANGLED: Inter_UpdateCreatureItemResult::~Inter_UpdateCreatureItemResult()
Inter_UpdateCreatureItemResult::~Inter_UpdateCreatureItemResult() {}
