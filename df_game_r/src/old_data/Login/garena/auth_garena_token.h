#ifndef OLDSRV_OLD_DATA_LOGIN_GARENA_AUTH_GARENA_TOKEN_H_
#define OLDSRV_OLD_DATA_LOGIN_GARENA_AUTH_GARENA_TOKEN_H_

#include "old_data/Login/garena/openssl_rsa.h"
#include "old_data/Login/garena/taiwan_garena_auth_data.h"

namespace Taiwan {

// IDA unit: Taiwan_AuthGarenaToken.cpp.  This is a namespace-only holder for
// the original process-global RSA object and its token conversion entrypoints.
class AuthGarenaToken {
public:
    static int init();
    static int init(const char* public_key_path);
    static int decrypt(const char* token, int token_length,
                       GarenaAuthData& output);

private:
    static COpensslRSA openSSLRSA_;
};

}  // namespace Taiwan

// IDA_EA: base64_decode @ 0x807EBF4
// IDA_NAME: _Z13base64_decodeRKSsRSt6vectorIhSaIhEE
bool base64_decode(const std::string& input,
                   std::vector<unsigned char>& output);

#endif  // OLDSRV_OLD_DATA_LOGIN_GARENA_AUTH_GARENA_TOKEN_H_
