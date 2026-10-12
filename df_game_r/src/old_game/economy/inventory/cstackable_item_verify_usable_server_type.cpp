#include "old_game/economy/inventory/inventory.h"

#include "core/config/env/environment.h"

#include <algorithm>
#include <vector>


// IDA_EA: 0x850E8DE
// IDA_NAME: _ZN14CStackableItem22verifyUsableServerTypeEv
// DEMANGLED: CStackableItem::verifyUsableServerType(void)
int CStackableItem::verifyUsableServerType() {
    // StackableAction::UsableServerType is the original int enum vector at
    // CStackableItem +952. An absent action rule permits every server type.
    if (usable_server_types_.empty()) {
        return 1;
    }

    // The original body obtains the configured CEnvironment server type and
    // searches the script-provided vector (general=1, event=2, starter=3).
    const int server_type = G_CEnvironment()->get_server_type();
    return std::find(
               usable_server_types_.begin(), usable_server_types_.end(),
               static_cast<StackableAction::UsableServerType>(server_type)) !=
           usable_server_types_.end();
}
