#ifndef OLDSRV_SECURITY_CRYPTO_GRAPH_H_
#define OLDSRV_SECURITY_CRYPTO_GRAPH_H_

#include <vector>

#include "infra/security/crypto/crypto_types.h"


// IDA: ICryptoGraph vtable users call Initialize at +0x0C, copy Encrypt at
// +0x18, copy Decrypt at +0x1C, GetKeySize at +0x20, and GetBlockSize at
// +0x24. This file keeps only the base class; concrete algorithms live in
// their own class files to match the restored df_game_r class layout.
class ICryptoGraph {
public:
    explicit ICryptoGraph(CRYPTO_GRAPH_TYPE graph_type);
    ICryptoGraph(const ICryptoGraph& rhs);
    virtual ~ICryptoGraph();
    ICryptoGraph& operator=(const ICryptoGraph& rhs);

    virtual int Initialize(const unsigned char* key, int length) = 0;
    virtual int SetMode(CRYPTO_MODE_TYPE mode) = 0;
    virtual int Encrypt(unsigned char* data, int length) = 0;
    virtual int Decrypt(unsigned char* data, int length) = 0;
    virtual int Encrypt(const unsigned char* input, int input_length,
                        unsigned char* output, int output_capacity) = 0;
    virtual int Decrypt(const unsigned char* input, int input_length,
                        unsigned char* output, int output_capacity) = 0;
    virtual int GetKeySize() const = 0;
    virtual int GetBlockSize() const = 0;

    // IDA: ICryptoGraph::GetApiType @0x809DF82.
    CRYPTO_GRAPH_TYPE GetApiType() const;

protected:
    std::vector<unsigned char> key_;  // Per-graph key slice from Cipher::Initialize.
    CRYPTO_GRAPH_TYPE graph_type_;  // Original constructor graph type argument.
    CRYPTO_MODE_TYPE mode_;  // Current mode accepted by SetMode wrappers.
    void* context_;  // Algorithm-specific state stored behind the old +0x08 slot.
};


#endif  // OLDSRV_SECURITY_CRYPTO_GRAPH_H_
