#include "old_game/player/core/cuser.h"

#include <algorithm>
#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/server_utility.h"
#include "common/manager/system_time.h"

class CRijndael;

extern char* encryptAES_(CRijndael* crypt, char* source, char* destination)
    __asm__("_Z11encryptAES_P9CRijndaelPcS1_");

// IDA_EA: 0x822EFDA
// IDA_NAME: _ZNK8Sanicova4CPad10isActivateEv
// DEMANGLED: Sanicova::CPad::isActivate(void)const
bool Sanicova::CPad::isActivate() const {
    return activated_;
}

// IDA_EA: 0x822EFE6
// IDA_NAME: _ZNK8Sanicova4CPad11isCertifiedEv
// DEMANGLED: Sanicova::CPad::isCertified(void)const
bool Sanicova::CPad::isCertified() const {
    return certified_;
}

// IDA_EA: 0x822EF8E
// IDA_NAME: _ZN8Sanicova4CPad10getFailCntEv
// DEMANGLED: Sanicova::CPad::getFailCnt(void)
int Sanicova::CPad::getFailCnt() {
    return fail_count_;
}

// IDA_EA: 0x822EFC0
// IDA_NAME: _ZNK8Sanicova4CPad15getRequestStateEv
// DEMANGLED: Sanicova::CPad::getRequestState(void)const
int Sanicova::CPad::getRequestState() const {
    return request_state_;
}

// IDA_EA: 0x822EFCC
// IDA_NAME: _ZN8Sanicova4CPad15setRequestStateENS0_18ENUM_REQUEST_STATEE
// DEMANGLED: Sanicova::CPad::setRequestState(Sanicova::CPad::ENUM_REQUEST_STATE)
Sanicova::CPad* Sanicova::CPad::setRequestState(ENUM_REQUEST_STATE state) {
    request_state_ = state;
    return this;
}

// IDA_EA: 0x822EF9A
// IDA_NAME: _ZN8Sanicova4CPad12incCancelCntEv
// DEMANGLED: Sanicova::CPad::incCancelCnt(void)
int Sanicova::CPad::incCancelCnt() {
    return ++cancel_count_;
}

// IDA_EA: 0x822EFB4
// IDA_NAME: _ZN8Sanicova4CPad12getCancelCntEv
// DEMANGLED: Sanicova::CPad::getCancelCnt(void)
int Sanicova::CPad::getCancelCnt() {
    return cancel_count_;
}

// IDA_EA: 0x84EBDFA
// IDA_NAME: _ZN8Sanicova4CPad12setCancelCntEi
// DEMANGLED: Sanicova::CPad::setCancelCnt(int)
Sanicova::CPad* Sanicova::CPad::setCancelCnt(int cancel_count) {
    cancel_count_ = cancel_count;
    return this;
}

// IDA_EA: 0x84EBE08
// IDA_NAME: _ZN8Sanicova4CPad11setActivateEb
// DEMANGLED: Sanicova::CPad::setActivate(bool)
Sanicova::CPad* Sanicova::CPad::setActivate(bool activated) {
    activated_ = activated;
    return this;
}

// IDA_EA: 0x826530E
// IDA_NAME: _ZN8Sanicova4CPad12setCertifiedEb
// DEMANGLED: Sanicova::CPad::setCertified(bool)
Sanicova::CPad* Sanicova::CPad::setCertified(bool certified) {
    certified_ = certified;
    return this;
}

// IDA_EA: 0x82652D8
// IDA_NAME: _ZN8Sanicova4CPad10incFailCntEv
// DEMANGLED: Sanicova::CPad::incFailCnt(void)
int Sanicova::CPad::incFailCnt() {
    return ++fail_count_;
}

// IDA_EA: 0x82652F2
// IDA_NAME: _ZN8Sanicova4CPad12resetFailCntEv
// DEMANGLED: Sanicova::CPad::resetFailCnt(void)
Sanicova::CPad* Sanicova::CPad::resetFailCnt() {
    fail_count_ = 0;
    return this;
}

// IDA_EA: 0x8265302
// IDA_NAME: _ZN8Sanicova4CPad18getAdvideChangePwdEv
// DEMANGLED: Sanicova::CPad::getAdvideChangePwd(void)
int Sanicova::CPad::getAdvideChangePwd() {
    return advise_change_password_ ? 1 : 0;
}

