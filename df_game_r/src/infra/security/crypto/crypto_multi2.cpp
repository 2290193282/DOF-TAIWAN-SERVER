#include "infra/security/crypto/crypto_multi2.h"

#include "infra/security/crypto/crypto_graph_internal.h"


// IDA_EA: 0x809AFF8
// IDA_NAME: _ZN14CNCryptoMulti2C2Ev
// DEMANGLED: CNCryptoMulti2::CNCryptoMulti2(void)
CNCryptoMulti2::CNCryptoMulti2() : ICryptoGraph(CRYPTO_GRAPH_MULTI2) {
    context_ = new crypto_detail::Multi2Context();
}

// IDA_EA: 0x809B096
// IDA_NAME: _ZN14CNCryptoMulti2D2Ev
// DEMANGLED: CNCryptoMulti2::~CNCryptoMulti2()
// IDA_EA: 0x809B0EA
// IDA_NAME: _ZN14CNCryptoMulti2D0Ev
// DEMANGLED: CNCryptoMulti2::~CNCryptoMulti2() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoMulti2::~CNCryptoMulti2() {
    delete static_cast<crypto_detail::Multi2Context*>(context_);
    context_ = 0;
}

// IDA_EA: 0x809B108
// IDA_NAME: _ZN14CNCryptoMulti2C2ERKS_
// DEMANGLED: CNCryptoMulti2::CNCryptoMulti2(CNCryptoMulti2 const&)
CNCryptoMulti2::CNCryptoMulti2(const CNCryptoMulti2& rhs)
    : ICryptoGraph(rhs) {
    context_ = 0;
}

// IDA_EA: 0x809B12C
// IDA_NAME: _ZN14CNCryptoMulti2aSERKS_
// DEMANGLED: CNCryptoMulti2::operator=(CNCryptoMulti2 const&)
CNCryptoMulti2& CNCryptoMulti2::operator=(const CNCryptoMulti2& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809B14C
// IDA_NAME: _ZN14CNCryptoMulti210InitializeEPKhi
// DEMANGLED: CNCryptoMulti2::Initialize(unsigned char const*,int)
int CNCryptoMulti2::Initialize(const unsigned char* key, int length) {
    crypto_detail::Multi2Context* context =
        static_cast<crypto_detail::Multi2Context*>(context_);
    if (context == 0) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if (key == 0 || length < GetKeySize()) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }

    key_.assign(key, key + GetKeySize());
    unsigned int raw_key[8];
    unsigned int seed[2];
    for (int index = 0; index < 8; ++index) {
        raw_key[index] = crypto_detail::ReadBE32(key + index * 4);
    }
    seed[0] = crypto_detail::ReadBE32(key + 32);
    seed[1] = crypto_detail::ReadBE32(key + 36);
    context->rounds = 128;
    crypto_detail::Multi2BuildSchedule(seed, raw_key, context->subkeys);
    context->initialized = true;
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809B1F4
// IDA_NAME: _ZN14CNCryptoMulti27EncryptEPhi
// DEMANGLED: CNCryptoMulti2::Encrypt(unsigned char *,int)
int CNCryptoMulti2::Encrypt(unsigned char* data, int length) {
    return crypto_detail::TransformMulti2(
        static_cast<crypto_detail::Multi2Context*>(context_), data, length, data,
        length, false);
}

// IDA_EA: 0x809B2CC
// IDA_NAME: _ZN14CNCryptoMulti27DecryptEPhi
// DEMANGLED: CNCryptoMulti2::Decrypt(unsigned char *,int)
int CNCryptoMulti2::Decrypt(unsigned char* data, int length) {
    return crypto_detail::TransformMulti2(
        static_cast<crypto_detail::Multi2Context*>(context_), data, length, data,
        length, true);
}

// IDA_EA: 0x809B3A4
// IDA_NAME: _ZN14CNCryptoMulti27EncryptEPKhiPhi
// DEMANGLED: CNCryptoMulti2::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoMulti2::Encrypt(const unsigned char* input, int input_length,
                            unsigned char* output, int output_capacity) {
    return crypto_detail::TransformMulti2(
        static_cast<crypto_detail::Multi2Context*>(context_), input,
        input_length, output, output_capacity, false);
}

// IDA_EA: 0x809B496
// IDA_NAME: _ZN14CNCryptoMulti27DecryptEPKhiPhi
// DEMANGLED: CNCryptoMulti2::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoMulti2::Decrypt(const unsigned char* input, int input_length,
                            unsigned char* output, int output_capacity) {
    return crypto_detail::TransformMulti2(
        static_cast<crypto_detail::Multi2Context*>(context_), input,
        input_length, output, output_capacity, true);
}

// IDA_EA: 0x809B592
// IDA_NAME: _ZNK14CNCryptoMulti210GetKeySizeEv
// DEMANGLED: CNCryptoMulti2::GetKeySize(void)const
int CNCryptoMulti2::GetKeySize() const { return 0x28; }

// IDA_EA: 0x809B588
// IDA_NAME: _ZNK14CNCryptoMulti212GetBlockSizeEv
// DEMANGLED: CNCryptoMulti2::GetBlockSize(void)const
int CNCryptoMulti2::GetBlockSize() const { return 8; }

// IDA_EA: 0x809B134
// IDA_NAME: _ZN14CNCryptoMulti27SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoMulti2::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoMulti2::SetMode(CRYPTO_MODE_TYPE mode) {
    if (mode != CRYPTO_MODE_ECB) {
        return CRYPTO_RESULT_UNSUPPORTED;
    }
    mode_ = mode;
    return CRYPTO_RESULT_OK;
}
