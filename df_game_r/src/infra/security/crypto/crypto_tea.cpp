#include "infra/security/crypto/crypto_tea.h"

#include <algorithm>
#include <cstring>

#include "infra/security/crypto/crypto_graph_internal.h"


// IDA_EA: 0x809CE50
// IDA_NAME: _ZN11CNCryptoTeaC2Ev
// DEMANGLED: CNCryptoTea::CNCryptoTea(void)
CNCryptoTea::CNCryptoTea() : ICryptoGraph(CRYPTO_GRAPH_TEA) {
    context_ = new crypto_detail::TeaContext();
}

// IDA_EA: 0x809CF76
// IDA_NAME: _ZN11CNCryptoTeaD2Ev
// DEMANGLED: CNCryptoTea::~CNCryptoTea()
// IDA_EA: 0x809D000
// IDA_NAME: _ZN11CNCryptoTeaD0Ev
// DEMANGLED: CNCryptoTea::~CNCryptoTea() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoTea::~CNCryptoTea() {
    delete static_cast<crypto_detail::TeaContext*>(context_);
    context_ = 0;
}

// IDA_EA: 0x809D01E
// IDA_NAME: _ZN11CNCryptoTeaC2ERKS_
// DEMANGLED: CNCryptoTea::CNCryptoTea(CNCryptoTea const&)
CNCryptoTea::CNCryptoTea(const CNCryptoTea& rhs) : ICryptoGraph(rhs) {
    context_ = 0;
}

// IDA_EA: 0x809D042
// IDA_NAME: _ZN11CNCryptoTeaaSERKS_
// DEMANGLED: CNCryptoTea::operator=(CNCryptoTea const&)
CNCryptoTea& CNCryptoTea::operator=(const CNCryptoTea& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809D0C2
// IDA_NAME: _ZN11CNCryptoTea10InitializeEPKhi
// DEMANGLED: CNCryptoTea::Initialize(unsigned char const*,int)
int CNCryptoTea::Initialize(const unsigned char* key, int length) {
    crypto_detail::TeaContext* context =
        static_cast<crypto_detail::TeaContext*>(context_);
    if (context == 0) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (key == 0) {
        return CRYPTO_RESULT_NULL_KEY;
    }
    if (length <= 0) {
        return CRYPTO_RESULT_INVALID_KEY_SIZE;
    }

    key_.assign(key, key + std::min(length, GetKeySize()));
    for (unsigned int index = 0; index < context->key_size; ++index) {
        context->key_bytes[index] = key[index % length];
    }
    for (int word = 0; word < 4; ++word) {
        context->key_words[word] =
            crypto_detail::ReadBE32(context->key_bytes + word * 4);
    }
    std::memset(context->chain0, 0, sizeof(context->chain0));
    std::memcpy(context->chain, context->chain0, sizeof(context->chain));
    context->mode = CRYPTO_MODE_ECB;
    context->padding = 0;
    context->initialized = true;
    mode_ = CRYPTO_MODE_ECB;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809D04A
// IDA_NAME: _ZN11CNCryptoTea7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoTea::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoTea::SetMode(CRYPTO_MODE_TYPE mode) {
    crypto_detail::TeaContext* context =
        static_cast<crypto_detail::TeaContext*>(context_);
    if (context == 0) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (mode == CRYPTO_MODE_ECB || mode == CRYPTO_MODE_CBC ||
        mode == CRYPTO_MODE_OFB) {
        mode_ = mode;
        context->mode = mode;
        return CRYPTO_RESULT_OK;
    }
    return CRYPTO_RESULT_INVALID_MODE;
}

// IDA_EA: 0x809D110
// IDA_NAME: _ZN11CNCryptoTea7EncryptEPhi
// DEMANGLED: CNCryptoTea::Encrypt(unsigned char *,int)
int CNCryptoTea::Encrypt(unsigned char* data, int length) {
    if ((length % GetBlockSize()) != 0) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    return crypto_detail::TransformTea(
        static_cast<crypto_detail::TeaContext*>(context_), data, length, data,
        length, false);
}

// IDA_EA: 0x809D19C
// IDA_NAME: _ZN11CNCryptoTea7DecryptEPhi
// DEMANGLED: CNCryptoTea::Decrypt(unsigned char *,int)
int CNCryptoTea::Decrypt(unsigned char* data, int length) {
    if ((length % GetBlockSize()) != 0) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    return crypto_detail::TransformTea(
        static_cast<crypto_detail::TeaContext*>(context_), data, length, data,
        length, true);
}

// IDA_EA: 0x809D228
// IDA_NAME: _ZN11CNCryptoTea7EncryptEPKhiPhi
// DEMANGLED: CNCryptoTea::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoTea::Encrypt(const unsigned char* input, int input_length,
                         unsigned char* output, int output_capacity) {
    if (context_ == 0) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_INVALID_LENGTH;
    }
    if ((input_length % GetBlockSize()) != 0) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    return crypto_detail::TransformTea(
        static_cast<crypto_detail::TeaContext*>(context_), input, input_length,
        output, output_capacity, false);
}

// IDA_EA: 0x809D2B6
// IDA_NAME: _ZN11CNCryptoTea7DecryptEPKhiPhi
// DEMANGLED: CNCryptoTea::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoTea::Decrypt(const unsigned char* input, int input_length,
                         unsigned char* output, int output_capacity) {
    if (context_ == 0) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_INVALID_LENGTH;
    }
    return crypto_detail::TransformTea(
        static_cast<crypto_detail::TeaContext*>(context_), input, input_length,
        output, output_capacity, true);
}

// IDA_EA: 0x809D312
// IDA_NAME: _ZNK11CNCryptoTea10GetKeySizeEv
// DEMANGLED: CNCryptoTea::GetKeySize(void)const
int CNCryptoTea::GetKeySize() const { return 0x10; }

// IDA_EA: 0x809D31C
// IDA_NAME: _ZNK11CNCryptoTea12GetBlockSizeEv
// DEMANGLED: CNCryptoTea::GetBlockSize(void)const
int CNCryptoTea::GetBlockSize() const { return 16; }
