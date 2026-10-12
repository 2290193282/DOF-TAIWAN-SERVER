#include "infra/security/crypto/crypto_shift.h"

#include "infra/security/crypto/crypto_graph_internal.h"

namespace {

// IDA_EA: 0x809C418
// IDA_NAME: _ZL3ROLji
unsigned int ROL(unsigned int value, int bits) {
    const unsigned int count = static_cast<unsigned int>(bits) & 0x1Fu;
    return count == 0u
               ? value
               : (value << count) | (value >> (32u - count));
}

// IDA_EA: 0x809C42D
// IDA_NAME: _ZL3RORji
unsigned int ROR(unsigned int value, int bits) {
    const unsigned int count = static_cast<unsigned int>(bits) & 0x1Fu;
    return count == 0u
               ? value
               : (value >> count) | (value << (32u - count));
}

}  // namespace


// IDA_EA: 0x809C442
// IDA_NAME: _ZN13CNCryptoShiftC2Ev
// DEMANGLED: CNCryptoShift::CNCryptoShift(void)
CNCryptoShift::CNCryptoShift()
    : ICryptoGraph(CRYPTO_GRAPH_SHIFT),
      shift_bits_(7),
      xor_mask_(490426012U) {}

// IDA_EA: 0x809C47A
// IDA_NAME: _ZN13CNCryptoShiftD2Ev
// DEMANGLED: CNCryptoShift::~CNCryptoShift()
// IDA_EA: 0x809C4AA
// IDA_NAME: _ZN13CNCryptoShiftD0Ev
// DEMANGLED: CNCryptoShift::~CNCryptoShift() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
CNCryptoShift::~CNCryptoShift() {}

// IDA_EA: 0x809C4C8
// IDA_NAME: _ZN13CNCryptoShiftC2ERKS_
// DEMANGLED: CNCryptoShift::CNCryptoShift(CNCryptoShift const&)
CNCryptoShift::CNCryptoShift(const CNCryptoShift& rhs)
    : ICryptoGraph(rhs), shift_bits_(7), xor_mask_(490426012U) {}

// IDA_EA: 0x809C4EC
// IDA_NAME: _ZN13CNCryptoShiftaSERKS_
// DEMANGLED: CNCryptoShift::operator=(CNCryptoShift const&)
CNCryptoShift& CNCryptoShift::operator=(const CNCryptoShift& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x809C4FE
// IDA_NAME: _ZN13CNCryptoShift10InitializeEPKhi
// DEMANGLED: CNCryptoShift::Initialize(unsigned char const*,int)
int CNCryptoShift::Initialize(const unsigned char* key, int length) {
    if (key == 0 || length <= 7) {
        return CRYPTO_RESULT_ERROR;
    }
    key_.assign(key, key + GetKeySize());
    shift_bits_ = crypto_detail::ReadLE32(key) & 0x1F;
    xor_mask_ = crypto_detail::ReadLE32(key + 4);
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809C4F4
// IDA_NAME: _ZN13CNCryptoShift7SetModeE16CRYPTO_MODE_TYPE
// DEMANGLED: CNCryptoShift::SetMode(CRYPTO_MODE_TYPE)
int CNCryptoShift::SetMode(CRYPTO_MODE_TYPE mode) {
    (void)mode;
    return CRYPTO_RESULT_UNSUPPORTED;
}

// IDA_EA: 0x809C534
// IDA_NAME: _ZN13CNCryptoShift7EncryptEPhi
// DEMANGLED: CNCryptoShift::Encrypt(unsigned char *,int)
int CNCryptoShift::Encrypt(unsigned char* data, int length) {
    if (shift_bits_ > 0x1F) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if ((length % GetBlockSize()) != 0) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    for (int offset = 0; offset < length; offset += GetBlockSize()) {
        unsigned int value = crypto_detail::ReadLE32(data + offset);
        ROR(value, shift_bits_);
        crypto_detail::WriteLE32(data + offset, value ^ xor_mask_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809C5FE
// IDA_NAME: _ZN13CNCryptoShift7DecryptEPhi
// DEMANGLED: CNCryptoShift::Decrypt(unsigned char *,int)
int CNCryptoShift::Decrypt(unsigned char* data, int length) {
    if (shift_bits_ > 0x1F) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if ((length % GetBlockSize()) != 0) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    for (int offset = 0; offset < length; offset += GetBlockSize()) {
        unsigned int value = xor_mask_ ^ crypto_detail::ReadLE32(data + offset);
        ROL(value, shift_bits_);
        crypto_detail::WriteLE32(data + offset, value);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809C6C8
// IDA_NAME: _ZN13CNCryptoShift7EncryptEPKhiPhi
// DEMANGLED: CNCryptoShift::Encrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoShift::Encrypt(const unsigned char* input, int input_length,
                           unsigned char* output, int output_capacity) {
    if (shift_bits_ > 0x1F) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if ((input_length % GetBlockSize()) != 0) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    for (int offset = 0; offset < input_length; offset += GetBlockSize()) {
        unsigned int value = crypto_detail::ReadLE32(input + offset);
        ROR(value, shift_bits_);
        crypto_detail::WriteLE32(output + offset, value ^ xor_mask_);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809C7B0
// IDA_NAME: _ZN13CNCryptoShift7DecryptEPKhiPhi
// DEMANGLED: CNCryptoShift::Decrypt(unsigned char const*,int,unsigned char *,int)
int CNCryptoShift::Decrypt(const unsigned char* input, int input_length,
                           unsigned char* output, int output_capacity) {
    if (shift_bits_ > 0x1F) {
        return CRYPTO_RESULT_NO_CONTEXT;
    }
    if ((input_length % GetBlockSize()) != 0) {
        return CRYPTO_RESULT_INVALID_BLOCK_SIZE;
    }
    if (input_length > output_capacity) {
        return CRYPTO_RESULT_BUFFER_TOO_SMALL;
    }
    for (int offset = 0; offset < input_length; offset += GetBlockSize()) {
        unsigned int value =
            xor_mask_ ^ crypto_detail::ReadLE32(input + offset);
        ROL(value, shift_bits_);
        crypto_detail::WriteLE32(output + offset, value);
    }
    return CRYPTO_RESULT_OK;
}

// IDA_EA: 0x809C898
// IDA_NAME: _ZNK13CNCryptoShift10GetKeySizeEv
// DEMANGLED: CNCryptoShift::GetKeySize(void)const
int CNCryptoShift::GetKeySize() const { return 0x08; }

// IDA_EA: 0x809C8A2
// IDA_NAME: _ZNK13CNCryptoShift12GetBlockSizeEv
// DEMANGLED: CNCryptoShift::GetBlockSize(void)const
int CNCryptoShift::GetBlockSize() const { return 4; }
