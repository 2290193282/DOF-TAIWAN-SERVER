#include "old_data/PVF/stackable/disguise_stackable_registry.h"

#include <limits.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "old_data/PVF/stackable/stackable_script.h"

namespace disguise_pvf {
namespace {

typedef std::map<uint32_t, DisguiseStackableRule> RuleMap;

RuleMap& Rules() {
    static RuleMap rules;
    return rules;
}

bool DecodeKind(const std::string& type, uint8_t& kind) {
    // sub_BC73E0 maps the three named non-character identities to 1..3 and
    // leaves the character/APC family at zero.  Reject arbitrary strings even
    // though the client sender's final else branch would also encode zero.
    if (type == "character" || type == "apc") {
        kind = 0;
        return true;
    }
    if (type == "npc") {
        kind = 1;
        return true;
    }
    if (type == "monster") {
        kind = 2;
        return true;
    }
    if (type == "creature") {
        kind = 3;
        return true;
    }
    return false;
}

}  // namespace

void ResetDisguiseStackableRegistry() {
    Rules().clear();
}

bool RegisterDisguiseStackable(const STStackableScript& script) {
    const int kDisguiseStackableType = 22;
    if (script.value_1e0 != kDisguiseStackableType) {
        return true;
    }

    const std::vector<std::string>& strings = script.item.string_data;
    const std::vector<int>& integers = script.item.int_data;
    if (script.item.index <= 0 || strings.empty() || integers.size() < 2u) {
        return false;
    }

    uint8_t kind = 0;
    if (!DecodeKind(strings[0], kind) || integers[0] < 0 ||
        integers[0] > USHRT_MAX || integers[1] < 0) {
        return false;
    }

    DisguiseStackableRule rule;
    rule.kind = kind;
    rule.disguise_id = static_cast<uint16_t>(integers[0]);
    rule.duration_seconds = integers[1];

    const uint32_t item_id = static_cast<uint32_t>(script.item.index);
    const std::pair<RuleMap::iterator, bool> inserted =
        Rules().insert(std::make_pair(item_id, rule));
    if (inserted.second) {
        return true;
    }

    // Stackable.lst should have a unique exact key.  Treat a byte-identical
    // duplicate as harmless, but reject conflicting definitions.
    const DisguiseStackableRule& existing = inserted.first->second;
    return existing.kind == rule.kind &&
           existing.disguise_id == rule.disguise_id &&
           existing.duration_seconds == rule.duration_seconds;
}

bool FindDisguiseStackable(uint32_t item_id, DisguiseStackableRule& rule) {
    const RuleMap::const_iterator found = Rules().find(item_id);
    if (found == Rules().end()) {
        return false;
    }
    rule = found->second;
    return true;
}

}  // namespace disguise_pvf
