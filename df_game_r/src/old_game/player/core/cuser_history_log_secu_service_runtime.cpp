#include "old_game/player/core/cuser.h"


// IDA_EA: 0x868479C
// IDA_NAME: _ZN15cUserHistoryLog11SecuServiceE22ENUM_SECU_SERVICE_TYPE24ENUM_SECU_SERVICE_ACTION
// DEMANGLED: cUserHistoryLog::SecuService(ENUM_SECU_SERVICE_TYPE,ENUM_SECU_SERVICE_ACTION)
// The original type-0/type-1 branches are the only observed service kinds.
// Other values reach the original no-log path. All recovered callers discard
// EAX, so this is intentionally a void member rather than fabricating the
// decompiler's unproven int result.
void cUserHistoryLog::SecuService(ENUM_SECU_SERVICE_TYPE service_type,
                                  ENUM_SECU_SERVICE_ACTION action) {
    if (service_type == static_cast<ENUM_SECU_SERVICE_TYPE>(0)) {
        owner_->LogHistory("s_gob,%d", static_cast<int>(action));
        return;
    }

    if (service_type == static_cast<ENUM_SECU_SERVICE_TYPE>(1)) {
        owner_->LogHistory("s_card,%d", static_cast<int>(action));
    }
}
