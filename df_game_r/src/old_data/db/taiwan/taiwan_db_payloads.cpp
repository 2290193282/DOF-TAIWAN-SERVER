#include "old_data/db/taiwan/taiwan_db_payloads.h"

#include <cstring>

namespace Taiwan {

// IDA_EA: 0x815DE9C
// IDA_NAME: _ZN6Taiwan18SigLoadGeolocation4initEv
// DEMANGLED: Taiwan::SigLoadGeolocation::init(void)
void* SigLoadGeolocation::init() {
    count = 0;
    return std::memset(country, 0, sizeof(country));
}

// IDA_EA: 0x815DECC
// IDA_NAME: _ZN6Taiwan18SigLoadGeolocation3setEPc
// DEMANGLED: Taiwan::SigLoadGeolocation::set(char *)
SigLoadGeolocation* SigLoadGeolocation::set(char* source) {
    std::strncpy(country[count], source, 15);
    ++count;
    return this;
}

// IDA_EA: 0x815DF06
// IDA_NAME: _ZN6Taiwan18SigLoadGeolocation7setDataERS0_
// DEMANGLED: Taiwan::SigLoadGeolocation::setData(Taiwan::SigLoadGeolocation &)
void* SigLoadGeolocation::setData(SigLoadGeolocation& source) {
    count = source.count;
    return std::memcpy(country, source.country, sizeof(country));
}

// IDA_EA: 0x815DF38
// IDA_NAME: _ZN6Taiwan17SigUpdateExitTime4initEv
// DEMANGLED: Taiwan::SigUpdateExitTime::init(void)
SigUpdateExitTime* SigUpdateExitTime::init() {
    account_id = 0;
    expire_time = 0;
    return this;
}

// IDA_EA: 0x815DF50
// IDA_NAME: _ZN6Taiwan17SigUpdateExitTime3setEjj
// DEMANGLED: Taiwan::SigUpdateExitTime::set(unsigned int,unsigned int)
SigUpdateExitTime* SigUpdateExitTime::set(uint32_t account,
                                           uint32_t timestamp) {
    account_id = account;
    expire_time = timestamp;
    return this;
}

// IDA_EA: 0x81639AC
// IDA_NAME: _ZN6Taiwan22SigSaveHeroMissionDataC2Ev
// DEMANGLED: Taiwan::SigSaveHeroMissionData::SigSaveHeroMissionData(void)
SigSaveHeroMissionData::SigSaveHeroMissionData() {
    init();
}

// IDA_EA: 0x81639C0
// IDA_NAME: _ZN6Taiwan22SigSaveHeroMissionData4initEv
// DEMANGLED: Taiwan::SigSaveHeroMissionData::init(void)
void* SigSaveHeroMissionData::init() {
    charac_no = 0;
    return std::memset(mission, 0, sizeof(mission));
}

// IDA_EA: 0x81639F0
// IDA_NAME: _ZN6Taiwan22SigLoadHeroMissionDataC2Ev
// DEMANGLED: Taiwan::SigLoadHeroMissionData::SigLoadHeroMissionData(void)
SigLoadHeroMissionData::SigLoadHeroMissionData() {
    init();
}

// IDA_EA: 0x8163A04
// IDA_NAME: _ZN6Taiwan22SigLoadHeroMissionData4initEv
// DEMANGLED: Taiwan::SigLoadHeroMissionData::init(void)
void* SigLoadHeroMissionData::init() {
    charac_no = 0;
    return std::memset(mission, 0, sizeof(mission));
}

}  // namespace Taiwan
