#include "old_game/economy/shop/npc_runtime.h"

#include "old_game/activity/schedule/event_value_types.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <new>
#include <utility>
#include <vector>

namespace {

struct NPCGiftLogPolicy0828 {
    std::vector<unsigned short> npc_ids;
    std::vector<unsigned int> item_ids;
};

typedef std::map<const CNPCRelationEventManager*, NPCGiftLogPolicy0828>
    NPCGiftLogPolicyMap0828;

// Deliberately process-lifetime storage.  CDataManager is a global singleton,
// and a heap-backed function static avoids shutdown-order use-after-destroy
// when its embedded manager is torn down.
NPCGiftLogPolicyMap0828& GiftLogPolicies0828() {
    static NPCGiftLogPolicyMap0828* const policies =
        new NPCGiftLogPolicyMap0828();
    return *policies;
}

bool IsStrictlyIncreasing(const std::vector<unsigned short>& values) {
    for (std::vector<unsigned short>::size_type index = 1;
         index < values.size(); ++index) {
        if (values[index - 1] >= values[index]) {
            return false;
        }
    }
    return true;
}

bool IsStrictlyIncreasing(const std::vector<unsigned int>& values) {
    for (std::vector<unsigned int>::size_type index = 1;
         index < values.size(); ++index) {
        if (values[index - 1] >= values[index]) {
            return false;
        }
    }
    return true;
}

}  // namespace

// IDA_EA: 0x890CE20
// IDA_NAME: _ZN24CNPCRelationEventManagerC2Ev
// DEMANGLED: CNPCRelationEventManager::CNPCRelationEventManager(void)
CNPCRelationEventManager::CNPCRelationEventManager()
    : npc_infos() {}

// IDA_EA: 0x890CE34
// IDA_NAME: _ZN24CNPCRelationEventManagerD2Ev
// DEMANGLED: CNPCRelationEventManager::~CNPCRelationEventManager()
CNPCRelationEventManager::~CNPCRelationEventManager() {
    GiftLogPolicies0828().erase(this);
    for (NPCInfoMap::iterator it = npc_infos.begin();
         it != npc_infos.end();
         ++it) {
        stNPCRelationEventInfo* const info = it->second;
        if (info) {
            info->~stNPCRelationEventInfo();
            ::operator delete(info);
        }
    }
}

// IDA_EA: 0x890CEEA
// IDA_NAME: _ZN24CNPCRelationEventManager10AddNPCInfoEtP22stNPCRelationEventInfo
// DEMANGLED: CNPCRelationEventManager::AddNPCInfo(unsigned short,stNPCRelationEventInfo *)
int CNPCRelationEventManager::AddNPCInfo(
    unsigned short npc_index,
    stNPCRelationEventInfo* info) {
    return npc_infos.insert(std::make_pair(npc_index, info)).second;
}

// IDA_EA: 0x890CF44
// IDA_NAME: _ZN24CNPCRelationEventManager16AddFavorItemInfoEt34ENUM_NPC_RELATION_EVENT_FAVOR_KINDjj
// DEMANGLED: CNPCRelationEventManager::AddFavorItemInfo(unsigned short,ENUM_NPC_RELATION_EVENT_FAVOR_KIND,unsigned int,unsigned int)
int CNPCRelationEventManager::AddFavorItemInfo(
    unsigned short npc_index,
    ENUM_NPC_RELATION_EVENT_FAVOR_KIND favor_kind,
    unsigned int item_id,
    unsigned int required_count) {
    NPCInfoMap::iterator it = npc_infos.find(npc_index);
    if (it == npc_infos.end()) {
        return 1;
    }

    stNPCEventItemInfo item;
    item.item_id = static_cast<int>(item_id);
    item.count = static_cast<int>(required_count);
    it->second->favor_infos[favor_kind].favor_items.push_back(item);
    return 0;
}

// IDA_EA: 0x890CFDA
// IDA_NAME: _ZN24CNPCRelationEventManager17AddRewardItemInfoEt34ENUM_NPC_RELATION_EVENT_FAVOR_KINDjj
// DEMANGLED: CNPCRelationEventManager::AddRewardItemInfo(unsigned short,ENUM_NPC_RELATION_EVENT_FAVOR_KIND,unsigned int,unsigned int)
int CNPCRelationEventManager::AddRewardItemInfo(
    unsigned short npc_index,
    ENUM_NPC_RELATION_EVENT_FAVOR_KIND favor_kind,
    unsigned int item_id,
    unsigned int item_count) {
    NPCInfoMap::iterator it = npc_infos.find(npc_index);
    if (it == npc_infos.end()) {
        return 1;
    }

    it->second->favor_infos[favor_kind].reward_item_id =
        static_cast<int>(item_id);
    it->second->favor_infos[favor_kind].reward_item_count =
        static_cast<int>(item_count);
    return static_cast<int>(item_count);
}

// IDA_EA: 0x890D07A
// IDA_NAME: _ZN24CNPCRelationEventManager18AddDislikeItemInfoEtj
// DEMANGLED: CNPCRelationEventManager::AddDislikeItemInfo(unsigned short,unsigned int)
int CNPCRelationEventManager::AddDislikeItemInfo(
    unsigned short npc_index,
    unsigned int item_id) {
    NPCInfoMap::iterator it = npc_infos.find(npc_index);
    if (it == npc_infos.end()) {
        return 1;
    }

    it->second->dislike_items.push_back(static_cast<int>(item_id));
    return 0;
}

