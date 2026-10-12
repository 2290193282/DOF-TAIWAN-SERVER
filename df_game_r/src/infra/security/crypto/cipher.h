#ifndef OLDSRV_SECURITY_CIPHER_H_
#define OLDSRV_SECURITY_CIPHER_H_

#include <vector>

#include "infra/security/crypto/crypto_graph.h"


// IDA: CreateCryptoGraph @0x809DF90.
ICryptoGraph* CreateCryptoGraph(CRYPTO_GRAPH_TYPE type);

// IDA: DestroyCryptoGraph @0x809E2D0.
ICryptoGraph** DestroyCryptoGraph(ICryptoGraph** graph);

// IDA: Cipher @0x8097196. The original owns 14 ICryptoGraph instances and
// stores total key length at offset +0x00.
class Cipher {
public:
    // IDA: every Cipher allocation is 0x410 bytes on i386 (see
    // GlobalInstance<Cipher>::inst_ptr @ 0x82A713A).  The original key is a
    // fixed 0x400-byte payload at +0x10, not a second std::vector.
    enum { kOriginalKeyCapacity = 0x400 };

    Cipher();
    Cipher(const Cipher& rhs);
    ~Cipher();
    Cipher& operator=(const Cipher& rhs);

    // IDA: Cipher::Initialize @0x8097306.
    bool Initialize(const unsigned char* key, int length);

    // IDA: Cipher::AdjustDataSize @0x80973C0. PacketBuf::encrypt_packet calls
    // this before allocating/copying the encrypted payload buffer.
    void AdjustDataSize(int sequence, int* size) const;

    // IDA: Cipher::Encrypt @0x8097436 / Cipher::Decrypt @0x809751E.
    bool Encrypt(int sequence, const unsigned char* input, int input_length,
                 unsigned char* output, int* output_capacity);
    bool Decrypt(int sequence, const unsigned char* input, int input_length,
                 unsigned char* output, int* output_capacity);

    // IDA: Cipher::GetTotalKeyLength @0x80975D4.
    int GetTotalKeyLength() const;

    // IDA: Cipher::GetOriginalKey @0x80975DE. CUser::InitOnAccept copies this
    // byte string into the first interface packet sent after socket accept.
    void GetOriginalKey(unsigned char* dest) const;

private:
    int total_key_length_;  // IDA offset +0x00: sum of all graph key lengths.
    std::vector<ICryptoGraph*> graphs_;  // IDA offset +0x04: vector<ICryptoGraph*>.
    unsigned char original_key_[kOriginalKeyCapacity];  // IDA offset +0x10.
};

#if defined(__i386__)
static_assert(sizeof(Cipher) == 0x410,
              "Cipher must retain the original i386 0x410-byte ABI layout");
#endif

// IDA: G_Cipher @0x82A713A.
Cipher* G_Cipher();


#endif  // OLDSRV_SECURITY_CIPHER_H_
