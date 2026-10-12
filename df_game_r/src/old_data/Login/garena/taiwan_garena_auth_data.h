#ifndef OLDSRV_LOGIN_TAIWAN_GARENA_AUTH_DATA_H_
#define OLDSRV_LOGIN_TAIWAN_GARENA_AUTH_DATA_H_

#include <stdint.h>
#include <stddef.h>

class CUser;

namespace Taiwan {

bool GarenaCheckTimeStamp(uint32_t token_time,
                          uint32_t current_time,
                          uint32_t login_time,
                          uint32_t last_play_time);

// IDA_EA: 0x817305A
// IDA_NAME: _ZN6Taiwan15GarenaCyberCafeEt
// DEMANGLED: Taiwan::GarenaCyberCafe(unsigned short)
bool GarenaCyberCafe(uint16_t gca_type);

// IDA_EA: 0x8471C74
// IDA_NAME: _ZN6Taiwan16GarenaAuthPCRoomEP5CUser
// DEMANGLED: Taiwan::GarenaAuthPCRoom(CUser *)
void GarenaAuthPCRoom(CUser* user);

// IDA unit: 02_login_account/Taiwan_GarenaAuthData. Old token payload size:
// 0x68 bytes, copied after RSA public decrypt.
// This token image is embedded at the deliberately unaligned CUser+0x6ef92
// address in the 0627 binary.  Its wire fields already prove byte packing;
// lowering the type alignment to one is therefore part of the original ABI,
// not synthetic padding in CUser.
struct __attribute__((packed)) GarenaAuthData {
    uint32_t getUid();
    char* getUname();
    uint32_t getTimeStamp();
    uint16_t getGcaType();
    bool getMobileAuth();
    GarenaAuthData* setPatchTime(uint32_t patch_time);
    uint32_t getPatchTime();
    int getGoldMemberType();

    uint32_t uid_be;              // +0x00
    char uname[32];               // +0x04
    uint32_t timestamp_be;        // +0x24
    uint16_t gca_type_be;         // +0x28
    unsigned char gold_member;    // +0x2A
    unsigned char pad_2b;         // +0x2B
    unsigned char mobile_auth;    // +0x2C
    unsigned char reserved[55];   // +0x2D
    uint32_t patch_time;          // +0x64
};

static_assert(sizeof(GarenaAuthData) == 0x68u,
              "GarenaAuthData old token payload must stay 0x68 bytes");
static_assert(offsetof(GarenaAuthData, mobile_auth) == 0x2cu,
              "GarenaAuthData mobile-auth flag must stay at +0x2c");
static_assert(offsetof(GarenaAuthData, patch_time) == 0x64u,
              "GarenaAuthData patch time must stay at +0x64");

}  // namespace Taiwan

#endif  // OLDSRV_LOGIN_TAIWAN_GARENA_AUTH_DATA_H_
