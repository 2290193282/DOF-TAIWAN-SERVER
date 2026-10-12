#include "old_data/PVF/event/level_reward_scripts.h"

#include "common/config.h"
#include <algorithm>

// IDA_EA: 0x81B88BC
// IDA_NAME: _ZN21LevelUpRewardItemDataC2Ev
// DEMANGLED: LevelUpRewardItemData::LevelUpRewardItemData(void)
LevelUpRewardItemData::LevelUpRewardItemData() {
    clear();
}

// IDA_EA: 0x81B88D0
// IDA_NAME: _ZN21LevelUpRewardItemData5clearEv
// DEMANGLED: LevelUpRewardItemData::clear(void)
LevelUpRewardItemData* LevelUpRewardItemData::clear() {
    item_id = -1;
    count = 0;
    return this;
}

// IDA_EA: 0x81B967E
// IDA_NAME: _ZN21LevelUpRewardItemDataD2Ev
// DEMANGLED: LevelUpRewardItemData::~LevelUpRewardItemData()
LevelUpRewardItemData::~LevelUpRewardItemData() {}

// IDA_EA: 0x81B8830
// IDA_NAME: _ZNK25LevelUpRewardItemTableKeyltERKS_
// DEMANGLED: LevelUpRewardItemTableKey::operator<(LevelUpRewardItemTableKey const&)const
bool LevelUpRewardItemTableKey::operator<(
    const LevelUpRewardItemTableKey& rhs) const {
    if (charac_job != rhs.charac_job) {
        return charac_job < rhs.charac_job;
    }
    if (first_grow_type == rhs.first_grow_type) {
        return charac_level < rhs.charac_level;
    }
    return first_grow_type < rhs.first_grow_type;
}

// IDA_EA: 0x81B8896
// IDA_NAME: _ZN25LevelUpRewardItemTableKeyC2Eiii
// DEMANGLED: LevelUpRewardItemTableKey::LevelUpRewardItemTableKey(int,int,int)
LevelUpRewardItemTableKey::LevelUpRewardItemTableKey(
    int level,
    int job,
    int grow_type)
    : charac_level(static_cast<uint16_t>(level)),
      charac_job(static_cast<uint8_t>(job)),
      first_grow_type(static_cast<uint8_t>(grow_type)) {}

// IDA_EA: 0x891BAC0
// IDA_NAME: _ZN25LevelUpRewardItemTableKeyC2Ev
// DEMANGLED: LevelUpRewardItemTableKey::LevelUpRewardItemTableKey(void)
LevelUpRewardItemTableKey::LevelUpRewardItemTableKey() {
    clear();
}

// IDA_EA: 0x891BAD4
// IDA_NAME: _ZN25LevelUpRewardItemTableKey5clearEv
// DEMANGLED: LevelUpRewardItemTableKey::clear(void)
LevelUpRewardItemTableKey* LevelUpRewardItemTableKey::clear() {
    first_grow_type = 0;
    charac_job = oldsrv::config::kCharacterJobInvalid;
    charac_level = 200;
    return this;
}

// IDA_EA: 0x81B88E8
// IDA_NAME: _ZN25stLevelUpRewardItemScript26GetGrowUpRewardItemFunctorC2ERK25LevelUpRewardItemTableKeyRSt6vectorI21LevelUpRewardItemDataSaIS5_EE
// DEMANGLED: stLevelUpRewardItemScript::GetGrowUpRewardItemFunctor::GetGrowUpRewardItemFunctor(LevelUpRewardItemTableKey const&,std::vector<LevelUpRewardItemData,std::allocator<LevelUpRewardItemData>> &)
stLevelUpRewardItemScript::GetGrowUpRewardItemFunctor::
    GetGrowUpRewardItemFunctor(
        const LevelUpRewardItemTableKey& key_value,
        std::vector<LevelUpRewardItemData>& out_value)
    : key(&key_value),
      out(&out_value) {}

// IDA_EA: 0x81B88FE
// IDA_NAME: _ZNK25stLevelUpRewardItemScript26GetGrowUpRewardItemFunctorclERKSt4pairIK25LevelUpRewardItemTableKey21LevelUpRewardItemDataE
// DEMANGLED: stLevelUpRewardItemScript::GetGrowUpRewardItemFunctor::operator()(std::pair<LevelUpRewardItemTableKey const,LevelUpRewardItemData> const&)const
void stLevelUpRewardItemScript::GetGrowUpRewardItemFunctor::operator()(
    const std::pair<const LevelUpRewardItemTableKey,
                    LevelUpRewardItemData>& entry) const {
    if (key->first_grow_type == entry.first.first_grow_type &&
        key->charac_job == entry.first.charac_job &&
        static_cast<int16_t>(entry.first.charac_level) <=
            static_cast<int16_t>(key->charac_level)) {
        out->push_back(entry.second);
    }
}

