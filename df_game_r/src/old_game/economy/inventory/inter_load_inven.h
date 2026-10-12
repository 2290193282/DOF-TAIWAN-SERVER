#ifndef OLDSRV_OLD_GAME_INVENTORY_INTER_LOAD_INVEN_H_
#define OLDSRV_OLD_GAME_INVENTORY_INTER_LOAD_INVEN_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
struct SIG_LOAD_INVENTORY;

// IDA unit: 06_userinfo_stat_inventory/Inter_LoadInven.
// DB signal 8 installs the persisted inventory state, validates every named
// storage region, and releases the character inventory/cache path.
class Inter_LoadInven : public ISigDispatcher {
public:
    Inter_LoadInven();
    virtual ~Inter_LoadInven();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);

private:
    void _UpdateRentalItems(CUser* user, SIG_LOAD_INVENTORY* payload);
    void _UpdateDimensionRentalItems(CUser* user,
                                     SIG_LOAD_INVENTORY* payload);
    bool _CheckUsableExpertJobEquipment(CUser* user);
    char _CheckOverEquipItem(CUser* user);
};

#if defined(__i386__)
static_assert(sizeof(Inter_LoadInven) == 0x04u,
              "Inter_LoadInven must retain its base-only IDA ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_INTER_LOAD_INVEN_H_