// IDA_EA: 0x8265326
// IDA_NAME: _ZN8Sanicova4CPad11getPasswordEv
// DEMANGLED: Sanicova::CPad::getPassword(void)
char* Sanicova::CPad::getPassword() {
    return encrypted_password_;
}

// IDA_EA: 0x828712C
// IDA_NAME: _ZN8Sanicova4CPad10setFailCntEi
// DEMANGLED: Sanicova::CPad::setFailCnt(int)
Sanicova::CPad* Sanicova::CPad::setFailCnt(int fail_count) {
    fail_count_ = fail_count;
    return this;
}

// IDA_EA: 0x828713A
// IDA_NAME: _ZN8Sanicova4CPad11setPasswordEPc
// DEMANGLED: Sanicova::CPad::setPassword(char *)
char* Sanicova::CPad::setPassword(char* password) {
    return std::strncpy(encrypted_password_, password,
                        sizeof(encrypted_password_));
}

// IDA_EA: 0x84EBDB2
// IDA_NAME: _ZN8Sanicova4CPad18setAdvideChangePwdEb
// DEMANGLED: Sanicova::CPad::setAdvideChangePwd(bool)
Sanicova::CPad* Sanicova::CPad::setAdvideChangePwd(
    bool advise_change_password) {
    advise_change_password_ = advise_change_password;
    return this;
}

// IDA_EA: 0x84EBDCA
// IDA_NAME: _ZN8Sanicova4CPad13getChangedPwdEv
// DEMANGLED: Sanicova::CPad::getChangedPwd(void)
int Sanicova::CPad::getChangedPwd() {
    return changed_password_ ? 1 : 0;
}

// IDA_EA: 0x84EBDD6
// IDA_NAME: _ZN8Sanicova4CPad14setVersionInfoEc
// DEMANGLED: Sanicova::CPad::setVersionInfo(char)
Sanicova::CPad* Sanicova::CPad::setVersionInfo(char version_info) {
    version_info_ = static_cast<uint8_t>(version_info);
    return this;
}

// IDA_EA: 0x84EBDEE
// IDA_NAME: _ZN8Sanicova4CPad14getVersionInfoEv
// DEMANGLED: Sanicova::CPad::getVersionInfo(void)
int Sanicova::CPad::getVersionInfo() {
    return version_info_;
}

// IDA_EA: 0x859984C
// IDA_NAME: _ZN8Sanicova4CPad20GetSelectedCipherIdxEv
// DEMANGLED: Sanicova::CPad::GetSelectedCipherIdx(void)
int Sanicova::CPad::GetSelectedCipherIdx() {
    const uint16_t maximum_cipher_index = 4999u;
    return static_cast<int>(std::min(selected_cipher_idx_,
                                     maximum_cipher_index));
}

// IDA_EA: 0x8599872
// IDA_NAME: _ZN8Sanicova4CPad9GetKeyIdxEi
// DEMANGLED: Sanicova::CPad::GetKeyIdx(int)
int Sanicova::CPad::GetKeyIdx(int key_group) {
    if (key_group) {
        return dword_940BE88[4 * selected_cipher_idx_];
    }
    return dword_940BE84[4 * selected_cipher_idx_];
}

// IDA_EA: 0x85998A6
// IDA_NAME: _ZN8Sanicova4CPad17isValidKeyGenTimeEv
// DEMANGLED: Sanicova::CPad::isValidKeyGenTime(void)
int Sanicova::CPad::isValidKeyGenTime() {
    if (static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec()) -
            key_generation_time_ <=
        20u) {
        return 0;
    }
    key_generation_time_ =
        static_cast<uint32_t>(GlobalData::s_systemTime_.getCurSec());
    return 1;
}

// IDA_EA: 0x85998EA
// IDA_NAME: _ZN8Sanicova4CPad18GenerateRandomDataEv
// DEMANGLED: Sanicova::CPad::GenerateRandomData(void)
int Sanicova::CPad::GenerateRandomData() {
    const int result = isValidKeyGenTime();
    if (static_cast<unsigned char>(result)) {
        GenerateRandomKeyIdx();
        return GenerateRandomShakeData();
    }
    return result;
}

