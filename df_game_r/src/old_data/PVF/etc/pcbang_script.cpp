#include "old_data/PVF/etc/pcbang_script.h"

#include "old_data/PVF/common/rdar_script.h"

#include <string>

// IDA_EA: 0x891AC9A
// IDA_NAME: _ZN14stNeoplePCBANG5clearEv
// DEMANGLED: stNeoplePCBANG::clear(void)
int stNeoplePCBANG::clear() {
    level = 0;
    people_count = 0;
    unknown_08 = 0;
    status_values.clear();
    return 0;
}

// IDA_EA: 0x891ACCE
// IDA_NAME: _ZN14stNeoplePCBANGC2Ev
// DEMANGLED: stNeoplePCBANG::stNeoplePCBANG(void)
stNeoplePCBANG::stNeoplePCBANG() : status_values() {
    clear();
}

// IDA_EA: 0x891C6C2
// IDA_NAME: _ZN14stNeoplePCBANGD2Ev
// DEMANGLED: stNeoplePCBANG::~stNeoplePCBANG()
stNeoplePCBANG::~stNeoplePCBANG() {}

// IDA_EA: 0x89276F8
// IDA_NAME: _ZN14stNeoplePCBANGC2ERKS_
// DEMANGLED: stNeoplePCBANG::stNeoplePCBANG(stNeoplePCBANG const&)
stNeoplePCBANG::stNeoplePCBANG(const stNeoplePCBANG& other)
    : level(other.level),
      people_count(other.people_count),
      unknown_08(other.unknown_08),
      status_values(other.status_values) {}

// IDA_EA: 0x89277BE
// IDA_NAME: _ZN14stNeoplePCBANGaSERKS_
// DEMANGLED: stNeoplePCBANG::operator=(stNeoplePCBANG const&)
stNeoplePCBANG& stNeoplePCBANG::operator=(const stNeoplePCBANG& other) {
    level = other.level;
    people_count = other.people_count;
    unknown_08 = other.unknown_08;
    status_values = other.status_values;
    return *this;
}

namespace {

void appendStatus(stNeoplePCBANG& entry, ENUM_CHANGE_STATUS_TYPE type) {
    entry.status_values.push_back(
        std::make_pair(type, ScanInt(static_cast<bool*>(0))));
}

}  // namespace

// IDA_EA: 0x8905773
// IDA_NAME: _Z18importPCBANGScriptPKcRSt6vectorI14stNeoplePCBANGSaIS2_EE
// DEMANGLED: importPCBANGScript(char const*,std::vector<stNeoplePCBANG,std::allocator<stNeoplePCBANG>> &)
int importPCBANGScript(const char* filename,
                       std::vector<stNeoplePCBANG>& entries) {
    if (!loadRDARScriptFile("", filename)) {
        return 5143;
    }

    std::string tag;
    entries.clear();
    stNeoplePCBANG entry;

    while (ScanType(tag, true) == 1) {
        if (tag == "[level]") {
            entry.level = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[people count]") {
            entry.people_count = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[physical attack]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(0));
        } else if (tag == "[physical defense]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(3));
        } else if (tag == "[magical attack]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(1));
        } else if (tag == "[magical defense]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(2));
        } else if (tag == "[fire resistance]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(20));
        } else if (tag == "[water resistance]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(21));
        } else if (tag == "[dark resistance]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(22));
        } else if (tag == "[light resistance]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(23));
        } else if (tag == "[HP regen speed]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(14));
        } else if (tag == "[MP regen speed]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(29));
        } else if (tag == "[move speed]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(11));
        } else if (tag == "[attack speed]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(10));
        } else if (tag == "[cast speed]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(12));
        } else if (tag == "[hit recovery]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(34));
        } else if (tag == "[jump power]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(8));
        } else if (tag == "[HP MAX]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(13));
        } else if (tag == "[MP MAX]") {
            appendStatus(entry, static_cast<ENUM_CHANGE_STATUS_TYPE>(48));
        } else if (tag == "[/level]") {
            entries.push_back(entry);
            entry.clear();
        }
    }

    return 1;
}
