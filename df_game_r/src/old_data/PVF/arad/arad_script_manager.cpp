#include "old_data/PVF/arad/arad_script_manager.h"

// Compile this provider with the data-manager owner header as a hard
// redefinition gate: both paths must resolve to the one ScriptInterface ABI.
#include "old_data/PVF/data/arad_data_manager.h"
#include "old_game/economy/mall/cerashop_add_restrict_runtime.h"
#include "old_game/appearance/avatar/arad_avatar_runtime.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/activity/schedule/levelup_support_script_services.h"

#include <cstdio>
#include <cstdlib>

class cMyTrace {
public:
    cMyTrace(const char* function, int line, int level, bool prefix,
             bool newline);
    int operator()(const char* format, ...);

private:
    char storage_[24];
};

namespace ARAD {

// IDA_EA: 0x82A69A0
// IDA_NAME: _ZN4ARAD13ScriptManagerC2Ev
// DEMANGLED: ARAD::ScriptManager::ScriptManager(void)
ScriptManager::ScriptManager()
    : Singleton<ScriptManager>(), scripts_() {}

// IDA_EA: 0x82B3D08
// IDA_NAME: _ZN4ARAD13ScriptManagerD2Ev
// DEMANGLED: ARAD::ScriptManager::~ScriptManager()
ScriptManager::~ScriptManager() {}

// IDA_EA: 0x82AB470
// IDA_NAME: _ZN4ARAD9SingletonINS_13ScriptManagerEEC2Ev
// DEMANGLED: ARAD::Singleton<ARAD::ScriptManager>::Singleton(void)
template <>
Singleton<ScriptManager>::Singleton() {}

// IDA_EA: 0x82B3D1B
// IDA_NAME: _ZN4ARAD9SingletonINS_13ScriptManagerEE3GetEv
// DEMANGLED: ARAD::Singleton<ARAD::ScriptManager>::Get(void)
template <>
ScriptManager* Singleton<ScriptManager>::Get() {
    static ScriptManager object;
    return &object;
}

// IDA_EA: 0x81A11DC
// IDA_NAME: _ZN4ARAD13ScriptManager10LoadScriptEv
// DEMANGLED: ARAD::ScriptManager::LoadScript(void)
bool ScriptManager::LoadScript() {
    Init();

    std::set<ScriptInterface*>::const_iterator it = scripts_.begin();
    const std::set<ScriptInterface*>::const_iterator end = scripts_.end();
    while (it != end) {
        if (!(*it)->Load()) {
            cMyTrace trace("bool ARAD::ScriptManager::LoadScript()", 18,
                           9, true, true);
            trace("Fail");
            return false;
        }

        cMyTrace trace("bool ARAD::ScriptManager::LoadScript()", 21,
                       9, false, true);
        trace("Success");
        ++it;
    }
    return true;
}

// IDA_EA: 0x81A12F8
// IDA_NAME: _ZN4ARAD13ScriptManager13DestroyScriptEv
// DEMANGLED: ARAD::ScriptManager::DestroyScript(void)
bool ScriptManager::DestroyScript() {
    std::set<ScriptInterface*>::const_iterator it = scripts_.begin();
    const std::set<ScriptInterface*>::const_iterator end = scripts_.end();
    while (it != end) {
        (*it)->Destroy();
        ++it;
    }
    return true;
}

// IDA_EA: 0x81A136C
// IDA_NAME: _ZN4ARAD13ScriptManager12RegistScriptEPNS_15ScriptInterfaceEi
// DEMANGLED: ARAD::ScriptManager::RegistScript(ARAD::ScriptInterface *,int)
bool ScriptManager::RegistScript(ScriptInterface* script, int source_line) {
    if (scripts_.find(script) != scripts_.end()) {
        std::printf(
            "Arad_ScriptManager::RegistScript() Duplicate Class : Line(%d)",
            source_line);
        std::exit(0);
    }
    return scripts_.insert(script).second;
}

// IDA_EA: 0x81A13F6
// IDA_NAME: _ZN4ARAD13ScriptManager4InitEv
// DEMANGLED: ARAD::ScriptManager::Init(void)
bool ScriptManager::Init() {
    RegistScript(static_cast<ScriptInterface*>(
                     Singleton<AvatarFixedHiddenOptionServer>::Get()),
                 19);
    RegistScript(static_cast<ScriptInterface*>(
                     Singleton<AvatarRechargeServer>::Get()),
                 19);
    RegistScript(static_cast<ScriptInterface*>(
                     Singleton<EmblemCompoundServer>::Get()),
                 19);
    RegistScript(static_cast<ScriptInterface*>(
                     Singleton<AvatarConvertServer>::Get()),
                 19);
    RegistScript(static_cast<ScriptInterface*>(
                     Singleton<CerashopAddRestrict::Manager>::Get()),
                 19);
    RegistScript(static_cast<ScriptInterface*>(
                     Singleton<LevelupSupportEventManger>::Get()),
                 19);
    RegistScript(static_cast<ScriptInterface*>(Singleton<Arad_DataManager>::Get()),
                 19);
    // The old CreateDnf server predates the named ScriptInterface base, but
    // its object begins with the same two-slot Load/Destroy vtable.  IDA's
    // original Init performs this zero-offset conversion.
    RegistScript(reinterpret_cast<ScriptInterface*>(
                     Singleton<CreateDnfEventSciprtServer>::Get()),
                 19);
    return RegistScript(static_cast<ScriptInterface*>(
                            Singleton<LevelupSupport2ndEventManger>::Get()),
                        19);
}

}  // namespace ARAD
