#include "old_game/appearance/avatar/arad_avatar_runtime.h"

#include "infra/log/format/trace.h"

// These are the three script-service lifecycle implementations called by
// ARAD::ScriptManager::Init.  The parsers themselves live in B's canonical
// arad_avatar_script_data provider; this C-side unit only restores the old
// singleton ownership and virtual ScriptInterface entry points.
namespace ARAD {

template <>
// IDA_EA: 0x81934A6
// IDA_NAME: _ZN4ARAD9SingletonI20AvatarRechargeServerEC2Ev
// DEMANGLED: ARAD::Singleton<AvatarRechargeServer>::Singleton(void)
Singleton<AvatarRechargeServer>::Singleton() {}

template <>
// IDA_EA: 0x81935A2
// IDA_NAME: _ZN4ARAD9SingletonI20AvatarRechargeServerE3GetEv
// DEMANGLED: ARAD::Singleton<AvatarRechargeServer>::Get(void)
AvatarRechargeServer* Singleton<AvatarRechargeServer>::Get() {
    static AvatarRechargeServer object;
    return &object;
}

template <>
Singleton<EmblemCompoundServer>::Singleton() {}

template <>
EmblemCompoundServer* Singleton<EmblemCompoundServer>::Get() {
    static EmblemCompoundServer object;
    return &object;
}

template <>
// IDA_EA: 0x819380A
// IDA_NAME: _ZN4ARAD9SingletonI19AvatarConvertServerEC2Ev
// DEMANGLED: ARAD::Singleton<AvatarConvertServer>::Singleton(void)
Singleton<AvatarConvertServer>::Singleton() {}

template <>
// IDA_EA: 0x819AFEF
// IDA_NAME: _ZN4ARAD9SingletonI19AvatarConvertServerE3GetEv
// DEMANGLED: ARAD::Singleton<AvatarConvertServer>::Get(void)
AvatarConvertServer* Singleton<AvatarConvertServer>::Get() {
    static AvatarConvertServer object;
    return &object;
}

}  // namespace ARAD

// IDA_EA: 0x8190200 / 0x8190246
AvatarRechargeServer::AvatarRechargeServer()
    : value_54_(0), value_58_(false), padding_59_{0, 0, 0} {}
AvatarRechargeServer::~AvatarRechargeServer() {}

// IDA_EA: 0x8190266
// IDA_NAME: _ZN20AvatarRechargeServer4TestEv
// DEMANGLED: AvatarRechargeServer::Test(void)
void AvatarRechargeServer::Test() {
    // The original function body is empty.
}

// IDA_EA: 0x819026C
// IDA_NAME: _ZN20AvatarRechargeServer4LoadEv
// DEMANGLED: AvatarRechargeServer::Load(void)
bool AvatarRechargeServer::Load() {
    cMyTrace trace("virtual bool AvatarRechargeServer::Load()", 34, 9, true,
                   false);
    trace("\t- Loading Avatar Recharge script - ");
    return script_.loadScript() == 0;
}

// IDA_EA: 0x81902DE
// IDA_NAME: _ZN20AvatarRechargeServer7DestroyEv
// DEMANGLED: AvatarRechargeServer::Destroy(void)
int AvatarRechargeServer::Destroy() {
    return 1;
}

// IDA_EA: 0x819114C
// IDA_NAME: _ZN20AvatarRechargeServer22GetRateFromEmblemGradeEi
// DEMANGLED: AvatarRechargeServer::GetRateFromEmblemGrade(int)
int AvatarRechargeServer::GetRateFromEmblemGrade(int grade) {
    return script_.getEmblemRecharge(grade);
}

// IDA_EA: 0x819155E / 0x8191594
EmblemCompoundServer::EmblemCompoundServer() {}
EmblemCompoundServer::~EmblemCompoundServer() {}

// IDA_EA: 0x81915BA / 0x819162C
bool EmblemCompoundServer::Load() {
    cMyTrace trace("virtual bool EmblemCompoundServer::Load()", 455, 9, true,
                   false);
    trace("\t- Loading Emblem Compound script - ");
    return script_.loadScript() == 0;
}

int EmblemCompoundServer::Destroy() {
    return 1;
}

// IDA_EA: 0x81921B6 / 0x81921EC
AvatarConvertServer::AvatarConvertServer() {}
AvatarConvertServer::~AvatarConvertServer() {}

// IDA_EA: 0x819220C
// IDA_NAME: _ZN19AvatarConvertServer4TestEv
// DEMANGLED: AvatarConvertServer::Test(void)
void AvatarConvertServer::Test() {
    // The original function body is empty.
}

// IDA_EA: 0x81930BA
// IDA_NAME: _ZN19AvatarConvertServer4LoadEv
// DEMANGLED: AvatarConvertServer::Load(void)
bool AvatarConvertServer::Load() {
    cMyTrace trace("virtual bool AvatarConvertServer::Load()", 895, 9, true,
                   false);
    trace("\t- Loading Avatar Convert script - ");
    return script_.loadScript() == 0;
}

// IDA_EA: 0x819312C
// IDA_NAME: _ZN19AvatarConvertServer7DestroyEv
// DEMANGLED: AvatarConvertServer::Destroy(void)
int AvatarConvertServer::Destroy() {
    return 1;
}