// IDA_EA: 0x8599918
// IDA_NAME: _ZN8Sanicova4CPad20GenerateRandomKeyIdxEv
// DEMANGLED: Sanicova::CPad::GenerateRandomKeyIdx(void)
Sanicova::CPad* Sanicova::CPad::GenerateRandomKeyIdx() {
    selected_cipher_idx_ = static_cast<uint16_t>(get_rand_int(5000));
    return this;
}

// IDA_EA: 0x8599936
// IDA_NAME: _ZN8Sanicova4CPad12GetShakeDataEi
// DEMANGLED: Sanicova::CPad::GetShakeData(int)
int Sanicova::CPad::GetShakeData(int index) {
    return shake_data_[index];
}

// IDA_EA: 0x8599946
// IDA_NAME: _ZN8Sanicova4CPad23GenerateRandomShakeDataEv
// DEMANGLED: Sanicova::CPad::GenerateRandomShakeData(void)
bool Sanicova::CPad::GenerateRandomShakeData() {
    for (int index = 0; index <= 3; ++index) {
        shake_data_[index] =
            static_cast<uint8_t>(get_rand_int(200) + 20u);
    }
    return false;
}

// IDA_EA: 0x8599712
// IDA_NAME: _ZNK8Sanicova4CPad11checkAnswerEPc
// DEMANGLED: Sanicova::CPad::checkAnswer(char *)const
bool Sanicova::CPad::checkAnswer(char* answer) const {
    // The original allocates 36-byte and 44-byte stack buffers, clears them,
    // copies exactly eight source bytes into the latter, then compares the
    // full stored 36-byte encrypted password.  Keep that byte contract: the
    // Pad answer is not a C string protocol field.
    char encrypted_answer[36];
    char plain_answer[44];
    std::memset(plain_answer, 0, sizeof(plain_answer));
    std::memset(encrypted_answer, 0, sizeof(encrypted_answer));
    std::strncpy(plain_answer, answer, 8u);
    encryptAES_(GlobalData::s_pcryptRijndael_MousePassword_, plain_answer,
                encrypted_answer);
    return std::strncmp(encrypted_password_, encrypted_answer,
                        sizeof(encrypted_password_)) == 0;
}

// IDA_EA: 0x85997B8
// IDA_NAME: _ZN8Sanicova4CPad18SendChagePwdRewardEP5CUser
// DEMANGLED: Sanicova::CPad::SendChagePwdReward(CUser *)
int Sanicova::CPad::SendChagePwdReward(CUser* user) {
    const int advised = getAdvideChangePwd();
    if (advised == 0) {
        return advised;
    }

    setAdvideChangePwd(false);
    if (user && user->getCurCharacNo() != 0) {
        return SendChagePwdRewardMail(user) ? 1 : 0;
    }
    return setChangedPwd(true) ? 1 : 0;
}

// IDA_EA: 0x859981E
// IDA_NAME: _ZN8Sanicova4CPad22SendChagePwdRewardMailEP5CUser
// DEMANGLED: Sanicova::CPad::SendChagePwdRewardMail(CUser *)
Sanicova::CPad* Sanicova::CPad::SendChagePwdRewardMail(CUser*) {
    setChangedPwd(false);
    return setAdvideChangePwd(false);
}

// IDA_EA: 0x85999E8
// IDA_NAME: _ZN8Sanicova4CPad13setChangedPwdEb
// DEMANGLED: Sanicova::CPad::setChangedPwd(bool)
Sanicova::CPad* Sanicova::CPad::setChangedPwd(bool changed_password) {
    changed_password_ = changed_password;
    return this;
}

// IDA_EA: 0x859957C
// IDA_NAME: _ZN8Sanicova4CPad4initEv
// DEMANGLED: Sanicova::CPad::init(void)
int Sanicova::CPad::init() {
    activated_ = false;
    certified_ = false;
    std::fill(encrypted_password_,
              encrypted_password_ + sizeof(encrypted_password_), '\0');
    fail_count_ = 0;
    request_state_ = ENUM_REQUEST_STATE_NONE;
    cancel_count_ = 0;
    init_cleared_byte_2c_ = 0;
    version_info_ = 0;
    init_cleared_byte_2d_ = 0;
    check_time_ = GlobalData::s_systemTime_.getCurSec();
    advise_change_password_ = false;
    changed_password_ = false;
    GenerateRandomKeyIdx();
    return GenerateRandomShakeData();
}
