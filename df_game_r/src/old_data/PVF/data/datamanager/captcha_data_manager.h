#ifndef OLDSRV_DATAMANAGER_CAPTCHA_DATA_MANAGER_H_
#define OLDSRV_DATAMANAGER_CAPTCHA_DATA_MANAGER_H_

#include <cstddef>
#include <deque>
#include <stdint.h>

#pragma pack(push, 1)

// IDA unit: 01_network_crypto_antibot/MSG_UDP_CAPCHA_CERTIFY.
// GenerateCaptchaData lays out the five UTF-16 challenge characters, the
// 220x60 image byte count, and the image itself contiguously.
struct MSG_UDP_CAPCHA_CERTIFY {
    ~MSG_UDP_CAPCHA_CERTIFY();

    uint16_t challenge[5];
    uint32_t image_size;
    unsigned char image[220 * 60];
};

#pragma pack(pop)

static_assert(sizeof(MSG_UDP_CAPCHA_CERTIFY) == 0x339Eu,
              "captcha message size must stay 0x339e");
static_assert(offsetof(MSG_UDP_CAPCHA_CERTIFY, image_size) == 0x0Au,
              "captcha image-size offset must stay 0x0a");
static_assert(offsetof(MSG_UDP_CAPCHA_CERTIFY, image) == 0x0Eu,
              "captcha image offset must stay 0x0e");

namespace WongWork {

// IDA unit: 12_pvf_datamanager_scripts/WongWork::CCapchaDataManager.
// The original object is a std::deque<MSG_UDP_CAPCHA_CERTIFY> at +0.
class CCapchaDataManager {
public:
    CCapchaDataManager();
    ~CCapchaDataManager();

    // IDA_EA: 0x8299F3A
    bool push(const MSG_UDP_CAPCHA_CERTIFY& data);
    // IDA_EA: 0x8299F74
    MSG_UDP_CAPCHA_CERTIFY* peekData();
    // IDA_EA: 0x86977FA
    bool isEmpty();

private:
    std::deque<MSG_UDP_CAPCHA_CERTIFY> data_;
};

#if defined(__i386__)
static_assert(sizeof(CCapchaDataManager) == 0x28u,
              "captcha manager deque layout must stay 0x28");
#endif

}  // namespace WongWork

#endif  // OLDSRV_DATAMANAGER_CAPTCHA_DATA_MANAGER_H_
