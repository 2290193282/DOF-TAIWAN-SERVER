#ifndef OLDSRV_OLD_DATA_PVF_ARAD_CERASHOP_ADD_RESTRICT_SCRIPT_SERVICE_H_
#define OLDSRV_OLD_DATA_PVF_ARAD_CERASHOP_ADD_RESTRICT_SCRIPT_SERVICE_H_

#include <stdint.h>

#include <map>

#include "old_data/PVF/arad/arad_script_interface.h"
#include "old_data/PVF/arad/arad_singleton.h"

namespace CerashopAddRestrict {

enum RestrictType {
    RESTRICT_ACCOUNT = 0,
    RESTRICT_CHARACTER = 1,
};

// IDA unit: CerashopAddRestrict::InfoDaily, 0x1c bytes on i386.
struct InfoDaily {
    InfoDaily(RestrictType type_value,
              uint32_t ipg_no_value,
              uint32_t limit_value,
              int32_t begin_time_value,
              int32_t end_time_value,
              int interval_value,
              int count_value)
        : type(type_value), ipg_no(ipg_no_value), limit(limit_value),
          begin_time(begin_time_value), end_time(end_time_value),
          interval(interval_value), count(count_value) {}

    RestrictType type;
    uint32_t ipg_no;
    uint32_t limit;
    int32_t begin_time;
    int32_t end_time;
    int interval;
    int count;
};

class ScriptLoader {
public:
    ScriptLoader() : restrict_infos_() {}
    ~ScriptLoader();

    int LoadScript();

private:
    int LoadScriptDaily(RestrictType type);
    std::map<uint32_t, InfoDaily*> restrict_infos_;
};

class Manager : public ARAD::Singleton<Manager>, public ARAD::ScriptInterface {
public:
    Manager();
    ~Manager();

    virtual bool Load();
    virtual int Destroy();

private:
    ScriptLoader script_loader_;
};

}  // namespace CerashopAddRestrict

namespace ARAD {
template <>
Singleton<CerashopAddRestrict::Manager>::Singleton();
template <>
CerashopAddRestrict::Manager*
Singleton<CerashopAddRestrict::Manager>::Get();
}  // namespace ARAD

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CerashopAddRestrict::InfoDaily) == 0x1cu,
              "CerashopAddRestrict InfoDaily i386 ABI must stay 0x1c");
static_assert(sizeof(CerashopAddRestrict::ScriptLoader) == 0x18u,
              "CerashopAddRestrict ScriptLoader i386 ABI must stay 0x18");
static_assert(sizeof(CerashopAddRestrict::Manager) == 0x1cu,
              "CerashopAddRestrict Manager i386 ABI must stay 0x1c");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_ARAD_CERASHOP_ADD_RESTRICT_SCRIPT_SERVICE_H_