// IDA_EA: 0x81B895A
// IDA_NAME: _ZN25stLevelUpRewardItemScript20get_data_when_growupERK25LevelUpRewardItemTableKeyRSt6vectorI21LevelUpRewardItemDataSaIS4_EE
// DEMANGLED: stLevelUpRewardItemScript::get_data_when_growup(LevelUpRewardItemTableKey const&,std::vector<LevelUpRewardItemData,std::allocator<LevelUpRewardItemData>> &)
int stLevelUpRewardItemScript::get_data_when_growup(
    const LevelUpRewardItemTableKey& key,
    std::vector<LevelUpRewardItemData>& out) {
    std::for_each(reward_items.begin(), reward_items.end(),
                  GetGrowUpRewardItemFunctor(key, out));
    return !out.empty();
}

// IDA_EA: 0x81B89DE
// IDA_NAME: _ZN25stLevelUpRewardItemScript8get_dataERK25LevelUpRewardItemTableKeyR21LevelUpRewardItemData
// DEMANGLED: stLevelUpRewardItemScript::get_data(LevelUpRewardItemTableKey const&,LevelUpRewardItemData &)
int stLevelUpRewardItemScript::get_data(
    const LevelUpRewardItemTableKey& key,
    LevelUpRewardItemData& out) {
    std::map<LevelUpRewardItemTableKey, LevelUpRewardItemData>::iterator it =
        reward_items.find(key);
    if (it == reward_items.end()) {
        return 0;
    }

    out = it->second;
    return 1;
}

// IDA_EA: 0x8371194
// IDA_NAME: _ZN25stLevelUpRewardItemScriptC2Ev
// DEMANGLED: stLevelUpRewardItemScript::stLevelUpRewardItemScript(void)
stLevelUpRewardItemScript::stLevelUpRewardItemScript()
    : reward_items() {
    clear();
}

// IDA_EA: 0x83711D6
// IDA_NAME: _ZN25stLevelUpRewardItemScript5clearEv
// DEMANGLED: stLevelUpRewardItemScript::clear(void)
void stLevelUpRewardItemScript::clear() {
    reward_items.clear();
}

// IDA_EA: 0x8371530
// IDA_NAME: _ZN25stLevelUpRewardItemScriptD2Ev
// DEMANGLED: stLevelUpRewardItemScript::~stLevelUpRewardItemScript()
stLevelUpRewardItemScript::~stLevelUpRewardItemScript() {}

// IDA_EA: 0x83714C6
// IDA_NAME: _ZN31stStepRewardStackableItemScriptC2Ev
// DEMANGLED: stStepRewardStackableItemScript::stStepRewardStackableItemScript(void)
stStepRewardStackableItemScript::stStepRewardStackableItemScript()
    : reward_items_by_step() {
    clear();
}

// IDA_EA: 0x8371508
// IDA_NAME: _ZN31stStepRewardStackableItemScript5clearEv
// DEMANGLED: stStepRewardStackableItemScript::clear(void)
void stStepRewardStackableItemScript::clear() {
    reward_items_by_step.clear();
}

// IDA_EA: 0x837151C
// IDA_NAME: _ZN31stStepRewardStackableItemScriptD2Ev
// DEMANGLED: stStepRewardStackableItemScript::~stStepRewardStackableItemScript()
stStepRewardStackableItemScript::~stStepRewardStackableItemScript() {}

// IDA_EA: 0x891BD0E
// IDA_NAME: _ZN31stStepRewardStackableItemScript8set_dataEiR17StackableItemData
// DEMANGLED: stStepRewardStackableItemScript::set_data(int,StackableItemData &)
void stStepRewardStackableItemScript::set_data(
    int step_index,
    StackableItemData& item_data) {
    std::map<int, std::vector<StackableItemData> >::iterator it =
        reward_items_by_step.find(step_index);
    if (it == reward_items_by_step.end()) {
        std::vector<StackableItemData> items;
        items.push_back(item_data);
        reward_items_by_step.insert(std::make_pair(step_index, items));
        return;
    }

    it->second.push_back(item_data);
}

// IDA_EA: 0x8371544
// IDA_NAME: _ZN24stReturnUserRewardScriptC2Ev
// DEMANGLED: stReturnUserRewardScript::stReturnUserRewardScript(void)
stReturnUserRewardScript::stReturnUserRewardScript()
    : first_dword(0),
      second_dword(0),
      level_up_rewards() {
    level_up_rewards.clear();
}

// IDA_EA: 0x837DDE6
// IDA_NAME: _ZN24stReturnUserRewardScriptD2Ev
// DEMANGLED: stReturnUserRewardScript::~stReturnUserRewardScript()
stReturnUserRewardScript::~stReturnUserRewardScript() {}
