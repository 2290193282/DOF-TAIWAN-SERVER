#ifndef OLDSRV_GLOBAL_NETWORK_IDENTITY_ABI_H_
#define OLDSRV_GLOBAL_NETWORK_IDENTITY_ABI_H_

#include <string>

// IDA_EA: 0x81F440A
// IDA_NAME: _Z11cutIPdclassPKc
// DEMANGLED: cutIPdclass(char const*)
std::string cutIPdclass(const char* ip);

// IDA_EA: 0x81F44BB
// IDA_NAME: _Z11checkDenyIPPKcS0_
// DEMANGLED: checkDenyIP(char const*,char const*)
int checkDenyIP(const char* first_ip, const char* second_ip);

#endif  // OLDSRV_GLOBAL_NETWORK_IDENTITY_ABI_H_
