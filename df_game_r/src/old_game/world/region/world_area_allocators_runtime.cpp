#include "old_game/world/region/world_area_allocators.h"


// IDA_EA: 0x85689A6
// IDA_NAME: _ZN11CSchoolArea12GetAreaIndexEv
// DEMANGLED: CSchoolArea::GetAreaIndex(void)
int CSchoolArea::GetAreaIndex() {
    if (free_areas.empty()) {
        return -1;
    }
    const int area = free_areas.top();
    free_areas.pop();
    return area;
}

// IDA_EA: 0x85689EC
// IDA_NAME: _ZN11CSchoolArea14SetAreaMappingEji
// DEMANGLED: CSchoolArea::SetAreaMapping(unsigned int,int)
int CSchoolArea::SetAreaMapping(uint32_t school_key, int area_index) {
    return area_by_school.insert(std::make_pair(school_key, area_index)).second;
}

// IDA_EA: 0x8568A3E
// IDA_NAME: _ZN11CSchoolArea13AddSchoolAreaEj
// DEMANGLED: CSchoolArea::AddSchoolArea(unsigned int)
int CSchoolArea::AddSchoolArea(uint32_t school_key) {
    const int area = GetAreaIndex();
    if (area >= 0) {
        return SetAreaMapping(school_key, area);
    }
    return area;
}

// IDA_EA: 0x8568A74
// IDA_NAME: _ZN11CSchoolArea13DelSchoolAreaEj
// DEMANGLED: CSchoolArea::DelSchoolArea(unsigned int)
bool CSchoolArea::DelSchoolArea(uint32_t school_key) {
    std::map<uint32_t, int>::iterator found = area_by_school.find(school_key);
    if (found != area_by_school.end()) {
        free_areas.push(found->second);
        area_by_school.erase(found);
        return true;
    }
    return false;
}

// IDA_EA: 0x856F074
// IDA_NAME: _ZN11CSchoolAreaD2Ev
// DEMANGLED: CSchoolArea::~CSchoolArea()
CSchoolArea::~CSchoolArea() {}

// IDA_EA: 0x856F18C
// IDA_NAME: _ZN11CSchoolAreaC2Ev
// DEMANGLED: CSchoolArea::CSchoolArea(void)
CSchoolArea::CSchoolArea() {}

// IDA_EA: 0x86D23BC
// IDA_NAME: _ZN11CSchoolArea13SetSchoolAreaEii
// DEMANGLED: CSchoolArea::SetSchoolArea(int,int)
bool CSchoolArea::SetSchoolArea(int begin_area, int end_area) {
    for (int area = begin_area; area < end_area; ++area) {
        free_areas.push(area);
    }
    return false;
}

// IDA_EA: 0x86D23F8
// IDA_NAME: _ZN11CSchoolArea12GetAreaIndexEj
// DEMANGLED: CSchoolArea::GetAreaIndex(unsigned int)
int CSchoolArea::GetAreaIndex(uint32_t school_key) {
    std::map<uint32_t, int>::iterator found = area_by_school.find(school_key);
    if (found != area_by_school.end()) {
        return found->second;
    }
    return -1;
}

// IDA_EA: 0x82AA324
// IDA_NAME: _ZN14CGuildAgitArea18AllocGuildAgitAreaEj
// DEMANGLED: CGuildAgitArea::AllocGuildAgitArea(unsigned int)
int CGuildAgitArea::AllocGuildAgitArea(uint32_t guild_key) {
    const int area = GetArea();
    if (area >= 0) {
        return SetAreaMapping(guild_key, area);
    }
    return area;
}

// IDA_EA: 0x82AA35A
// IDA_NAME: _ZN14CGuildAgitArea7GetAreaEv
// DEMANGLED: CGuildAgitArea::GetArea(void)
int CGuildAgitArea::GetArea() {
    if (free_areas.empty()) {
        return -1;
    }
    const int area = free_areas.top();
    free_areas.pop();
    free_areas.pop();
    free_areas.pop();
    free_areas.pop();
    return area;
}

// IDA_EA: 0x82AA3CA
// IDA_NAME: _ZN14CGuildAgitArea14SetAreaMappingEji
// DEMANGLED: CGuildAgitArea::SetAreaMapping(unsigned int,int)
int CGuildAgitArea::SetAreaMapping(uint32_t guild_key, int area_index) {
    return area_by_guild.insert(std::make_pair(guild_key, area_index)).second;
}

// IDA_EA: 0x856F100
// IDA_NAME: _ZN14CGuildAgitAreaD2Ev
// DEMANGLED: CGuildAgitArea::~CGuildAgitArea()
CGuildAgitArea::~CGuildAgitArea() {}

// IDA_EA: 0x856F24C
// IDA_NAME: _ZN14CGuildAgitAreaC2Ev
// DEMANGLED: CGuildAgitArea::CGuildAgitArea(void)
CGuildAgitArea::CGuildAgitArea() {}

// IDA_EA: 0x86D249E
// IDA_NAME: _ZN14CGuildAgitArea22SetGuildAgitAreaDomainEii
// DEMANGLED: CGuildAgitArea::SetGuildAgitAreaDomain(int,int)
bool CGuildAgitArea::SetGuildAgitAreaDomain(int begin_area, int end_area) {
    for (int area = begin_area; area < end_area; ++area) {
        free_areas.push(area);
    }
    return false;
}

// IDA_EA: 0x86D24DA
// IDA_NAME: _ZN14CGuildAgitArea21GetGuildAgitAreaIndexEji
// DEMANGLED: CGuildAgitArea::GetGuildAgitAreaIndex(unsigned int,int)
int CGuildAgitArea::GetGuildAgitAreaIndex(uint32_t guild_key, int offset) {
    std::map<uint32_t, int>::iterator found = area_by_guild.find(guild_key);
    if (found != area_by_guild.end()) {
        return found->second - offset;
    }
    return -1;
}

// IDA_EA: 0x86D2542
// IDA_NAME: _ZN14CGuildAgitArea20ReleaseGuildAgitAreaEj
// DEMANGLED: CGuildAgitArea::ReleaseGuildAgitArea(unsigned int)
bool CGuildAgitArea::ReleaseGuildAgitArea(uint32_t guild_key) {
    std::map<uint32_t, int>::iterator found = area_by_guild.find(guild_key);
    if (found != area_by_guild.end()) {
        free_areas.push(found->second - 3);
        free_areas.push(found->second - 2);
        free_areas.push(found->second - 1);
        free_areas.push(found->second);
        area_by_guild.erase(found);
        return true;
    }
    return false;
}
