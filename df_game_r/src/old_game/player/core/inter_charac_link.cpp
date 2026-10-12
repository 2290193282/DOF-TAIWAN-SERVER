#include "old_game/player/core/inter_charac_link.h"

#include "old_data/db/character/db_charac_link.h"
#include "old_game/player/core/cuser.h"


Inter_CharacLink::Inter_CharacLink() : ISigDispatcher() {}

Inter_CharacLink::~Inter_CharacLink() {}

int Inter_CharacLink::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    if (!user || !payload ||
        payload_size !=
            static_cast<int>(sizeof(charac_link_db::Message))) {
        return 0;
    }

    const charac_link_db::Message* const response =
        reinterpret_cast<const charac_link_db::Message*>(payload);
    user->ApplyCharacLinkDatabaseResult(*response);
    // A malformed/stale feature reply is fail-closed locally.  Returning a
    // nonzero value here would turn an optional CMD309/310 failure into the
    // root InterDispatcher's fatal network disconnect contract.
    return 0;
}
