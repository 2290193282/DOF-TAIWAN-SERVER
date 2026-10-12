#ifndef OLDSRV_SECURITY_CHECKSUM_H_
#define OLDSRV_SECURITY_CHECKSUM_H_


// IDA checksum graph order used by IChecksum constructor callers.
enum CRYPTO_CHECKSUM_TYPE {
    CRYPTO_CHECKSUM_MD5 = 0,
    CRYPTO_CHECKSUM_SHA256 = 1,
    CRYPTO_CHECKSUM_SHA384 = 2,
    CRYPTO_CHECKSUM_SHA512 = 3,
};

// IDA checksum return values observed in CNChecksum* wrappers.
enum CHECKSUM_RESULT {
    CHECKSUM_RESULT_OK = 0x6FFFFFFF,
    CHECKSUM_RESULT_ERROR = 0x70000000,
    CHECKSUM_RESULT_OUTPUT_TOO_SMALL = 0x70000001,
    CHECKSUM_RESULT_FILE_OPEN_FAILED = 0x70000002,
    CHECKSUM_RESULT_INVALID_ARGUMENT = 0x70000003,
    CHECKSUM_RESULT_INIT_REJECTED = 0x70000004,
    CHECKSUM_RESULT_INVALID_LENGTH = 0x70000005,
    CHECKSUM_RESULT_UNSUPPORTED = 0x70000017,
};

// IDA: IChecksum stores only the checksum type at old offset +0x04.
class IChecksum {
public:
    explicit IChecksum(CRYPTO_CHECKSUM_TYPE type);
    IChecksum(const IChecksum& rhs);
    virtual ~IChecksum();
    IChecksum& operator=(const IChecksum& rhs);

    CRYPTO_CHECKSUM_TYPE checksum_type() const;

    virtual int Initialize(const unsigned char* key, int length) = 0;
    virtual int MakeChecksumFromMemory(const unsigned char* data, int length,
                                       unsigned char* output,
                                       int output_capacity) = 0;
    virtual int MakeChecksumFromFile(const char* filename,
                                     unsigned char* output,
                                     int output_capacity) = 0;
    virtual int ChecksumStart() = 0;
    virtual int ChecksumUpdate(const unsigned char* data, int length) = 0;
    virtual int ChecksumFinish(unsigned char* output, int output_capacity) = 0;
    virtual int GetChecksumSize() const = 0;

protected:
    CRYPTO_CHECKSUM_TYPE checksum_type_;
};

class CNChecksumMd5 : public IChecksum {
public:
    CNChecksumMd5();
    ~CNChecksumMd5();

    int Initialize(const unsigned char* key, int length);
    int MakeChecksumFromMemory(const unsigned char* data, int length,
                               unsigned char* output, int output_capacity);
    int MakeChecksumFromFile(const char* filename, unsigned char* output,
                             int output_capacity);
    int ChecksumStart();
    int ChecksumUpdate(const unsigned char* data, int length);
    int ChecksumFinish(unsigned char* output, int output_capacity);
    int GetChecksumSize() const;

private:
    void* context_;  // MD5_CTX allocated by the old wrapper context slot.
};

class CNChecksumSha256 : public IChecksum {
public:
    CNChecksumSha256();
    CNChecksumSha256(const CNChecksumSha256& rhs);
    ~CNChecksumSha256();
    CNChecksumSha256& operator=(const CNChecksumSha256& rhs);

    int Initialize(const unsigned char* key, int length);
    int MakeChecksumFromMemory(const unsigned char* data, int length,
                               unsigned char* output, int output_capacity);
    int MakeChecksumFromFile(const char* filename, unsigned char* output,
                             int output_capacity);
    int ChecksumStart();
    int ChecksumUpdate(const unsigned char* data, int length);
    int ChecksumFinish(unsigned char* output, int output_capacity);
    int GetChecksumSize() const;

private:
    void* context_;  // SHA256_CTX allocated by the old wrapper context slot.
};

class CNChecksumSha384 : public IChecksum {
public:
    CNChecksumSha384();
    CNChecksumSha384(const CNChecksumSha384& rhs);
    ~CNChecksumSha384();
    CNChecksumSha384& operator=(const CNChecksumSha384& rhs);

    int Initialize(const unsigned char* key, int length);
    int MakeChecksumFromMemory(const unsigned char* data, int length,
                               unsigned char* output, int output_capacity);
    int MakeChecksumFromFile(const char* filename, unsigned char* output,
                             int output_capacity);
    int ChecksumStart();
    int ChecksumUpdate(const unsigned char* data, int length);
    int ChecksumFinish(unsigned char* output, int output_capacity);
    int GetChecksumSize() const;

private:
    void* context_;  // SHA512_CTX used by OpenSSL SHA384 routines.
};

class CNChecksumSha512 : public IChecksum {
public:
    CNChecksumSha512();
    CNChecksumSha512(const CNChecksumSha512& rhs);
    ~CNChecksumSha512();
    CNChecksumSha512& operator=(const CNChecksumSha512& rhs);

    int Initialize(const unsigned char* key, int length);
    int MakeChecksumFromMemory(const unsigned char* data, int length,
                               unsigned char* output, int output_capacity);
    int MakeChecksumFromFile(const char* filename, unsigned char* output,
                             int output_capacity);
    int ChecksumStart();
    int ChecksumUpdate(const unsigned char* data, int length);
    int ChecksumFinish(unsigned char* output, int output_capacity);
    int GetChecksumSize() const;

private:
    void* context_;  // SHA512_CTX allocated by the old wrapper context slot.
};

// IDA_EA: 0x809E309
// IDA_NAME: _Z20CreateCryptoChecksum20CRYPTO_CHECKSUM_TYPE
// DEMANGLED: CreateCryptoChecksum(CRYPTO_CHECKSUM_TYPE)
IChecksum* CreateCryptoChecksum(CRYPTO_CHECKSUM_TYPE type);

// IDA_EA: 0x809E423
// IDA_NAME: _Z21DestroyCryptoChecksumRP9IChecksum
// DEMANGLED: DestroyCryptoChecksum(IChecksum *&)
IChecksum** DestroyCryptoChecksum(IChecksum*& checksum);

// IDA_EA: 0x808CC6A
// IDA_NAME: _Z18getSha256UsingSaltPKhiPc
// DEMANGLED: getSha256UsingSalt(unsigned char const*,int,char *)
int getSha256UsingSalt(const unsigned char* src, int length, char* output);


#endif  // OLDSRV_SECURITY_CHECKSUM_H_
