#ifndef OLDSRV_OLD_DATA_PVF_STACKABLE_DISGUISE_STACKABLE_REGISTRY_H_
#define OLDSRV_OLD_DATA_PVF_STACKABLE_DISGUISE_STACKABLE_REGISTRY_H_

#include <stdint.h>

struct STStackableScript;

namespace disguise_pvf {

// Typed projection of the target client's [disguise] stackable contract.
// The first [int data] value is the u16 revision/model id and the second is
// the client-owned local disguise timer in seconds.  The item id itself is
// carried by CMD211's 0828-only u32 tail and identifies the client-side PVF
// effect template (for example, the village-movement-speed appendage).
struct DisguiseStackableRule {
    uint8_t kind;
    uint16_t disguise_id;
    int32_t duration_seconds;
};

// CItemList's PVF import owner calls reset once before loading Stackable.lst
// and registers each successfully parsed prototype.  Register returns true
// for non-disguise items and well-formed disguise items, and false only for a
// malformed [disguise] definition.
void ResetDisguiseStackableRegistry();
bool RegisterDisguiseStackable(const STStackableScript& script);

// Runtime packet owners use the immutable registry; no PVF parser or DB work
// runs on the packet thread.
bool FindDisguiseStackable(uint32_t item_id, DisguiseStackableRule& rule);

}  // namespace disguise_pvf

#endif  // OLDSRV_OLD_DATA_PVF_STACKABLE_DISGUISE_STACKABLE_REGISTRY_H_
