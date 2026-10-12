#include "old_data/Login/garena/auth_garena_token.h"

#include "infra/log/format/clog.h"

#include <cctype>
#include <cstring>
#include <dlfcn.h>
#include <string>

namespace {

// The i386 CentOS 6 runtime supplies libcrypto.so.10 but not the matching
// development headers.  These declarations preserve the old OpenSSL C ABI;
// the original calls below and padding value are from 0x816C3A2/0x816C532.
struct OpenSslApi {
    typedef void* (*BioSFileFn)();
    typedef void* (*BioNewFn)(void*);
    typedef long (*BioCtrlFn)(void*, int, long, void*);
    typedef void* (*PemReadPublicKeyFn)(void*, void**, void*, void*);
    typedef void (*BioFreeAllFn)(void*);
    typedef void (*RsaFreeFn)(void*);
    typedef int (*RsaSizeFn)(const void*);
    typedef int (*RsaCryptFn)(int, const unsigned char*, unsigned char*,
                              void*, int);

    void* handle;
    BioSFileFn bio_s_file;
    BioNewFn bio_new;
    BioCtrlFn bio_ctrl;
    PemReadPublicKeyFn pem_read_bio_rsa_pubkey;
    BioFreeAllFn bio_free_all;
    RsaFreeFn rsa_free;
    RsaSizeFn rsa_size;
    RsaCryptFn rsa_private_encrypt;
    RsaCryptFn rsa_public_decrypt;

    OpenSslApi()
        : handle(0),
          bio_s_file(0),
          bio_new(0),
          bio_ctrl(0),
          pem_read_bio_rsa_pubkey(0),
          bio_free_all(0),
          rsa_free(0),
          rsa_size(0),
          rsa_private_encrypt(0),
          rsa_public_decrypt(0) {}

