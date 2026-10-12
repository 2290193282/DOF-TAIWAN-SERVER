#ifndef OLDSRV_SECURITY_MD5_LEGACY_H_
#define OLDSRV_SECURITY_MD5_LEGACY_H_

#include <stdint.h>


// IDA_EA: 0x80B3D57
// IDA_NAME: _Z3md5PhiS_
// DEMANGLED: md5(unsigned char *,int,unsigned char *)
// The provider is BLOCKED until the active IDB exposes the complete
// md5_context layout used by this wrapper's 0xd8-byte stack object.
void* md5(uint8_t* src, int n400, uint8_t* p_s);


#endif  // OLDSRV_SECURITY_MD5_LEGACY_H_
