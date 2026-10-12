#ifndef OLDSRV_OLD_DATA_LOGIN_GARENA_OPENSSL_RSA_H_
#define OLDSRV_OLD_DATA_LOGIN_GARENA_OPENSSL_RSA_H_

#include <vector>
#include <string>

// IDA class: COpensslRSA @ 0x816C238.  The old object owns one RSA* and is
// deliberately non-copyable; AuthGarenaToken keeps the sole process-global
// instance for the lifetime of df_game_r.
class COpensslRSA {
public:
    COpensslRSA();
    ~COpensslRSA();

    int Init();
    int Init(const char* public_key_path);
    int Encrypt(const char* input, int input_length,
                std::vector<unsigned char>& output);
    int Decrypt(const char* input, int input_length,
                std::vector<unsigned char>& output);

private:
    COpensslRSA(const COpensslRSA&);
    COpensslRSA& operator=(const COpensslRSA&);

    void* rsa_;  // RSA* at old offset +0x0.
};

// IDA_EA: GetAppFolder @ 0x816C715.  The old Garena build returns an empty
// prefix, so configured key paths are resolved relative to the game cwd.
std::string GetAppFolder();

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(COpensslRSA) == 0x04u,
              "COpensslRSA old-server ABI must remain one RSA pointer");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_GARENA_OPENSSL_RSA_H_
