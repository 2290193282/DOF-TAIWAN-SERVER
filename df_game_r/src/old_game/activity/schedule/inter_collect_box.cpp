#include "old_game/activity/schedule/inter_collect_box.h"

#include "old_data/db/event/db_collect_box.h"
#include "old_game/activity/schedule/collect_box_dispatcher.h"


Inter_CollectBox::Inter_CollectBox() : ISigDispatcher() {}

Inter_CollectBox::~Inter_CollectBox() {}

int Inter_CollectBox::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    if (!user || !payload ||
        payload_size !=
            static_cast<int>(sizeof(collect_box_db::Message))) {
        return 0;
    }
    const collect_box_db::Message response =
        *reinterpret_cast<const collect_box_db::Message*>(payload);
    collect_box::ApplyDatabaseResult(user, response);
    // Optional feature failures are local.  A nonzero Inter return is the
    // root dispatcher's fatal disconnect contract and must never be used for
    // malformed, stale, or failed CollectBox replies.
    return 0;
}
