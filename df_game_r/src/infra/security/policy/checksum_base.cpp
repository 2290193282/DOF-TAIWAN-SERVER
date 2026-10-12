#include "infra/security/policy/checksum.h"


// IDA_EA: 0x809DE70
// IDA_NAME: _ZN9IChecksumC2E20CRYPTO_CHECKSUM_TYPE
// DEMANGLED: IChecksum::IChecksum(CRYPTO_CHECKSUM_TYPE)
IChecksum::IChecksum(CRYPTO_CHECKSUM_TYPE type) : checksum_type_(type) {}

// IDA_EA: 0x809DE88
// IDA_NAME: _ZN9IChecksumD2Ev
// DEMANGLED: IChecksum::~IChecksum()
// IDA_EA: 0x809DEAE
// IDA_NAME: _ZN9IChecksumD0Ev
// DEMANGLED: IChecksum::~IChecksum() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
IChecksum::~IChecksum() {}

// IDA_EA: 0x809DECC
// IDA_NAME: _ZN9IChecksumC2ERKS_
// DEMANGLED: IChecksum::IChecksum(IChecksum const&)
IChecksum::IChecksum(const IChecksum& rhs) : checksum_type_(rhs.checksum_type_) {}

// IDA_EA: 0x809DEE6
// IDA_NAME: _ZN9IChecksumaSERKS_
// DEMANGLED: IChecksum::operator=(IChecksum const&)
IChecksum& IChecksum::operator=(const IChecksum& rhs) {
    (void)rhs;
    return *this;
}

CRYPTO_CHECKSUM_TYPE IChecksum::checksum_type() const {
    return checksum_type_;
}
