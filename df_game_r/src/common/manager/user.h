#ifndef OLDSRV_GLOBAL_USER_H_
#define OLDSRV_GLOBAL_USER_H_

#include "infra/network/packet/PacketEnum.h"


class PacketBuf;
class PacketGuard;
class CUser;

// IDA_EA: 0x8653270
// IDA_NAME: _ZN5CUser10UpdateDataEv
// DEMANGLED: CUser::UpdateData(void)
// Active reconstructed-tree ABI: CUser is declared at global scope.
// Declaration only; the full lifecycle body remains C-owned.
// IDA_EA: CUser::GetUID @0x80C8C96; CUser::Send @0x86485BA;
// CUser::getAntibotKey/setAntibotKey @0x822FCE4/0x84EC87A.
// IDA unit: CUser. Only narrow public ABI accessors are active; the full CUser
// layout remains C-owned and intentionally incomplete here.
// IDA: CUser::GetUID @0x80C8C96.
// IDA: CUser::get_acc_id @0x80DA36E.
// IDA: CUser::get_unique_id @0x80DA37C.
// IDA: CUser::get_state @0x80DA38C.
// IDA: CUser::getSecuType @0x84B9AA2.
// IDA: CUserCharacInfo::getCurCharacR @0x8120432.
// IDA: CUserCharacInfo::getCurCharacNo @0x80CBC4E.
// IDA: CUserCharacInfo::getCurCharacName @0x8101028.
// IDA: CUserCharacInfo::get_charac_level @0x80DA2B8.
// IDA: CUserCharacInfo::get_charac_job @0x80FDF20.
// IDA: CUserCharacInfo::getCurCharFirstGrowType @0x8110C94.
// IDA: CUser::Send @0x86485BA.
#endif  // OLDSRV_GLOBAL_USER_H_