// IDA_EA: 0x890D100
// IDA_NAME: _ZN24CNPCRelationEventManager13IsDislikeItemEtj
// DEMANGLED: CNPCRelationEventManager::IsDislikeItem(unsigned short,unsigned int)
int CNPCRelationEventManager::IsDislikeItem(
    unsigned short npc_index,
    unsigned int item_id) {
    NPCInfoMap::iterator it = npc_infos.find(npc_index);
    if (it == npc_infos.end()) {
        return 0;
    }

    for (unsigned int i = 0; i < it->second->dislike_items.size(); ++i) {
        if (static_cast<unsigned int>(it->second->dislike_items.at(i)) ==
            item_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x890D1C0
// IDA_NAME: _ZN24CNPCRelationEventManager11GetItemKindEtjj
// DEMANGLED: CNPCRelationEventManager::GetItemKind(unsigned short,unsigned int,unsigned int)
stTempRewardInfo CNPCRelationEventManager::GetItemKind(
    unsigned short npc_index,
    unsigned int item_id,
    unsigned int item_count) {
    stTempRewardInfo reward_info;

    NPCInfoMap::iterator it = npc_infos.find(npc_index);
    if (it == npc_infos.end()) {
        return reward_info;
    }

    stNPCRelationEventInfo* info = it->second;
    for (int favor_kind = 0; favor_kind <= 2; ++favor_kind) {
        std::vector<stNPCEventItemInfo>& favor_items =
            info->favor_infos[favor_kind].favor_items;
        for (unsigned int i = 0; i < favor_items.size(); ++i) {
            stNPCEventItemInfo& item = favor_items.at(i);
            const unsigned int required_count =
                static_cast<unsigned int>(item.count);
            if (static_cast<unsigned int>(item.item_id) == item_id &&
                required_count <= item_count) {
                if (required_count != 0) {
                    reward_info.first_dword =
                        static_cast<unsigned int>(favor_kind);
                    reward_info.second_dword = static_cast<unsigned int>(
                        static_cast<long double>(item_count) /
                        static_cast<long double>(required_count));
                }
                return reward_info;
            }
        }
    }
    return reward_info;
}

// IDA_EA: 0x890D326
// IDA_NAME: _ZN24CNPCRelationEventManager13GetRewardItemEt34ENUM_NPC_RELATION_EVENT_FAVOR_KIND
// DEMANGLED: CNPCRelationEventManager::GetRewardItem(unsigned short,ENUM_NPC_RELATION_EVENT_FAVOR_KIND)
stNPCEventItemInfo CNPCRelationEventManager::GetRewardItem(
    unsigned short npc_index,
    ENUM_NPC_RELATION_EVENT_FAVOR_KIND favor_kind) {
    stNPCEventItemInfo item;
    item.item_id = 0;
    item.count = 0;

    NPCInfoMap::iterator it = npc_infos.find(npc_index);
    if (it != npc_infos.end()) {
        item.item_id = it->second->favor_infos[favor_kind].reward_item_id;
        item.count = it->second->favor_infos[favor_kind].reward_item_count;
    }
    return item;
}

// IDA_EA: 0x890D3D0
// IDA_NAME: _ZN24CNPCRelationEventManager13GetFavorIndexEPKc
// DEMANGLED: CNPCRelationEventManager::GetFavorIndex(char const*)
int CNPCRelationEventManager::GetFavorIndex(const char* tag) {
    if (!std::strncmp("[good]", tag, 0x0a)) {
        return 0;
    }
    if (!std::strncmp("[soso]", tag, 0x0a)) {
        return 1;
    }
    if (!std::strncmp("[bad]", tag, 0x0a)) {
        return 2;
    }
    return 3;
}

int CNPCRelationEventManager::ReplaceImportedState(
    CNPCRelationEventManager& parsed,
    const std::vector<unsigned short>& gift_npc_log_ids,
    const std::vector<unsigned int>& gift_item_log_ids) {
    if (&parsed == this || !IsStrictlyIncreasing(gift_npc_log_ids) ||
        !IsStrictlyIncreasing(gift_item_log_ids)) {
        return 0;
    }

    NPCGiftLogPolicy0828 next_policy;
    try {
        next_policy.npc_ids = gift_npc_log_ids;
        next_policy.item_ids = gift_item_log_ids;
        NPCGiftLogPolicyMap0828& policies = GiftLogPolicies0828();
        NPCGiftLogPolicyMap0828::iterator found = policies.find(this);
        if (found == policies.end()) {
            found = policies.insert(std::make_pair(this, next_policy)).first;
        } else {
            found->second.npc_ids.swap(next_policy.npc_ids);
            found->second.item_ids.swap(next_policy.item_ids);
        }
    } catch (...) {
        return 0;
    }

    // std::map::swap cannot fail.  The parsed temporary now owns the old
    // records and releases them when the importer returns.
    npc_infos.swap(parsed.npc_infos);
    return 1;
}

int CNPCRelationEventManager::IsGiftNpcLogTarget(
    unsigned short npc_index) const {
    const NPCGiftLogPolicyMap0828& policies = GiftLogPolicies0828();
    NPCGiftLogPolicyMap0828::const_iterator found = policies.find(this);
    return found != policies.end() &&
           std::binary_search(found->second.npc_ids.begin(),
                              found->second.npc_ids.end(),
                              npc_index);
}

int CNPCRelationEventManager::IsGiftItemLogTarget(
    unsigned int item_id) const {
    const NPCGiftLogPolicyMap0828& policies = GiftLogPolicies0828();
    NPCGiftLogPolicyMap0828::const_iterator found = policies.find(this);
    return found != policies.end() &&
           std::binary_search(found->second.item_ids.begin(),
                              found->second.item_ids.end(),
                              item_id);
}
