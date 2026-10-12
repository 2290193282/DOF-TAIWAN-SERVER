#include "infra/security/crypto/cipher.h"

#include <cstring>
#include <vector>

#include "common/manager/global_instance.h"
#include "infra/security/crypto/crypto_anubis.h"
#include "infra/security/crypto/crypto_blowfish.h"
#include "infra/security/crypto/crypto_cast5.h"
#include "infra/security/crypto/crypto_kasumi.h"
#include "infra/security/crypto/crypto_khazad.h"
#include "infra/security/crypto/crypto_multi2.h"
#include "infra/security/crypto/crypto_noekeon.h"
#include "infra/security/crypto/crypto_rc6.h"
#include "infra/security/crypto/crypto_rijndael.h"
#include "infra/security/crypto/crypto_shift.h"
#include "infra/security/crypto/crypto_skipjack.h"
#include "infra/security/crypto/crypto_tea.h"
#include "infra/security/crypto/crypto_twofish.h"
#include "infra/security/crypto/crypto_xtea.h"


namespace {

const int kCryptoGraphCount = 14;

}  // namespace

template <>
Cipher* GlobalInstance<Cipher>::m_p = 0;

template <>
Cipher* GlobalInstance<Cipher>::inst_ptr();

template <>
Cipher* GlobalInstance<Cipher>::create();

// IDA_EA: 0x8097160
// IDA_NAME: _Z13GetCryptoTypei
// DEMANGLED: GetCryptoType(int)
int GetCryptoType(int sequence) {
    return sequence % kCryptoGraphCount;
}

// IDA_EA: 0x809DF90
// IDA_NAME: _Z17CreateCryptoGraph17CRYPTO_GRAPH_TYPE
// DEMANGLED: CreateCryptoGraph(CRYPTO_GRAPH_TYPE)
ICryptoGraph* CreateCryptoGraph(CRYPTO_GRAPH_TYPE type) {
    switch (type) {
        case CRYPTO_GRAPH_SHIFT:
            return new CNCryptoShift();
        case CRYPTO_GRAPH_RIJNDAEL:
            return new CNCryptoRijndael();
        case CRYPTO_GRAPH_BLOWFISH:
            return new CNCryptoBlowFish();
        case CRYPTO_GRAPH_RC6:
            return new CNCryptoRc6();
        case CRYPTO_GRAPH_TWOFISH:
            return new CNCryptoTwoFish();
        case CRYPTO_GRAPH_TEA:
            return new CNCryptoTea();
        case CRYPTO_GRAPH_KASUMI:
            return new CNCryptoKasumi();
        case CRYPTO_GRAPH_XTEA:
            return new CNCryptoXTea();
        case CRYPTO_GRAPH_NOEKEON:
            return new CNCryptoNoekeon();
        case CRYPTO_GRAPH_KHAZAD:
            return new CNCryptoKhazad();
        case CRYPTO_GRAPH_CAST5:
            return new CNCryptoCast5();
        case CRYPTO_GRAPH_SKIPJACK:
            return new CNCryptoSkipjack();
        case CRYPTO_GRAPH_MULTI2:
            return new CNCryptoMulti2();
        case CRYPTO_GRAPH_ANUBIS:
            return new CNCryptoAnubis();
    }
    return 0;
}

// IDA_EA: 0x809E2D0
// IDA_NAME: _Z18DestroyCryptoGraphRP12ICryptoGraph
// DEMANGLED: DestroyCryptoGraph(ICryptoGraph *&)
ICryptoGraph** DestroyCryptoGraph(ICryptoGraph** graph) {
    if (graph != 0 && *graph != 0) {
        delete *graph;
        *graph = 0;
    }
    return graph;
}

