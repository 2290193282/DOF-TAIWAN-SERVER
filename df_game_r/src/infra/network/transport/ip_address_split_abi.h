#ifndef OLDSRV_GLOBAL_IP_ADDRESS_SPLIT_ABI_H_
#define OLDSRV_GLOBAL_IP_ADDRESS_SPLIT_ABI_H_

// IDA_EA: 0x86B1DD5
// IDA_NAME: _Z22Divide_C_ClassIP_EndIPPcS_S_
// DEMANGLED: Divide_C_ClassIP_EndIP(char *,char *,char *)
// Splits an IPv4 text address at its final dot.  The original function does
// not append a terminator after either strncpy; callers own pre-zeroed output
// storage when they require a C string.
void Divide_C_ClassIP_EndIP(char* c_class_destination,
                            char* end_destination,
                            char* source);


#endif  // OLDSRV_GLOBAL_IP_ADDRESS_SPLIT_ABI_H_
