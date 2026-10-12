#include "old_data/db/core/db_conn_info.h"

#include <cstring>


typedef char STDBConnInfo_Size_Must_Be_0x168[(sizeof(STDBConnInfo) == 0x168) ? 1 : -1];

// IDA_EA: 0x80ECD38
// IDA_NAME: _ZN12STDBConnInfoC2Ev
// DEMANGLED: STDBConnInfo::STDBConnInfo(void)
STDBConnInfo::STDBConnInfo() : db_port(0), server_group(0) {
    std::memset(db_ip, 0, sizeof(db_ip));
    std::memset(pad_17, 0, sizeof(pad_17));
    std::memset(db_userid, 0, sizeof(db_userid));
    std::memset(db_passwd, 0, sizeof(db_passwd));
    std::memset(db_name, 0, sizeof(db_name));
    std::memset(reserved, 0, sizeof(reserved));
}
