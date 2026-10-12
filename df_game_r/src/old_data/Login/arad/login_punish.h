#ifndef OLDSRV_OLD_DATA_LOGIN_ARAD_LOGIN_PUNISH_H_
#define OLDSRV_OLD_DATA_LOGIN_ARAD_LOGIN_PUNISH_H_

#include "old_data/db/account/login_payloads.h"

namespace ARAD {
namespace PUNISH_CHECK {

// IDA_EA: 0x8196D1A
// IDA_NAME: _ZN4ARAD12PUNISH_CHECK19punish_check_by_WEBEP14SIG_LOGIN_DATAli
// DEMANGLED: ARAD::PUNISH_CHECK::punish_check_by_WEB(SIG_LOGIN_DATA *,long,int)
int punish_check_by_WEB(SIG_LOGIN_DATA* login, long end_time, int punish_type);

// IDA_EA: 0x8196D51
// IDA_NAME: _ZN4ARAD12PUNISH_CHECK19punish_check_by_ARSEP14SIG_LOGIN_DATAli
// DEMANGLED: ARAD::PUNISH_CHECK::punish_check_by_ARS(SIG_LOGIN_DATA *,long,int)
int punish_check_by_ARS(SIG_LOGIN_DATA* login, long end_time, int punish_type);

// IDA_EA: 0x8196D88
// IDA_NAME: _ZN4ARAD12PUNISH_CHECK19punish_check_by_SYSEP14SIG_LOGIN_DATAli
// DEMANGLED: ARAD::PUNISH_CHECK::punish_check_by_SYS(SIG_LOGIN_DATA *,long,int)
int punish_check_by_SYS(SIG_LOGIN_DATA* login, long end_time, int punish_type);

// IDA_EA: 0x8196DBF
// IDA_NAME: _ZN4ARAD12PUNISH_CHECK19punish_check_by_HUBEP14SIG_LOGIN_DATAi
// DEMANGLED: ARAD::PUNISH_CHECK::punish_check_by_HUB(SIG_LOGIN_DATA *,int)
char* punish_check_by_HUB(SIG_LOGIN_DATA* login, int unused);

// IDA_EA: 0x8196DCE
// IDA_NAME: _ZN4ARAD12PUNISH_CHECK19punish_check_by_RESEP14SIG_LOGIN_DATAi
// DEMANGLED: ARAD::PUNISH_CHECK::punish_check_by_RES(SIG_LOGIN_DATA *,int)
void punish_check_by_RES(SIG_LOGIN_DATA* login, int punish_type);

}  // namespace PUNISH_CHECK
}  // namespace ARAD

#endif  // OLDSRV_OLD_DATA_LOGIN_ARAD_LOGIN_PUNISH_H_
