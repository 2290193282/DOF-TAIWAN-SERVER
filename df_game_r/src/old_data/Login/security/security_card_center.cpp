#include "old_data/Login/security/security_card_center.h"

#include "core/config/env/environment.h"
#include "common/manager/dnff_lib.h"
#include "common/manager/global_data.h"
#include "common/manager/server_utility.h"
#include "common/manager/system_time.h"

#include <cstdio>
#include <cstring>

namespace {

const char kSecurityCardCharacterSet[] =
    "+<>?@#$&ABCDEFGHIJKLMNPQRSTUVWXYZ123456789";
const int kSecurityCardBlockSize = 16;
const uint16_t kSecurityCardTimeCountLimit = 0xC34Fu;

}  // namespace

namespace WongWork {

// IDA_EA: 0x82A6654
// IDA_NAME: _ZN8WongWork19CSecurityCardCenterC2Ev
// DEMANGLED: WongWork::CSecurityCardCenter::CSecurityCardCenter(void)
CSecurityCardCenter::CSecurityCardCenter()
    : CRijndael(), character_set_() {
    // Raw C2 does not initialize the scalar at +0x484; init() sets it to 1.
}

// IDA_EA: 0x85FE026
// IDA_NAME: _ZN8WongWork19CSecurityCardCenter16_updateTimeCountEv
// DEMANGLED: WongWork::CSecurityCardCenter::_updateTimeCount(void)
CSecurityCardCenter* CSecurityCardCenter::_updateTimeCount() {
    ++time_count_;
    if (time_count_ > kSecurityCardTimeCountLimit) {
        time_count_ = 1;
    }
    return this;
}

// IDA_EA: 0x85FE05E
// IDA_NAME: _ZN8WongWork19CSecurityCardCenter4initEv
// DEMANGLED: WongWork::CSecurityCardCenter::init(void)
CSecurityCardCenter* CSecurityCardCenter::init() {
    CRijndael::Initialize("n6;fw;vjfs0fuguf", 16, CRijndael::sm_chain0, 16,
                          kSecurityCardBlockSize, 0, 0);
    // The raw body appends without clearing.  GlobalData calls this exactly
    // once after C2, so repeated direct calls intentionally retain that old
    // append behavior.
    for (unsigned int index = 0;
         index < sizeof(kSecurityCardCharacterSet) - 1;
         ++index) {
        character_set_.push_back(
            static_cast<unsigned char>(kSecurityCardCharacterSet[index]));
    }
    time_count_ = 1;
    return this;
}

// IDA_EA: 0x85FE0EC
// IDA_NAME: _ZN8WongWork19CSecurityCardCenter17generateServerKeyEPc
// DEMANGLED: WongWork::CSecurityCardCenter::generateServerKey(char *)
char* CSecurityCardCenter::generateServerKey(char* destination) {
    _updateTimeCount();
    CEnvironment* const environment = G_CEnvironment();
    const int random_value = static_cast<int>(get_rand_int(99999));
    const int time_count = static_cast<int>(time_count_);
    const int current_second = GlobalData::system_time()->getCurSec();

    std::snprintf(destination, 0x21u, "%02d%02d%010d%05d%05d",
                  static_cast<int>(
                      environment->get_accept_handshake_flag_888()),
                  static_cast<int>(
                      environment->get_accept_handshake_flag_432()),
                  current_second, time_count, random_value);
    return destination;
}

// IDA_EA: 0x85FE17E
// IDA_NAME: _ZN8WongWork19CSecurityCardCenter15generateCertKeyEPc
// DEMANGLED: WongWork::CSecurityCardCenter::generateCertKey(char *)
char* CSecurityCardCenter::generateCertKey(char* destination) {
    const int random_value = static_cast<int>(get_rand_int(89999)) + 10000;
    std::snprintf(destination, 6u, "%d", random_value);
    return destination;
}

// IDA_EA: 0x85FE1BA
// IDA_NAME: _ZNK8WongWork19CSecurityCardCenter16generateCardDataEPc
// DEMANGLED: WongWork::CSecurityCardCenter::generateCardData(char *)const
char* CSecurityCardCenter::generateCardData(char* destination) const {
    std::list<unsigned char> remaining(character_set_);
    for (unsigned int output_index = 0; output_index <= 0x25u;
         ++output_index) {
        const int selected_index =
            static_cast<int>(get_rand_int(static_cast<int>(remaining.size())));
        int current_index = 0;
        for (std::list<unsigned char>::iterator iterator = remaining.begin();
             iterator != remaining.end(); ++iterator) {
            if (current_index == selected_index) {
                destination[output_index] = static_cast<char>(*iterator);
                remaining.erase(iterator);
                break;
            }
            ++current_index;
        }
    }
    return destination;
}

// IDA_EA: 0x85FE2E2
// IDA_NAME: _ZN8WongWork19CSecurityCardCenter13encryptStringEPKcPc
// DEMANGLED: WongWork::CSecurityCardCenter::encryptString(char const*,char *)
char* CSecurityCardCenter::encryptString(const char* source, char* destination) {
    char encrypted[256];
    char plain_text[256];

    std::memset(plain_text, 0, sizeof(plain_text));
    std::memset(encrypted, 0, sizeof(encrypted));
    std::strncpy(plain_text, source, sizeof(plain_text));

    const int source_length = static_cast<int>(std::strlen(plain_text));
    int encrypted_length = source_length;
    if (source_length % kSecurityCardBlockSize) {
        encrypted_length = kSecurityCardBlockSize *
                           (source_length / kSecurityCardBlockSize + 1);
        // IMethod::Pad with padding mode 0 (the exact mode set by init())
        // fills only the trailing part of this final block with zero bytes.
        std::memset(plain_text + source_length, 0,
                    static_cast<unsigned int>(encrypted_length - source_length));
    }

    CRijndael::Encrypt(plain_text, encrypted,
                       static_cast<unsigned int>(encrypted_length));
    DNFFLib::Binary2Hex(
        reinterpret_cast<const unsigned char*>(encrypted), encrypted_length,
        destination);
    // The original writes one byte beyond Binary2Hex's normal terminator.
    destination[2 * encrypted_length + 1] = '\0';
    return destination;
}

// IDA_EA: 0x85FE444
// IDA_NAME: _ZNK8WongWork19CSecurityCardCenter16checkPhoneNumberEPKc
// DEMANGLED: WongWork::CSecurityCardCenter::checkPhoneNumber(char const*)const
int CSecurityCardCenter::checkPhoneNumber(const char* source) const {
    for (unsigned int index = 0; index <= 0xBu && source[index]; ++index) {
        if (static_cast<unsigned int>(source[index] - '0') > 9u) {
            return 0;
        }
    }
    return 1;
}

}  // namespace WongWork
