#ifndef OLDSRV_OLD_DATA_PVF_ETC_QUICK_PARTY_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_QUICK_PARTY_SCRIPT_H_

#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"

namespace QuickParty {

// IDA_EA: 0x8A6E282
// IDA_NAME: _ZN10QuickParty22importQuickPartyScriptEPKcPNS_14QuickPartyDataE
// DEMANGLED: QuickParty::importQuickPartyScript(char const*,QuickParty::QuickPartyData *)
int importQuickPartyScript(const char* filename, QuickPartyData* data);

}  // namespace QuickParty

#endif  // OLDSRV_OLD_DATA_PVF_ETC_QUICK_PARTY_SCRIPT_H_
