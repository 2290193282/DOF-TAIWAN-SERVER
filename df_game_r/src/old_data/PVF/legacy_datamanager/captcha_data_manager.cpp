#include "old_data/PVF/data/datamanager/captcha_data_manager.h"

#include "common/manager/global_data.h"
#include "common/manager/server_utility.h"

// IDA_EA: 0x82BCDA4
// IDA_NAME: _ZN22MSG_UDP_CAPCHA_CERTIFYD2Ev
// DEMANGLED: MSG_UDP_CAPCHA_CERTIFY::~MSG_UDP_CAPCHA_CERTIFY()
MSG_UDP_CAPCHA_CERTIFY::~MSG_UDP_CAPCHA_CERTIFY() {}

namespace WongWork {

// IDA_EA: 0x82A7002
// IDA_NAME: _ZN8WongWork18CCapchaDataManagerC2Ev
// DEMANGLED: WongWork::CCapchaDataManager::CCapchaDataManager(void)
CCapchaDataManager::CCapchaDataManager() : data_() {}

// IDA_EA: 0x82A7016
// IDA_NAME: _ZN8WongWork18CCapchaDataManagerD2Ev
// DEMANGLED: WongWork::CCapchaDataManager::~CCapchaDataManager()
CCapchaDataManager::~CCapchaDataManager() {}

// IDA_EA: 0x8299F3A
// IDA_NAME: _ZN8WongWork18CCapchaDataManager4pushERK22MSG_UDP_CAPCHA_CERTIFY
// DEMANGLED: WongWork::CCapchaDataManager::push(MSG_UDP_CAPCHA_CERTIFY const&)
bool CCapchaDataManager::push(const MSG_UDP_CAPCHA_CERTIFY& data) {
    data_.push_back(data);
    const bool trim = data_.size() > 40u;
    if (trim) {
        data_.pop_front();
    }
    return trim;
}

// IDA_EA: 0x8299F74
// IDA_NAME: _ZN8WongWork18CCapchaDataManager8peekDataEv
// DEMANGLED: WongWork::CCapchaDataManager::peekData(void)
MSG_UDP_CAPCHA_CERTIFY* CCapchaDataManager::peekData() {
    const unsigned int index =
        get_rand_int(static_cast<int>(data_.size()));
    return &data_.at(index);
}

// IDA_EA: 0x86977FA
// IDA_NAME: _ZN8WongWork18CCapchaDataManager7isEmptyEv
// DEMANGLED: WongWork::CCapchaDataManager::isEmpty(void)
bool CCapchaDataManager::isEmpty() {
    return data_.empty();
}

}  // namespace WongWork

// IDA_DATA: _ZN10GlobalData19s_capchaDataManagerE
// DEMANGLED: GlobalData::s_capchaDataManager
WongWork::CCapchaDataManager GlobalData::s_capchaDataManager;
