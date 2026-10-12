#include "infra/network/transport/ip_address_split_abi.h"

#include <cstring>


// IDA_EA: 0x86B1DD5
// IDA_NAME: _Z22Divide_C_ClassIP_EndIPPcS_S_
// DEMANGLED: Divide_C_ClassIP_EndIP(char *,char *,char *)
void Divide_C_ClassIP_EndIP(char* c_class_destination,
                            char* end_destination,
                            char* source) {
    if (!source) {
        return;
    }

    char* const final_dot = std::strrchr(source, '.');
    if (!final_dot || static_cast<size_t>(final_dot - source) > 0x0cu) {
        return;
    }

    const size_t end_length = std::strlen(final_dot + 1);
    if (end_length > 4u) {
        return;
    }
    if (c_class_destination) {
        std::strncpy(c_class_destination,
                     source,
                     static_cast<size_t>(final_dot - source));
    }
    if (end_destination) {
        std::strncpy(end_destination, final_dot + 1, end_length);
    }
}
