#ifndef OLDSRV_OLD_GAME_EVENT_INTER_ADVANCE_ALTAR_LOAD_H_
#define OLDSRV_OLD_GAME_EVENT_INTER_ADVANCE_ALTAR_LOAD_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

namespace advancealtar {

// IDA unit: 99_misc_global/advancealtar/Inter_Load.
// DBTask outer signal 25 installs the persisted Advance Altar record into
// CUser::advance_altar_manager_ and releases CACHE_SECTION 10.
class Inter_Load : public ISigDispatcher {
public:
    Inter_Load();
    virtual ~Inter_Load();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_Load) == 0x04u,
              "advancealtar::Inter_Load must retain its base-only ABI");
#endif

}  // namespace advancealtar

#endif  // OLDSRV_OLD_GAME_EVENT_INTER_ADVANCE_ALTAR_LOAD_H_
