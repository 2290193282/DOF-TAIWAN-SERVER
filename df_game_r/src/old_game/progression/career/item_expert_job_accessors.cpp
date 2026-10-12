#include "old_game/economy/inventory/item.h"

// IDA_EA: 0x850D2C2
// IDA_NAME: _ZNK5CItem35GetExpertJobCompoundResultVariationERN12STItemScript10SEXPERTJOB31stExpertCompoundResultVariationE
// DEMANGLED: CItem::GetExpertJobCompoundResultVariation(STItemScript::SEXPERTJOB::stExpertCompoundResultVariation &)const
int CItem::GetExpertJobCompoundResultVariation(
    STItemScript::SEXPERTJOB::stExpertCompoundResultVariation& value) const {
    // CItem::C2 constructs this named expert-job value at +0x110.  The two
    // floats are copied verbatim by the old function; this accessor keeps the
    // recovered projection within the CItem owner.
    value = expert_job_result_variation_;
    return static_cast<int>(value.value_00);
}
