#ifndef OLDSRV_OLD_DATA_PVF_ETC_PARTY_MATCHING_INFO_H_
#define OLDSRV_OLD_DATA_PVF_ETC_PARTY_MATCHING_INFO_H_

#include <string>
#include <vector>

// IDA unit: 09_dungeon_pvp_party/stPartyMatchingInfo.
// IDA refs: 0x891B78E, 0x891B81C, 0x891CA12, 0x892EAD6, 0x892EBE0.
//
// This value is shared by the PVF importer and the dungeon/party runtime.  Keep
// one canonical declaration so both subsystems use the same C++ type and ABI.
struct stPartyMatchingInfo {
    stPartyMatchingInfo();
    stPartyMatchingInfo(const stPartyMatchingInfo& rhs);
    ~stPartyMatchingInfo();

    int clear();
    stPartyMatchingInfo& operator=(const stPartyMatchingInfo& rhs);

    std::string channel_name;
    std::string region_name;
    std::vector<int> dungeon_indices;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(stPartyMatchingInfo) == 0x14u,
              "stPartyMatchingInfo must retain the 0627 i386 value ABI");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_ETC_PARTY_MATCHING_INFO_H_
