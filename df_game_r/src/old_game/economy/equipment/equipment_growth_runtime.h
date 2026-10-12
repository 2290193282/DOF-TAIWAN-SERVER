#ifndef OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_EQUIPMENT_GROWTH_RUNTIME_H_
#define OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_EQUIPMENT_GROWTH_RUNTIME_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"


class CUser;
class PacketBuf;
struct Inven_Item;

namespace item_growth_db {
struct Message;
}

namespace equipment_growth_runtime {

// Returns true only when CMD169 is positively identified as the 0828
// equipment-growth request.  The byte-identical 0627 expert-job action 0/1
// frames fall through to their original owner.
bool TryHandleCommand(CUser* user, PacketBuf& packet);

// RECONSTRUCTED producer for PVF [exp acquire] "death".  The caller owns the
// battle eligibility checks; this function awards every equipped growth item
// and emits one exact NOTI477 per changed GUID.
void AwardDeathProgress(CUser* user,
                        int monster_kind,
                        int monster_level,
                        int dungeon_difficulty);

// CargoTransportStone client logic clears the maintained progress on a
// successful growth-equipment transfer.  This cache/DB helper is called only
// after the canonical inventory transaction commits.
bool ClearForCargoTransport(CUser* user, const Inven_Item& item);

// Completes only correlated stage-reset/compensation acknowledgements.  The
// local inventory mutation and its legacy save flag are deliberately held
// until the native SQL006 reset is durable.
void HandlePersistenceResult(CUser* user,
                             const item_growth_db::Message& message);

// Detaches any in-flight SQL006 transition from this CUser slot before the
// slot can be recycled.  A reset that may already have reached queue6 is
// followed by an exact compensating write on the same FIFO.
void ResetUserPersistence(CUser* user);

}  // namespace equipment_growth_runtime

class Inter_ItemGrowthState : public ISigDispatcher {
public:
    Inter_ItemGrowthState();
    virtual ~Inter_ItemGrowthState();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_ItemGrowthState) == 0x04u,
              "equipment-growth inter dispatcher must remain base-only");
#endif

#endif  // OLDSRV_OLD_GAME_ECONOMY_EQUIPMENT_EQUIPMENT_GROWTH_RUNTIME_H_
