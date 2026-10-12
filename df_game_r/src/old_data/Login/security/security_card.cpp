#include "old_data/Login/security/security_card.h"

#include <string.h>


namespace WongWork {

// IDA_EA: 0x822EF0E
// IDA_NAME: _ZNK8WongWork13CSecurityCard10isActivateEv
// DEMANGLED: WongWork::CSecurityCard::isActivate(void)const
bool CSecurityCard::isActivate() const {
    return active_ != 0;
}

// IDA_EA: 0x822EF1A
// IDA_NAME: _ZNK8WongWork13CSecurityCard11isCertifiedEv
// DEMANGLED: WongWork::CSecurityCard::isCertified(void)const
bool CSecurityCard::isCertified() const {
    return certified_ != 0;
}

// IDA_EA: 0x822EF26
// IDA_NAME: _ZN8WongWork13CSecurityCard10getFailCntEv
// DEMANGLED: WongWork::CSecurityCard::getFailCnt(void)
int CSecurityCard::getFailCnt() {
    return fail_count_;
}

// IDA_EA: 0x822EF32
// IDA_NAME: _ZN8WongWork13CSecurityCard12incCancelCntEv
// DEMANGLED: WongWork::CSecurityCard::incCancelCnt(void)
int CSecurityCard::incCancelCnt() {
    return ++cancel_count_;
}

// IDA_EA: 0x822EF4C
// IDA_NAME: _ZN8WongWork13CSecurityCard12getCancelCntEv
// DEMANGLED: WongWork::CSecurityCard::getCancelCnt(void)
int CSecurityCard::getCancelCnt() {
    return cancel_count_;
}

// IDA_EA: 0x822EF58
// IDA_NAME: _ZN8WongWork13CSecurityCard15getRequestStateEv
// DEMANGLED: WongWork::CSecurityCard::getRequestState(void)
int CSecurityCard::getRequestState() {
    return request_state_;
}

// IDA_EA: 0x822EF64
// IDA_NAME: _ZN8WongWork13CSecurityCard15setRequestStateENS0_18ENUM_REQUEST_STATEE
// DEMANGLED: WongWork::CSecurityCard::setRequestState(WongWork::CSecurityCard::ENUM_REQUEST_STATE)
CSecurityCard* CSecurityCard::setRequestState(ENUM_REQUEST_STATE state) {
    request_state_ = state;
    return this;
}

// IDA_EA: 0x822EF72
// IDA_NAME: _ZN8WongWork13CSecurityCard12isCertFlagOnEv
// DEMANGLED: WongWork::CSecurityCard::isCertFlagOn(void)
bool CSecurityCard::isCertFlagOn() {
    return cert_flag_ == 1;
}

// IDA_EA: 0x826528A
// IDA_NAME: _ZN8WongWork13CSecurityCard10getCertCntEv
// DEMANGLED: WongWork::CSecurityCard::getCertCnt(void)
int CSecurityCard::getCertCnt() {
    return cert_count_;
}

// IDA_EA: 0x8265296
// IDA_NAME: _ZN8WongWork13CSecurityCard12setCertifiedEb
// DEMANGLED: WongWork::CSecurityCard::setCertified(bool)
CSecurityCard* CSecurityCard::setCertified(bool certified) {
    certified_ = certified ? 1 : 0;
    return this;
}

// IDA_EA: 0x82652AE
// IDA_NAME: _ZN8WongWork13CSecurityCard10incFailCntEv
// DEMANGLED: WongWork::CSecurityCard::incFailCnt(void)
int CSecurityCard::incFailCnt() {
    return ++fail_count_;
}

// IDA_EA: 0x82652C8
// IDA_NAME: _ZN8WongWork13CSecurityCard12resetFailCntEv
// DEMANGLED: WongWork::CSecurityCard::resetFailCnt(void)
CSecurityCard* CSecurityCard::resetFailCnt() {
    fail_count_ = 0;
    return this;
}

// IDA_EA: 0x84EBD82
// IDA_NAME: _ZN8WongWork13CSecurityCard11setActivateEb
// DEMANGLED: WongWork::CSecurityCard::setActivate(bool)
CSecurityCard* CSecurityCard::setActivate(bool active) {
    active_ = active ? 1 : 0;
    return this;
}

// IDA_EA: 0x84EBD9A
// IDA_NAME: _ZN8WongWork13CSecurityCard11setCertFlagEc
// DEMANGLED: WongWork::CSecurityCard::setCertFlag(char)
CSecurityCard* CSecurityCard::setCertFlag(char flag) {
    cert_flag_ = static_cast<unsigned char>(flag);
    return this;
}

// IDA_EA: 0x85FDD68
// IDA_NAME: _ZN8WongWork13CSecurityCard4initEv
// DEMANGLED: WongWork::CSecurityCard::init(void)
CSecurityCard* CSecurityCard::init() {
    active_ = 0;
    certified_ = 0;
    fail_count_ = 0;
    request_state_ = REQUEST_STATE_NONE;
    cancel_count_ = 0;
    cert_count_ = 0;
    cert_flag_ = 0;
    question_[4] = 0;
    return this;
}

// IDA_EA: 0x85FDDB2
// IDA_NAME: _ZN8WongWork13CSecurityCard16generateQuestionERsS1_S1_S1_
// DEMANGLED: WongWork::CSecurityCard::generateQuestion(short &,short &,short &,short &)
short* CSecurityCard::generateQuestion(short& first_x,
                                       short& first_y,
                                       short& second_x,
                                       short& second_y) {
    if ((!question_[0] && !question_[1] && !question_[2] && !question_[3]) ||
        question_[0] > 9 || question_[1] >= 4 || question_[2] > 9 ||
        question_[3] >= 4) {
        question_[0] = 0;
        question_[1] = 0;
        question_[2] = 1;
        question_[3] = 1;
    }
    first_x = static_cast<signed char>(question_[0]);
    first_y = static_cast<signed char>(question_[1]);
    second_x = static_cast<signed char>(question_[2]);
    second_y = static_cast<signed char>(question_[3]);
    return &second_y;
}

// IDA_EA: 0x85FDE9C
// IDA_NAME: _ZN8WongWork13CSecurityCard19setSecurityCardInfoEPKcbbiiiPc
// DEMANGLED: WongWork::CSecurityCard::setSecurityCardInfo(char const*,bool,bool,int,int,int,char *)
CSecurityCard* CSecurityCard::setSecurityCardInfo(const char* card_data,
                                                   bool active,
                                                   bool certified,
                                                   int fail_count,
                                                   int cert_flag,
                                                   int cert_count,
                                                   char* question) {
    ::strncpy(card_data_, card_data, sizeof(card_data_));
    active_ = active ? 1 : 0;
    certified_ = certified ? 1 : 0;
    fail_count_ = fail_count;
    cert_flag_ = static_cast<unsigned char>(cert_flag);
    cert_count_ = cert_count;
    if (question) {
        ::strncpy(reinterpret_cast<char*>(question_), question,
                  sizeof(question_));
        question_[0] = static_cast<unsigned char>(question_[0] - '0');
        question_[1] = static_cast<unsigned char>(question_[1] - '0');
        question_[2] = static_cast<unsigned char>(question_[2] - '0');
        question_[3] = static_cast<unsigned char>(question_[3] - '0');
    }
    return this;
}

// IDA_EA: 0x85FDF6A
// IDA_NAME: _ZNK8WongWork13CSecurityCard11checkAnswerEhh
// DEMANGLED: WongWork::CSecurityCard::checkAnswer(unsigned char,unsigned char)const
bool CSecurityCard::checkAnswer(unsigned char first, unsigned char second) const {
    if (!question_[0] && !question_[1] && !question_[2] && !question_[3]) {
        return false;
    }
    return static_cast<unsigned char>(
               card_data_[10 * question_[1] + question_[0]]) == first &&
           static_cast<unsigned char>(
               card_data_[10 * question_[3] + question_[2]]) == second;
}

// IDA_EA: 0x86959A8
// IDA_NAME: _ZN8WongWork13CSecurityCard14resetCancelCntEv
// DEMANGLED: WongWork::CSecurityCard::resetCancelCnt(void)
CSecurityCard* CSecurityCard::resetCancelCnt() {
    cancel_count_ = 0;
    return this;
}

// IDA_EA: 0x86959B8
// IDA_NAME: _ZN8WongWork13CSecurityCard11getCertFlagEv
// DEMANGLED: WongWork::CSecurityCard::getCertFlag(void)
int CSecurityCard::getCertFlag() {
    return cert_flag_;
}

// IDA_EA: 0x86959C4
// IDA_NAME: _ZN8WongWork13CSecurityCardC2Ev
// DEMANGLED: WongWork::CSecurityCard::CSecurityCard(void)
CSecurityCard::CSecurityCard() {}

// IDA_EA: 0x86959CA
// IDA_NAME: _ZN8WongWork13CSecurityCardD2Ev
// DEMANGLED: WongWork::CSecurityCard::~CSecurityCard()
CSecurityCard::~CSecurityCard() {}

}  // namespace WongWork