    bool load() {
        if (handle != 0) {
            return bio_s_file != 0 && bio_new != 0 && bio_ctrl != 0 &&
                   pem_read_bio_rsa_pubkey != 0 && bio_free_all != 0 &&
                   rsa_free != 0 && rsa_size != 0 && rsa_private_encrypt != 0 &&
                   rsa_public_decrypt != 0;
        }

        handle = dlopen("libcrypto.so.10", RTLD_NOW | RTLD_GLOBAL);
        if (handle == 0) {
            handle = dlopen("libcrypto.so", RTLD_NOW | RTLD_GLOBAL);
        }
        if (handle == 0) {
            return false;
        }

        bio_s_file = reinterpret_cast<BioSFileFn>(dlsym(handle, "BIO_s_file"));
        bio_new = reinterpret_cast<BioNewFn>(dlsym(handle, "BIO_new"));
        bio_ctrl = reinterpret_cast<BioCtrlFn>(dlsym(handle, "BIO_ctrl"));
        pem_read_bio_rsa_pubkey = reinterpret_cast<PemReadPublicKeyFn>(
            dlsym(handle, "PEM_read_bio_RSA_PUBKEY"));
        bio_free_all =
            reinterpret_cast<BioFreeAllFn>(dlsym(handle, "BIO_free_all"));
        rsa_free = reinterpret_cast<RsaFreeFn>(dlsym(handle, "RSA_free"));
        rsa_size = reinterpret_cast<RsaSizeFn>(dlsym(handle, "RSA_size"));
        rsa_private_encrypt = reinterpret_cast<RsaCryptFn>(
            dlsym(handle, "RSA_private_encrypt"));
        rsa_public_decrypt = reinterpret_cast<RsaCryptFn>(
            dlsym(handle, "RSA_public_decrypt"));
        return bio_s_file != 0 && bio_new != 0 && bio_ctrl != 0 &&
               pem_read_bio_rsa_pubkey != 0 && bio_free_all != 0 &&
               rsa_free != 0 && rsa_size != 0 && rsa_private_encrypt != 0 &&
               rsa_public_decrypt != 0;
    }
};

OpenSslApi& GetOpenSslApi() {
    static OpenSslApi api;
    return api;
}

bool is_base64(unsigned char value) {
    // IDA_EA: 0x807E9F8
    return std::isalnum(static_cast<unsigned char>(value)) != 0 ||
           value == '+' || value == '/';
}

const char kBase64Characters[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

}  // namespace

// IDA_EA: GetAppFolder @ 0x816C715
// IDA_NAME: _Z12GetAppFolderv
std::string GetAppFolder() {
    return std::string();
}

// IDA_EA: COpensslRSA::C2 @ 0x816C238
// IDA_NAME: _ZN11COpensslRSAC2Ev
COpensslRSA::COpensslRSA() : rsa_(0) {}

// IDA_EA: COpensslRSA::D2 @ 0x816C246
// IDA_NAME: _ZN11COpensslRSAD2Ev
COpensslRSA::~COpensslRSA() {
    if (rsa_ != 0) {
        OpenSslApi& api = GetOpenSslApi();
        if (api.load()) {
            api.rsa_free(rsa_);
        }
        rsa_ = 0;
    }
}

// IDA_EA: COpensslRSA::Init @ 0x816C26E
// IDA_NAME: _ZN11COpensslRSA4InitEv
int COpensslRSA::Init() {
    return Init("publickey.pem");
}

// IDA_EA: COpensslRSA::Init @ 0x816C28A
// IDA_NAME: _ZN11COpensslRSA4InitEPKc
int COpensslRSA::Init(const char* public_key_path) {
    if (public_key_path == 0) {
        return 0;
    }

    OpenSslApi& api = GetOpenSslApi();
    if (!api.load()) {
        return 0;
    }

    void* const bio = api.bio_new(api.bio_s_file());
    const std::string full_path = GetAppFolder() + public_key_path;
    api.bio_ctrl(bio, 108, 3, const_cast<char*>(full_path.c_str()));
    rsa_ = api.pem_read_bio_rsa_pubkey(bio, 0, 0, 0);
    api.bio_free_all(bio);
    return rsa_ != 0;
}

// IDA_EA: COpensslRSA::Encrypt @ 0x816C3A2
// IDA_NAME: _ZN11COpensslRSA7EncryptEPKciRSt6vectorIhSaIhEE
int COpensslRSA::Encrypt(const char* input,
                         int input_length,
                         std::vector<unsigned char>& output) {
    OpenSslApi& api = GetOpenSslApi();
    if (rsa_ == 0 || !api.load()) {
        return 0;
    }

    const int block_size = api.rsa_size(rsa_);
    const int source_block_size = block_size - 12;
    int consumed = 0;
    int current_size = input_length - consumed;
    if (current_size > source_block_size) {
        current_size = source_block_size;
    }

    while (current_size > 0) {
        std::vector<unsigned char> encrypted(block_size, 0);
        const int result = api.rsa_private_encrypt(
            current_size,
            reinterpret_cast<const unsigned char*>(input + consumed),
            &encrypted[0],
            rsa_,
            1);  // RSA_PKCS1_PADDING, immediate 1 at 0x816C42E/0x816C5B5.
        if (result <= 0) {
            return 0;
        }
        output.insert(output.end(), encrypted.begin(), encrypted.begin() + result);
        consumed += current_size;
        current_size = input_length - consumed;
        if (current_size > source_block_size) {
            current_size = source_block_size;
        }
    }
    return 1;
}

// IDA_EA: COpensslRSA::Decrypt @ 0x816C532
// IDA_NAME: _ZN11COpensslRSA7DecryptEPKciRSt6vectorIhSaIhEE
int COpensslRSA::Decrypt(const char* input,
                         int input_length,
                         std::vector<unsigned char>& output) {
    OpenSslApi& api = GetOpenSslApi();
    if (rsa_ == 0 || !api.load()) {
        return 0;
    }

    const int block_size = api.rsa_size(rsa_);
    int consumed = 0;
    int current_size = input_length - consumed;
    if (current_size > block_size) {
        current_size = block_size;
    }

    while (current_size > 0) {
        std::vector<unsigned char> decrypted(block_size, 0);
        const int result = api.rsa_public_decrypt(
            current_size,
            reinterpret_cast<const unsigned char*>(input + consumed),
            &decrypted[0],
            rsa_,
            1);  // RSA_PKCS1_PADDING, immediate 1 at 0x816C5B5.
        if (result <= 0) {
            return 0;
        }
        output.insert(output.end(), decrypted.begin(), decrypted.begin() + result);
        consumed += current_size;
        current_size = input_length - consumed;
        if (current_size > block_size) {
            current_size = block_size;
        }
    }
    return 1;
}

// IDA_EA: base64_decode @ 0x807EBF4
// IDA_NAME: _Z13base64_decodeRKSsRSt6vectorIhSaIhEE
bool base64_decode(const std::string& input,
                   std::vector<unsigned char>& output) {
    int input_length = static_cast<int>(input.size());
    int index = 0;
    int block_length = 0;
    unsigned char block4[4];
    unsigned char block3[3];

    while (input_length-- && input[index] != '=' &&
           is_base64(static_cast<unsigned char>(input[index]))) {
        block4[block_length++] = static_cast<unsigned char>(input[index++]);
        if (block_length == 4) {
            for (int i = 0; i < 4; ++i) {
                block4[i] = static_cast<unsigned char>(
                    std::string(kBase64Characters).find(block4[i]));
            }
            block3[0] = static_cast<unsigned char>(
                (block4[0] << 2) + ((block4[1] & 0x30) >> 4));
            block3[1] = static_cast<unsigned char>(
                ((block4[1] & 0x0F) << 4) + ((block4[2] & 0x3C) >> 2));
            block3[2] = static_cast<unsigned char>(
                ((block4[2] & 0x03) << 6) + block4[3]);
            output.push_back(block3[0]);
            output.push_back(block3[1]);
            output.push_back(block3[2]);
            block_length = 0;
        }
    }

    if (block_length != 0) {
        for (int i = block_length; i < 4; ++i) {
            block4[i] = 0;
        }
        for (int i = 0; i < 4; ++i) {
            block4[i] = static_cast<unsigned char>(
                std::string(kBase64Characters).find(block4[i]));
        }
        block3[0] = static_cast<unsigned char>(
            (block4[0] << 2) + ((block4[1] & 0x30) >> 4));
        block3[1] = static_cast<unsigned char>(
            ((block4[1] & 0x0F) << 4) + ((block4[2] & 0x3C) >> 2));
        block3[2] = static_cast<unsigned char>(
            ((block4[2] & 0x03) << 6) + block4[3]);
        for (int i = 0; i < block_length - 1; ++i) {
            output.push_back(block3[i]);
        }
    }
    return true;
}

namespace Taiwan {

COpensslRSA AuthGarenaToken::openSSLRSA_;

// IDA_EA: 0x81730C2
// IDA_NAME: _ZN6Taiwan15AuthGarenaToken4initEv
// DEMANGLED: Taiwan::AuthGarenaToken::init(void)
int AuthGarenaToken::init() {
    return openSSLRSA_.Init();
}

// IDA_EA: 0x81730D6
// IDA_NAME: _ZN6Taiwan15AuthGarenaToken4initEPKc
// DEMANGLED: Taiwan::AuthGarenaToken::init(char const*)
int AuthGarenaToken::init(const char* public_key_path) {
    if (std::string(public_key_path) == std::string()) {
        return openSSLRSA_.Init();
    }
    return openSSLRSA_.Init(public_key_path);
}

// IDA_EA: 0x81731F2
// IDA_NAME: _ZN6Taiwan15AuthGarenaToken7decryptEPKciRNS_14GarenaAuthDataE
// DEMANGLED: Taiwan::AuthGarenaToken::decrypt(char const*,int,Taiwan::GarenaAuthData &)
int AuthGarenaToken::decrypt(const char* token,
                             int,
                             GarenaAuthData& output) {
    if (token == 0) {
        CLog::instance()->logError("[Taiwan, Auth] Token is null!");
        return 123;
    }

    std::vector<unsigned char> encrypted;
    base64_decode(std::string(token), encrypted);
    if (encrypted.empty()) {
        CLog::instance()->logError("[Taiwan, Auth] Invalid token format!");
        return 132;
    }

    std::vector<unsigned char> decrypted;
    if (openSSLRSA_.Decrypt(
            reinterpret_cast<const char*>(&encrypted[0]),
            static_cast<int>(encrypted.size()),
            decrypted) != 1) {
        CLog::instance()->logError("[Taiwan, Auth] Decrypt token failed!");
        return 140;
    }

    if (decrypted.size() > 0x68u) {
        CLog::instance()->logError(
            "[Taiwan, Auth] Invalid decrypted token length! (buffer_size:%u, token_size:%u)",
            0x68u,
            static_cast<unsigned int>(decrypted.size()));
        return 147;
    }

    std::memset(&output, 0, 0x68u);
    std::memcpy(&output, &decrypted[0], decrypted.size() - 1u);
    CLog::instance()->logNotice(
        "[Taiwan, Auth] Success!! Uid:%u, UserName:%s, Timestamp:%u, GcaType:%u, GoldMemberType:%u, Mobile:%u",
        output.getUid(),
        output.getUname(),
        output.getTimeStamp(),
        static_cast<unsigned int>(output.getGcaType()),
        static_cast<unsigned int>(output.getGoldMemberType()),
        static_cast<unsigned int>(output.getMobileAuth()));
    return 0;
}

}  // namespace Taiwan
