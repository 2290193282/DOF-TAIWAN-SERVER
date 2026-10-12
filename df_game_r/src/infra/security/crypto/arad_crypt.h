#ifndef OLDSRV_SECURITY_ARAD_CRYPT_H_
#define OLDSRV_SECURITY_ARAD_CRYPT_H_

class CRijndael;

namespace ARAD {
namespace ARAD_CRYPT {

// IDA_EA: 0x8184ABC
// IDA_NAME: _ZN4ARAD10ARAD_CRYPT10encryptAESEP9CRijndaelPcS3_
// DEMANGLED: ARAD::ARAD_CRYPT::encryptAES(CRijndael *,char *,char *)
char* encryptAES(CRijndael* crypt, char* source, char* destination);

}  // namespace ARAD_CRYPT
}  // namespace ARAD

#endif  // OLDSRV_SECURITY_ARAD_CRYPT_H_
