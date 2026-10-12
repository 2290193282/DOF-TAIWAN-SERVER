#ifndef OLDSRV_LOGIN_SECURITY_CARD_H_
#define OLDSRV_LOGIN_SECURITY_CARD_H_

#include <stddef.h>
#include <stdint.h>


namespace WongWork {

// IDA unit: 02_login_account/WongWork_CSecurityCard.
// The storage fields below preserve the old 32-bit object offsets.
class CSecurityCard {
public:
    enum ENUM_REQUEST_STATE {
        REQUEST_STATE_NONE = 0,
    };

    CSecurityCard();
    ~CSecurityCard();

    bool isActivate() const;
    bool isCertified() const;
    int getFailCnt();
    int incCancelCnt();
    int getCancelCnt();
    int getRequestState();
    CSecurityCard* setRequestState(ENUM_REQUEST_STATE state);
    bool isCertFlagOn();
    int getCertCnt();
    CSecurityCard* setCertified(bool certified);
    int incFailCnt();
    CSecurityCard* resetFailCnt();
    CSecurityCard* setActivate(bool active);
    CSecurityCard* setCertFlag(char flag);
    CSecurityCard* init();
    short* generateQuestion(short& first_x,
                            short& first_y,
                            short& second_x,
                            short& second_y);
    CSecurityCard* setSecurityCardInfo(const char* card_data,
                                       bool active,
                                       bool certified,
                                       int fail_count,
                                       int cert_flag,
                                       int cert_count,
                                       char* question);
    bool checkAnswer(unsigned char first, unsigned char second) const;
    CSecurityCard* resetCancelCnt();
    int getCertFlag();

private:
    unsigned char reserved_00[8];       // +0x00
    char card_data_[39];                // +0x08
    unsigned char active_;              // +0x2F
    unsigned char certified_;           // +0x30
    unsigned char cert_flag_;           // +0x31
    unsigned char reserved_32[2];       // +0x32
    int32_t fail_count_;                // +0x34
    int32_t cert_count_;                // +0x38
    unsigned char question_[5];         // +0x3C
    unsigned char reserved_41[3];       // +0x41
    int32_t request_state_;             // +0x44
    int32_t cancel_count_;              // +0x48
};

typedef char CSecurityCard_Size_Must_Be_0x4C[
    (sizeof(CSecurityCard) == 0x4C) ? 1 : -1];

}  // namespace WongWork

#endif  // OLDSRV_LOGIN_SECURITY_CARD_H_
