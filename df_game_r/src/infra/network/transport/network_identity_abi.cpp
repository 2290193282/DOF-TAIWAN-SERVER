#include "infra/network/transport/network_identity_abi.h"

#include <cstring>

// IDA_EA: 0x808CA12
// IDA_NAME: _Z34_NS_PI_2ND_GetDefaultRandomHashKeyv
// DEMANGLED: _NS_PI_2ND_GetDefaultRandomHashKey(void)
int _NS_PI_2ND_GetDefaultRandomHashKey() {
    return 7;
}

// IDA_EA: 0x81F440A
// IDA_NAME: _Z11cutIPdclassPKc
// DEMANGLED: cutIPdclass(char const*)
std::string cutIPdclass(const char* ip) {
    std::string result(ip);
    result.erase(result.rfind('.'), std::string::npos);
    return result;
}

// IDA_EA: 0x81F44BB
// IDA_NAME: _Z11checkDenyIPPKcS0_
// DEMANGLED: checkDenyIP(char const*,char const*)
int checkDenyIP(const char* first_ip, const char* second_ip) {
    if (first_ip == 0 || second_ip == 0) {
        return 0;
    }
    if (*first_ip == 0 || *second_ip == 0) {
        return 0;
    }
    if (std::strncmp(first_ip, second_ip, 0x10u) == 0) {
        return 0;
    }

    std::string first_dclass = cutIPdclass(first_ip);
    std::string second_dclass = cutIPdclass(second_ip);
    if (first_dclass == second_dclass) {
        return 0;
    }

    first_dclass.erase(first_dclass.find('.'), std::string::npos);
    second_dclass.erase(second_dclass.find('.'), std::string::npos);
    return first_dclass != "0" && second_dclass != "0";
}
