#ifndef OLDSRV_OLD_DATA_DB_CREATURE_COMPOUND_CREATURE_MARKER_H_
#define OLDSRV_OLD_DATA_DB_CREATURE_COMPOUND_CREATURE_MARKER_H_

namespace compound_creature {

// Process-internal marker carried only over signal135/signal136.  It is not a
// client wire value and is never stored in creature_items.
const int kInsertUpdateKind = 0x226;

}  // namespace compound_creature

#endif  // OLDSRV_OLD_DATA_DB_CREATURE_COMPOUND_CREATURE_MARKER_H_
