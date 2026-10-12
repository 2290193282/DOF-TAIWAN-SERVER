#include "common/manager/chatting_emoticon.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_game/player/core/cuser.h"

#include <stdio.h>
#include <string.h>
#include <string>


namespace {

typedef std::map<std::string, int> TypeLookupMap;

}  // namespace

char g_ChattingEmoticonScriptBaseDirectory[256];
STScriptFileList g_ChattingEmoticonScriptFileList;

// IDA_EA: 0x80E5EE6
// IDA_NAME: _ZN26CChattingEmoticonConfigRow5clearEv
// DEMANGLED: CChattingEmoticonConfigRow::clear(void)
CChattingEmoticonConfigRow* CChattingEmoticonConfigRow::clear() {
    ::memset(this, 0, 0x14u);
    icon_id = 0;
    return this;
}

// IDA_EA: 0x80E8638
// IDA_NAME: _ZN26CChattingEmoticonConfigRowD2Ev
// DEMANGLED: CChattingEmoticonConfigRow::~CChattingEmoticonConfigRow()
CChattingEmoticonConfigRow::~CChattingEmoticonConfigRow() {}

// IDA_EA: 0x84B7368
// IDA_NAME: _ZN26CChattingEmoticonConfigRowC2Ev
// DEMANGLED: CChattingEmoticonConfigRow::CChattingEmoticonConfigRow(void)
CChattingEmoticonConfigRow::CChattingEmoticonConfigRow() { clear(); }

// IDA_EA: 0x80E5F12
// IDA_NAME: _ZN23CChattingEmoticonConfig5clearEv
// DEMANGLED: CChattingEmoticonConfig::clear(void)
bool CChattingEmoticonConfig::clear() {
    unsigned int index = 0;
    for (;; ++index) {
        const bool result = index <= 0x17u;
        if (index > 0x17u) {
            return result;
        }
        rows[index].clear();
    }
}

// IDA_EA: 0x84B738C
// IDA_NAME: _ZN23CChattingEmoticonConfigC2Ev
// DEMANGLED: CChattingEmoticonConfig::CChattingEmoticonConfig(void)
CChattingEmoticonConfig::CChattingEmoticonConfig() {}

// IDA_EA: 0x80E5C2A
// IDA_NAME: _ZN20ChattingEmoticonBaseC2Ev
// DEMANGLED: ChattingEmoticonBase::ChattingEmoticonBase(void)
ChattingEmoticonBase::ChattingEmoticonBase()
    : index_(0), premium_indices_() {}

// IDA_EA: 0x80E5C54
// IDA_NAME: _ZN20ChattingEmoticonBaseD2Ev
// DEMANGLED: ChattingEmoticonBase::~ChattingEmoticonBase()
// IDA deleting destructor: 0x80E5C88.
ChattingEmoticonBase::~ChattingEmoticonBase() {}

// IDA_EA: 0x80E5DDE
// IDA_NAME: _ZN20ChattingEmoticonBase15AddPremiumIndexEi
// DEMANGLED: ChattingEmoticonBase::AddPremiumIndex(int)
int ChattingEmoticonBase::AddPremiumIndex(int premium_index) {
    int result = premium_index;
    if (premium_index >= 2660543) {
        if (premium_index <= 2660544) {
            premium_indices_.insert(83);
        } else if (premium_index <= 2660546) {
            premium_indices_.insert(87);
        }
    }
    return result;
}

// IDA_EA: 0x80E5F46
// IDA_NAME: _ZNK20ChattingEmoticonBase16isCommonEmoticonERK5CUser
// DEMANGLED: ChattingEmoticonBase::isCommonEmoticon(CUser const&)const
int ChattingEmoticonBase::isCommonEmoticon(const CUser& user) const {
    return isUsable(user);
}

// IDA_EA: 0x80E5F68
// IDA_NAME: _ZNK20ChattingEmoticonBase8GetIndexEv
// DEMANGLED: ChattingEmoticonBase::GetIndex(void)const
int ChattingEmoticonBase::GetIndex() const { return index_; }

// IDA_EA: 0x80E5F74
// IDA_NAME: _ZN20ChattingEmoticonBase8SetIndexEi
// DEMANGLED: ChattingEmoticonBase::SetIndex(int)
ChattingEmoticonBase* ChattingEmoticonBase::SetIndex(int index) {
    index_ = index;
    return this;
}

