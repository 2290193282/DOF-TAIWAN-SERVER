#ifndef OLDSRV_GLOBAL_PVP_ASSAULT_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_PVP_ASSAULT_PUBLIC_ABI_H_

class CUser;

namespace pvp_assault {

class CAssaultMgr;

// IDA_EA: 0x82350A0
// IDA_NAME: _ZN11pvp_assault21GetInstanceAssaultMgrEv
// DEMANGLED: pvp_assault::GetInstanceAssaultMgr(void)
CAssaultMgr* GetInstanceAssaultMgr();

}  // namespace pvp_assault

#endif  // OLDSRV_GLOBAL_PVP_ASSAULT_PUBLIC_ABI_H_
