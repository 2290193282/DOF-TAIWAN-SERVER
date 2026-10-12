#include "old_data/PVF/event/datamanager/npc_relation_event_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_game/economy/shop/npc_runtime.h"

#include <limits.h>
#include <new>
#include <stdint.h>

#include <string>
#include <vector>

namespace {

struct NPCRelationToken0828 {
    NPCRelationToken0828()
        : kind(DNF_TOK_ERROR), integer(0), floating(0.0f), text() {}

    DNFLEX_TOKEN kind;
    int integer;
    float floating;
    std::string text;
};

class NPCRelationTokenReader0828 {
public:
    bool Next(NPCRelationToken0828& token) {
        token.integer = 0;
        token.floating = 0.0f;
        token.text.clear();
        token.kind = ScanAll(&token.integer, &token.floating, &token.text);
        return token.kind != DNF_TOK_ERROR;
    }
};

bool ReadInteger0828(NPCRelationTokenReader0828& reader, int& value) {
    NPCRelationToken0828 token;
    if (!reader.Next(token) ||
        (token.kind != DNF_TOK_DECIMAL &&
         token.kind != DNF_TOK_HEXADECIMAL)) {
        return false;
    }
    value = token.integer;
    return true;
}

bool ReadType0828(NPCRelationTokenReader0828& reader,
                  const char* expected) {
    NPCRelationToken0828 token;
    return reader.Next(token) && token.kind == DNF_TOK_TYPE &&
           token.text == expected;
}

template <typename ValueType>
bool ReadPositiveIncreasingList0828(
    NPCRelationTokenReader0828& reader,
    const char* closing_tag,
    unsigned int maximum,
    std::vector<ValueType>& output) {
    output.clear();
    while (true) {
        NPCRelationToken0828 token;
        if (!reader.Next(token)) {
            return false;
        }
        if (token.kind == DNF_TOK_TYPE) {
            return token.text == closing_tag && !output.empty();
        }
        if ((token.kind != DNF_TOK_DECIMAL &&
             token.kind != DNF_TOK_HEXADECIMAL) ||
            token.integer <= 0 ||
            static_cast<unsigned int>(token.integer) > maximum) {
            return false;
        }
        const ValueType value = static_cast<ValueType>(token.integer);
        if (!output.empty() && output.back() >= value) {
            return false;
        }
        output.push_back(value);
    }
}

bool ParseDislikeItems0828(NPCRelationTokenReader0828& reader,
                           CNPCRelationEventManager& manager,
                           unsigned short npc_index) {
    bool saw_item = false;
    while (true) {
        NPCRelationToken0828 token;
        if (!reader.Next(token)) {
            return false;
        }
        if (token.kind == DNF_TOK_TYPE) {
            return token.text == "[/can not give item]" && saw_item;
        }
        if ((token.kind != DNF_TOK_DECIMAL &&
             token.kind != DNF_TOK_HEXADECIMAL) ||
            token.integer <= 0 ||
            manager.AddDislikeItemInfo(
                npc_index, static_cast<unsigned int>(token.integer)) != 0) {
            return false;
        }
        saw_item = true;
    }
}

bool ParseFavorBlock0828(NPCRelationTokenReader0828& reader,
                         CNPCRelationEventManager& manager,
                         unsigned short npc_index,
                         const std::string& opening_tag) {
    const int favor_index = manager.GetFavorIndex(opening_tag.c_str());
    if (favor_index < NPC_RELATION_EVENT_FAVOR_GOOD ||
        favor_index > NPC_RELATION_EVENT_FAVOR_BAD ||
        !ReadType0828(reader, "[item]")) {
        return false;
    }

    bool saw_item = false;
    while (true) {
        NPCRelationToken0828 token;
        if (!reader.Next(token)) {
            return false;
        }
        if (token.kind == DNF_TOK_TYPE) {
            if (token.text != "[/item]" || !saw_item) {
                return false;
            }
            break;
        }
        if ((token.kind != DNF_TOK_DECIMAL &&
             token.kind != DNF_TOK_HEXADECIMAL) ||
            token.integer <= 0) {
            return false;
        }
        const unsigned int item_id =
            static_cast<unsigned int>(token.integer);
        int required_count = 0;
        if (!ReadInteger0828(reader, required_count) || required_count <= 0 ||
            manager.AddFavorItemInfo(
                npc_index,
                static_cast<ENUM_NPC_RELATION_EVENT_FAVOR_KIND>(favor_index),
                item_id,
                static_cast<unsigned int>(required_count)) != 0) {
            return false;
        }
        saw_item = true;
    }

    int reward_item_id = 0;
    int reward_item_count = 0;
    if (!ReadType0828(reader, "[reward]") ||
        !ReadInteger0828(reader, reward_item_id) || reward_item_id <= 0 ||
        !ReadInteger0828(reader, reward_item_count) ||
        reward_item_count <= 0 ||
        manager.AddRewardItemInfo(
            npc_index,
            static_cast<ENUM_NPC_RELATION_EVENT_FAVOR_KIND>(favor_index),
            static_cast<unsigned int>(reward_item_id),
            static_cast<unsigned int>(reward_item_count)) <= 0) {
        return false;
    }

    std::string closing_tag("[/");
    closing_tag.append(opening_tag, 1, opening_tag.size() - 2);
    closing_tag += "]";
    return ReadType0828(reader, closing_tag.c_str());
}

bool ParseNPCRelationEvent0828(
    CNPCRelationEventManager& parsed,
    std::vector<unsigned short>& gift_npc_log_ids,
    std::vector<unsigned int>& gift_item_log_ids) {
    NPCRelationTokenReader0828 reader;
    bool saw_gift_npc_log = false;
    bool saw_gift_item_log = false;
    bool saw_legacy_wrapper = false;
    bool inside_legacy_wrapper = false;
    bool saw_npc = false;
    bool have_current_npc = false;
    unsigned short current_npc = 0;

    while (true) {
        NPCRelationToken0828 token;
        if (!reader.Next(token)) {
            return false;
        }
        if (token.kind == DNF_TOK_EOF) {
            break;
        }
        if (token.kind != DNF_TOK_TYPE) {
            return false;
        }

        if (token.text == "[gift npc log]") {
            if (saw_gift_npc_log || saw_npc || inside_legacy_wrapper ||
                !ReadPositiveIncreasingList0828(
                    reader,
                    "[/gift npc log]",
                    static_cast<unsigned int>(USHRT_MAX),
                    gift_npc_log_ids)) {
                return false;
            }
            saw_gift_npc_log = true;
        } else if (token.text == "[gift item log]") {
            if (saw_gift_item_log || saw_npc || inside_legacy_wrapper ||
                !ReadPositiveIncreasingList0828(
                    reader,
                    "[/gift item log]",
                    static_cast<unsigned int>(INT_MAX),
                    gift_item_log_ids)) {
                return false;
            }
            saw_gift_item_log = true;
        } else if (token.text == "[NPC]") {
            if (inside_legacy_wrapper || saw_gift_npc_log ||
                saw_gift_item_log) {
                return false;
            }
            saw_legacy_wrapper = true;
            inside_legacy_wrapper = true;
            have_current_npc = false;
        } else if (token.text == "[/NPC]") {
            if (!inside_legacy_wrapper) {
                return false;
            }
            inside_legacy_wrapper = false;
            have_current_npc = false;
        } else if (token.text == "[index]") {
            int npc_index = 0;
            if (!ReadInteger0828(reader, npc_index) || npc_index <= 0 ||
                npc_index > USHRT_MAX) {
                return false;
            }
            stNPCRelationEventInfo* const info =
                new (std::nothrow) stNPCRelationEventInfo();
            if (!info ||
                parsed.AddNPCInfo(static_cast<unsigned short>(npc_index),
                                  info) == 0) {
                delete info;
                return false;
            }
            current_npc = static_cast<unsigned short>(npc_index);
            have_current_npc = true;
            saw_npc = true;
        } else if (token.text == "[good]" || token.text == "[soso]" ||
                   token.text == "[bad]") {
            if (!have_current_npc ||
                !ParseFavorBlock0828(
                    reader, parsed, current_npc, token.text)) {
                return false;
            }
        } else if (token.text == "[can not give item]") {
            if (!have_current_npc ||
                !ParseDislikeItems0828(reader, parsed, current_npc)) {
                return false;
            }
        } else {
            return false;
        }
    }

    if (inside_legacy_wrapper || !saw_npc ||
        saw_gift_npc_log != saw_gift_item_log) {
        return false;
    }
    // The 0828 form owns both policy blocks and has no [NPC] wrapper.  The
    // old 0627 form owns the wrapper and no policy blocks.  Mixed/anonymous
    // variants are rejected instead of being accepted by token skipping.
    return saw_gift_npc_log || saw_legacy_wrapper;
}

}  // namespace

// IDA_EA: 0x890D44F
// IDA_NAME: _Z28importNPCRelationEventScriptPKcP24CNPCRelationEventManager
// DEMANGLED: importNPCRelationEventScript(char const*,CNPCRelationEventManager *)
int importNPCRelationEventScript(const char* filename,
                                 CNPCRelationEventManager* manager) {
    if (!manager || !filename || !loadRDARScriptFile("", filename)) {
        return 0;
    }

    CNPCRelationEventManager parsed;
    std::vector<unsigned short> gift_npc_log_ids;
    std::vector<unsigned int> gift_item_log_ids;
    if (!ParseNPCRelationEvent0828(
            parsed, gift_npc_log_ids, gift_item_log_ids)) {
        return 0;
    }
    return manager->ReplaceImportedState(
        parsed, gift_npc_log_ids, gift_item_log_ids);
}