// IDA_EA: 0x80E5D42
// IDA_NAME: _ZNK26ChattingEmoticon_byPremium8isUsableERK5CUser
// DEMANGLED: ChattingEmoticon_byPremium::isUsable(CUser const&)const
int ChattingEmoticon_byPremium::isUsable(const CUser& user) const {
    for (std::set<int>::const_iterator it = premium_indices_.begin();
         it != premium_indices_.end(); ++it) {
        if (user.isAffectedPremium(static_cast<ENUM_PREMIUM_TYPE>(*it))) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x80E5DCA
// IDA_NAME: _ZNK26ChattingEmoticon_byPremium18GetStartArrayIndexEv
// DEMANGLED: ChattingEmoticon_byPremium::GetStartArrayIndex(void)const
int ChattingEmoticon_byPremium::GetStartArrayIndex() const { return 6; }

// IDA_EA: 0x80E5DD4
// IDA_NAME: _ZNK26ChattingEmoticon_byPremium16GetEndArrayIndexEv
// DEMANGLED: ChattingEmoticon_byPremium::GetEndArrayIndex(void)const
int ChattingEmoticon_byPremium::GetEndArrayIndex() const { return 17; }

// IDA_EA: 0x80E5F9E
// IDA_NAME: _ZN26ChattingEmoticon_byPremiumC2Ev
// DEMANGLED: ChattingEmoticon_byPremium::ChattingEmoticon_byPremium(void)
ChattingEmoticon_byPremium::ChattingEmoticon_byPremium()
    : ChattingEmoticonBase() {}

// IDA_EA: 0x80E942A
// IDA_NAME: _ZN26ChattingEmoticon_byPremiumD2Ev
// DEMANGLED: ChattingEmoticon_byPremium::~ChattingEmoticon_byPremium()
// IDA deleting destructor: 0x80E945A.
ChattingEmoticon_byPremium::~ChattingEmoticon_byPremium() {}

// IDA_EA: 0x80E5E4E
// IDA_NAME: _ZNK24ChattingEmoticon_byLevel8isUsableERK5CUser
// DEMANGLED: ChattingEmoticon_byLevel::isUsable(CUser const&)const
int ChattingEmoticon_byLevel::isUsable(const CUser& user) const {
    return user.get_charac_level() >= level_;
}

// IDA_EA: 0x80E5E6C
// IDA_NAME: _ZNK24ChattingEmoticon_byLevel18GetStartArrayIndexEv
// DEMANGLED: ChattingEmoticon_byLevel::GetStartArrayIndex(void)const
int ChattingEmoticon_byLevel::GetStartArrayIndex() const { return 18; }

// IDA_EA: 0x80E5E76
// IDA_NAME: _ZNK24ChattingEmoticon_byLevel16GetEndArrayIndexEv
// DEMANGLED: ChattingEmoticon_byLevel::GetEndArrayIndex(void)const
int ChattingEmoticon_byLevel::GetEndArrayIndex() const { return 23; }

// IDA_EA: 0x80E5E80
// IDA_NAME: _ZNK24ChattingEmoticon_byLevel16isCommonEmoticonERK5CUser
// DEMANGLED: ChattingEmoticon_byLevel::isCommonEmoticon(CUser const&)const
int ChattingEmoticon_byLevel::isCommonEmoticon(const CUser& user) const {
    (void)user;
    return 1;
}

// IDA_EA: 0x80E5FBA
// IDA_NAME: _ZN24ChattingEmoticon_byLevelC2Ei
// DEMANGLED: ChattingEmoticon_byLevel::ChattingEmoticon_byLevel(int)
ChattingEmoticon_byLevel::ChattingEmoticon_byLevel(int level)
    : ChattingEmoticonBase(), level_(level) {}

// IDA_EA: 0x80E5FE0
// IDA_NAME: _ZN24ChattingEmoticon_byLevel15AddPremiumIndexEi
// DEMANGLED: ChattingEmoticon_byLevel::AddPremiumIndex(int)
int ChattingEmoticon_byLevel::AddPremiumIndex(int premium_index) {
    (void)premium_index;
    return 0;
}

// IDA_EA: 0x80E93DC
// IDA_NAME: _ZN24ChattingEmoticon_byLevelD2Ev
// DEMANGLED: ChattingEmoticon_byLevel::~ChattingEmoticon_byLevel()
// IDA deleting destructor: 0x80E940C.
ChattingEmoticon_byLevel::~ChattingEmoticon_byLevel() {}

// IDA_EA: 0x837E418
// IDA_NAME: _ZN21CChattingEmoticonListC2Ev
// DEMANGLED: CChattingEmoticonList::CChattingEmoticonList(void)
CChattingEmoticonList::CChattingEmoticonList()
    : by_index_(), by_type_(), reserved_() {}

// IDA_EA: 0x837E1AA
// IDA_NAME: _ZN21CChattingEmoticonListD2Ev
// DEMANGLED: CChattingEmoticonList::~CChattingEmoticonList()
CChattingEmoticonList::~CChattingEmoticonList() {}

// IDA_EA: 0x80E53A8
// IDA_NAME: _ZN21CChattingEmoticonList7FactoryERKN12EmoticonType14EmoticonScriptE
// DEMANGLED: CChattingEmoticonList::Factory(EmoticonType::EmoticonScript const&)
ChattingEmoticonBase* CChattingEmoticonList::Factory(
    const EmoticonType::EmoticonScript& script) {
    ChattingEmoticonBase* emoticon = 0;
    switch (script.value_04) {
        case 2:
            emoticon = new ChattingEmoticon_byPremium();
            emoticon->SetIndex(script.index);
            break;
        case 4:
            emoticon = new ChattingEmoticon_byLevel(script.value_08);
            emoticon->SetIndex(script.index);
            break;
        case 1:
            emoticon = new ChattingEmoticon_byEvent();
            emoticon->SetIndex(script.index);
            break;
    }
    return emoticon;
}

// IDA_EA: 0x80E54D2
// IDA_NAME: _ZN21CChattingEmoticonList9Load_listEv
// DEMANGLED: CChattingEmoticonList::Load_list(void)
int CChattingEmoticonList::Load_list() {
    const char* filename = 0;
    EmoticonType::EmoticonScript script;
    for (STScriptFileList::iterator it =
             g_ChattingEmoticonScriptFileList.begin();
         it != g_ChattingEmoticonScriptFileList.end(); ++it) {
        filename = it->second.c_str();
        if (filename == 0) {
            return 1;
        }
        if (ImportChattingEmoticon(script, filename) != 1) {
            ::printf("Mission Script Error: %s\n", filename);
            return 0;
        }

        ChattingEmoticonBase* emoticon = Factory(script);
        if (!emoticon) {
            return 0;
        }

        const int emoticon_index = emoticon->GetIndex();
        if (by_index_.find(emoticon_index) != by_index_.end()) {
            delete emoticon;
            return 0;
        }

        by_index_.insert(std::make_pair(emoticon_index, emoticon));
        if (by_type_.find(script.value_04) == by_type_.end()) {
            by_type_.insert(std::make_pair(
                script.value_04, std::vector<ChattingEmoticonBase*>()));
        }
        by_type_[script.value_04].push_back(emoticon);
    }
    return 1;
}

// IDA_EA: 0x80E581C
// IDA_NAME: _ZN21CChattingEmoticonList13find_emoticonEi
// DEMANGLED: CChattingEmoticonList::find_emoticon(int)
ChattingEmoticonBase* CChattingEmoticonList::find_emoticon(int index) {
    EmoticonByIndexMap::iterator it = by_index_.find(index);
    if (it == by_index_.end()) {
        return 0;
    }
    return it->second;
}

// IDA_EA: 0x80E5880
// IDA_NAME: _ZN21CChattingEmoticonList6ReCalcER23CChattingEmoticonConfigRK5CUser
// DEMANGLED: CChattingEmoticonList::ReCalc(CChattingEmoticonConfig &,CUser const&)
int CChattingEmoticonList::ReCalc(CChattingEmoticonConfig& config,
                                  const CUser& user) {
    std::vector<CChattingEmoticonConfigRow> old_rows;
    std::set<int> old_indexes;
    for (unsigned int index = 0; index <= 0x17u; ++index) {
        ChattingEmoticonBase* emoticon = find_emoticon(config.rows[index].icon_id);
        if (emoticon && emoticon->isCommonEmoticon(user) == 1) {
            old_rows.push_back(config.rows[index]);
            old_indexes.insert(config.rows[index].icon_id);
        }
    }

    config.clear();

    for (unsigned int index = 0; index < old_rows.size(); ++index) {
        ChattingEmoticonBase* emoticon = find_emoticon(old_rows[index].icon_id);
        if (emoticon) {
            for (int slot = emoticon->GetStartArrayIndex();
                 slot <= emoticon->GetEndArrayIndex();
                 ++slot) {
                if (!config.rows[slot].icon_id) {
                    config.rows[slot] = old_rows[index];
                    break;
                }
            }
        }
    }

    for (EmoticonByIndexMap::iterator it = by_index_.begin();
         it != by_index_.end(); ++it) {
        ChattingEmoticonBase* emoticon = it->second;
        const int emoticon_index = emoticon->GetIndex();
        if (old_indexes.find(emoticon_index) == old_indexes.end() &&
            emoticon->isCommonEmoticon(user) == 1) {
            for (int slot = emoticon->GetStartArrayIndex();
                 slot <= emoticon->GetEndArrayIndex();
                 ++slot) {
                if (!config.rows[slot].icon_id) {
                    config.rows[slot].icon_id =
                        static_cast<uint16_t>(emoticon_index);
                    break;
                }
            }
        }
    }
    return 1;
}

// IDA_EA: 0x88E30F8
// IDA_NAME: _Z30initChattingEmoticonScriptListPcS_
// DEMANGLED: initChattingEmoticonScriptList(char *,char *)
int initChattingEmoticonScriptList(char* base_dir, char* filename) {
    ::strncpy(g_ChattingEmoticonScriptBaseDirectory, base_dir, 0xFFu);
    if (g_ChattingEmoticonScriptBaseDirectory[
            ::strlen(g_ChattingEmoticonScriptBaseDirectory) - 1] != '\\' &&
        g_ChattingEmoticonScriptBaseDirectory[
            ::strlen(g_ChattingEmoticonScriptBaseDirectory) - 1] != '/') {
        const size_t length = ::strlen(g_ChattingEmoticonScriptBaseDirectory);
        ::memcpy(g_ChattingEmoticonScriptBaseDirectory + length, "/", 2u);
    }
    return loadRDARScriptListFile(&g_ChattingEmoticonScriptFileList,
                                  g_ChattingEmoticonScriptBaseDirectory,
                                  filename) == 1;
}

// IDA_EA: 0x88E31B4
// IDA_NAME: _Z22ImportChattingEmoticonRN12EmoticonType14EmoticonScriptEPKc
// DEMANGLED: ImportChattingEmoticon(EmoticonType::EmoticonScript &,char const*)
int ImportChattingEmoticon(EmoticonType::EmoticonScript& script,
                           const char* filename) {
    if (!loadRDARScriptFile(g_ChattingEmoticonScriptBaseDirectory, filename)) {
        return 0;
    }
    script.index =
        g_ChattingEmoticonScriptFileList.FindIndexByFullScan(filename);

    std::string value;
    std::string type_name;
    static TypeLookupMap lookup;
    static bool inited = false;
    if (!inited) {
        lookup.insert(std::make_pair("[type]", 47));
        lookup.insert(std::make_pair("[ani]", 64));
        lookup.insert(std::make_pair("[macro]", 70));
        inited = true;
    }

    while (ScanType(type_name, true) == 1) {
        TypeLookupMap::iterator it = lookup.find(type_name);
        const int type = (it != lookup.end()) ? it->second : -1;
        if (type == 0 || type == 47) {
            if (ScanStr(&value) != 1) {
                return 0;
            }
            if (value == "EVENT") {
                script.value_04 = 1;
            } else if (value == "CASH") {
                script.value_04 = 2;
            } else {
                script.value_04 = 4;
            }
            if (!ScanInt(&script.value_08)) {
                return 0;
            }
        } else if (type == 64) {
            if (ScanStr(&script.value_0C) != 1) {
                return 0;
            }
        } else if (type == 70) {
            if (ScanStr(&script.value_10) != 1) {
                return 0;
            }
        }
    }
    return 1;
}
