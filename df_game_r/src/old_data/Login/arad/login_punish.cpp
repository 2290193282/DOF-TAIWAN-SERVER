#include "old_data/Login/arad/login_punish.h"

namespace ARAD {
namespace PUNISH_CHECK {

// IDA_EA: 0x8196D1A
// IDA_NAME: _ZN4ARAD12PUNISH_CHECK19punish_check_by_WEBEP14SIG_LOGIN_DATAli
// DEMANGLED: ARAD::PUNISH_CHECK::punish_check_by_WEB(SIG_LOGIN_DATA *,long,int)
int punish_check_by_WEB(SIG_LOGIN_DATA* login, long end_time, int punish_type) {
    if (punish_type != 1) {
        return 1;
    }
    login->fail_code = 4;
    if (end_time == 0) {
        login->fail_code = 1001;
    }
    return 0;
}

// IDA_EA: 0x8196D51
// IDA_NAME: _ZN4ARAD12PUNISH_CHECK19punish_check_by_ARSEP14SIG_LOGIN_DATAli
// DEMANGLED: ARAD::PUNISH_CHECK::punish_check_by_ARS(SIG_LOGIN_DATA *,long,int)
int punish_check_by_ARS(SIG_LOGIN_DATA* login, long end_time, int punish_type) {
    if (punish_type != 1) {
        return 1;
    }
    login->fail_code = 4;
    if (end_time == 0) {
        login->fail_code = 1003;
    }
    return 0;
}

// IDA_EA: 0x8196D88
// IDA_NAME: _ZN4ARAD12PUNISH_CHECK19punish_check_by_SYSEP14SIG_LOGIN_DATAli
// DEMANGLED: ARAD::PUNISH_CHECK::punish_check_by_SYS(SIG_LOGIN_DATA *,long,int)
int punish_check_by_SYS(SIG_LOGIN_DATA* login, long end_time, int punish_type) {
    if (punish_type != 1) {
        return 1;
    }
    login->fail_code = 4;
    if (end_time == 0) {
        login->fail_code = 1002;
    }
    return 0;
}

// IDA_EA: 0x8196DBF
// IDA_NAME: _ZN4ARAD12PUNISH_CHECK19punish_check_by_HUBEP14SIG_LOGIN_DATAi
// DEMANGLED: ARAD::PUNISH_CHECK::punish_check_by_HUB(SIG_LOGIN_DATA *,int)
char* punish_check_by_HUB(SIG_LOGIN_DATA* login, int /* unused */) {
    login->punish_user_marker = 1;
    return reinterpret_cast<char*>(login);
}

// IDA_EA: 0x8196DCE
// IDA_NAME: _ZN4ARAD12PUNISH_CHECK19punish_check_by_RESEP14SIG_LOGIN_DATAi
// DEMANGLED: ARAD::PUNISH_CHECK::punish_check_by_RES(SIG_LOGIN_DATA *,int)
void punish_check_by_RES(SIG_LOGIN_DATA* /* login */, int /* punish_type */) {
    // The old-server body at 0x8196DCE is intentionally empty.
}

}  // namespace PUNISH_CHECK
}  // namespace ARAD