// IDA_EA: 0x8097196
// IDA_NAME: _ZN6CipherC2Ev
// DEMANGLED: Cipher::Cipher(void)
Cipher::Cipher() : total_key_length_(0), graphs_() {
    graphs_.reserve(kCryptoGraphCount);
    for (int index = 0; index < kCryptoGraphCount; ++index) {
        ICryptoGraph* graph = CreateCryptoGraph(static_cast<CRYPTO_GRAPH_TYPE>(index));
        graphs_.push_back(graph);
        if (graph != 0) {
            total_key_length_ += graph->GetKeySize();
        }
    }
}

// IDA_EA: 0x80972E8
// IDA_NAME: _ZN6CipherC2ERKS_
// DEMANGLED: Cipher::Cipher(Cipher const&)
Cipher::Cipher(const Cipher& rhs)
    : total_key_length_(0), graphs_() {
    (void)rhs;
}

// IDA_EA: 0x809724C
// IDA_NAME: _ZN6CipherD2Ev
// DEMANGLED: Cipher::~Cipher()
Cipher::~Cipher() {
    for (int index = 0;
         index < kCryptoGraphCount && index < static_cast<int>(graphs_.size());
         ++index) {
        ICryptoGraph* graph = graphs_[index];
        DestroyCryptoGraph(&graph);
        graphs_[index] = 0;
    }
    graphs_.clear();
}

// IDA_EA: 0x80972FE
// IDA_NAME: _ZN6CipheraSERKS_
// DEMANGLED: Cipher::operator=(Cipher const&)
Cipher& Cipher::operator=(const Cipher& rhs) {
    (void)rhs;
    return *this;
}

// IDA_EA: 0x8097306
// IDA_NAME: _ZN6Cipher10InitializeEPKhi
// DEMANGLED: Cipher::Initialize(unsigned char const*,int)
bool Cipher::Initialize(const unsigned char* key, int length) {
    if (key == 0 || length < 0 || length > kOriginalKeyCapacity) {
        return false;
    }
    // IDA 0x8097323: memcpy(this + 0x10, src, n) precedes per-graph setup.
    std::memcpy(original_key_, key, length);

    const unsigned char* cursor = key;
    int remaining = length;
    for (std::vector<ICryptoGraph*>::iterator it = graphs_.begin();
         it != graphs_.end(); ++it) {
        ICryptoGraph* graph = *it;
        if (graph == 0) {
            return false;
        }
        const int key_size = graph->GetKeySize();
        if (remaining < key_size) {
            return false;
        }
        if (graph->Initialize(cursor, key_size) != CRYPTO_RESULT_OK) {
            return false;
        }
        cursor += key_size;
        remaining -= key_size;
    }
    return true;
}

// IDA_EA: 0x80973C0
// IDA_NAME: _ZNK6Cipher14AdjustDataSizeEiPi
// DEMANGLED: Cipher::AdjustDataSize(int,int *) const
void Cipher::AdjustDataSize(int sequence, int* size) const {
    if (size == 0) {
        return;
    }
    const int crypto_type = GetCryptoType(sequence);
    if (crypto_type < 0 || crypto_type >= static_cast<int>(graphs_.size())) {
        return;
    }
    ICryptoGraph* graph = graphs_[crypto_type];
    if (graph == 0) {
        return;
    }
    const int block_size = graph->GetBlockSize();
    if (block_size <= 0) {
        return;
    }

    const int original_size = *size;
    const int remainder = original_size % block_size;
    if (remainder != 0) {
        *size = original_size + block_size - remainder;
    }
}

