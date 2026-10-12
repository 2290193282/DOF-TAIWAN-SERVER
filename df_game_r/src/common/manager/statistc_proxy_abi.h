#ifndef OLDSRV_GLOBAL_STATISTC_PROXY_ABI_H_
#define OLDSRV_GLOBAL_STATISTC_PROXY_ABI_H_

class CUser;

// IDA unit: 99_misc_global/statistc_proxy.  The original namespace spelling
// omits the second 'i'; preserving it is necessary for the exported symbol.
namespace statistc_proxy {

// IDA_EA: 0x860FDD2
// IDA_NAME: _ZN14statistc_proxy3addEP5CUserPKcS3_jS3_z
// DEMANGLED: statistc_proxy::add(CUser *,char const*,char const*,unsigned int,char const*,...)
int add(CUser* user,
        const char* category,
        const char* statistic,
        unsigned int value,
        const char* format,
        ...);

}  // namespace statistc_proxy

#endif  // OLDSRV_GLOBAL_STATISTC_PROXY_ABI_H_
