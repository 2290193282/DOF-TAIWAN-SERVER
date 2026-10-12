#include "old_data/PVF/data/data_manager.h"

// This former migration unit duplicated 114 CDataManager functions by
// projecting raw this+offset addresses.  It is intentionally definition-free:
// the canonical 0627 owner is src/old_data/PVF/data/data_manager_full.cpp,
// which provides the complete named CDataManager object graph and lifecycle.
// Keeping this tombstone prevents the obsolete offset facade from being
// accidentally reintroduced into a future manifest.