// IDA_EA: 0x8097436
// IDA_NAME: _ZN6Cipher7EncryptEiPKhiPhPi
// DEMANGLED: Cipher::Encrypt(int,unsigned char const*,int,unsigned char *,int *)
bool Cipher::Encrypt(int sequence, const unsigned char* input, int input_length,
                     unsigned char* output, int* output_capacity) {
    if (output_capacity == 0) {
        return false;
    }
    if (input_length <= 0) {
        *output_capacity = 0;
        return true;
    }

    const int crypto_type = GetCryptoType(sequence);
    if (crypto_type < 0 || crypto_type >= static_cast<int>(graphs_.size())) {
        return false;
    }
    ICryptoGraph* graph = graphs_[crypto_type];
    if (graph == 0) {
        return false;
    }

    int adjusted_length = input_length;
    const int block_size = graph->GetBlockSize();
    if (block_size <= 0) {
        return false;
    }
    if (adjusted_length % block_size) {
        adjusted_length += block_size - (adjusted_length % block_size);
    }

    std::vector<unsigned char> padded;
    const unsigned char* source = input;
    if (adjusted_length != input_length) {
        padded.assign(adjusted_length, 0);
        if (input != 0) {
            std::memcpy(&padded[0], input, input_length);
        }
        source = &padded[0];
    }

    if (graph->Encrypt(source, adjusted_length, output, *output_capacity) !=
        CRYPTO_RESULT_OK) {
        return false;
    }
    *output_capacity = adjusted_length;
    return true;
}

// IDA_EA: 0x809751E
// IDA_NAME: _ZN6Cipher7DecryptEiPKhiPhPi
// DEMANGLED: Cipher::Decrypt(int,unsigned char const*,int,unsigned char *,int *)
bool Cipher::Decrypt(int sequence, const unsigned char* input, int input_length,
                     unsigned char* output, int* output_capacity) {
    if (output_capacity == 0) {
        return false;
    }
    if (input_length <= 0) {
        *output_capacity = 0;
        return true;
    }

    const int crypto_type = GetCryptoType(sequence);
    if (crypto_type < 0 || crypto_type >= static_cast<int>(graphs_.size())) {
        return false;
    }
    ICryptoGraph* graph = graphs_[crypto_type];
    if (graph == 0) {
        return false;
    }
    if (graph->GetBlockSize() <= 0) {
        return false;
    }
    if (graph->Decrypt(input, input_length, output, *output_capacity) !=
        CRYPTO_RESULT_OK) {
        return false;
    }
    *output_capacity = input_length;
    return true;
}

// IDA_EA: 0x80975D4
// IDA_NAME: _ZNK6Cipher17GetTotalKeyLengthEv
// DEMANGLED: Cipher::GetTotalKeyLength(void) const
int Cipher::GetTotalKeyLength() const {
    return total_key_length_;
}

// IDA_EA: 0x80975DE
// IDA_NAME: _ZN6Cipher14GetOriginalKeyEPh
// DEMANGLED: Cipher::GetOriginalKey(unsigned char *)
void Cipher::GetOriginalKey(unsigned char* dest) const {
    if (dest == 0) {
        return;
    }
    // IDA 0x8097602: memcpy(dest, this + 0x10, total_key_length).
    std::memcpy(dest, original_key_, total_key_length_);
}

// IDA_EA: 0x82A713A
// IDA_NAME: _Z8G_Cipherv
// DEMANGLED: G_Cipher(void)
Cipher* G_Cipher() {
    return GlobalInstance<Cipher>::inst_ptr();
}

// IDA_EA: 0x82AB9DA
// IDA_NAME: _ZN14GlobalInstanceI6CipherE8inst_ptrEv
// DEMANGLED: GlobalInstance<Cipher>::inst_ptr(void)
template <>
Cipher* GlobalInstance<Cipher>::inst_ptr() {
    GlobalInstance<Cipher>::create();
    return GlobalInstance<Cipher>::m_p;
}

// IDA_EA: 0x82B6577
// IDA_NAME: _ZN14GlobalInstanceI6CipherE6createEv
// DEMANGLED: GlobalInstance<Cipher>::create(void)
template <>
Cipher* GlobalInstance<Cipher>::create() {
    if (GlobalInstance<Cipher>::m_p == 0) {
        GlobalInstance<Cipher>::m_p = new Cipher();
    }
    return GlobalInstance<Cipher>::m_p;
}
