#include "old_data/PVF/arad/cerashop_add_restrict_script_service.h"

#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <cstdio>
#include <ctime>
#include <string>
#include <utility>

namespace CerashopAddRestrict {
namespace {

bool ReadTime(int16_t values[4]) {
    bool success = false;
    for (int index = 0; index != 4; ++index) {
        values[index] = static_cast<int16_t>(ScanInt(&success));
        if (!success) {
            return false;
        }
    }
    return true;
}

int32_t ToTime(const int16_t values[4]) {
    time_t now = time(0);
    tm local_time;
    localtime_r(&now, &local_time);
    local_time.tm_year = values[0] - 1900;
    local_time.tm_mon = values[1] - 1;
    local_time.tm_mday = values[2];
    local_time.tm_hour = values[3];
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    return static_cast<int32_t>(mktime(&local_time));
}

}  // namespace

ScriptLoader::~ScriptLoader() {
    for (std::map<uint32_t, InfoDaily*>::iterator it = restrict_infos_.begin();
         it != restrict_infos_.end(); ++it) {
        delete it->second;
    }
}

int ScriptLoader::LoadScriptDaily(RestrictType type) {
    std::string token;
    bool is_type = false;
    bool success = false;
    while (1) {
        const int ipg_no = ScanTypeOrInt(is_type, token, success, true);
        if (is_type || !success) {
            break;
        }
        const int limit = ScanInt(static_cast<bool*>(0));
        const int first_count = ScanInt(static_cast<bool*>(0));
        int16_t begin[4];
        int16_t end[4];
        if (!ReadTime(begin) || !ReadTime(end)) {
            return -1;
        }
        const int interval = ScanInt(static_cast<bool*>(0));
        if (restrict_infos_.find(static_cast<uint32_t>(ipg_no)) !=
            restrict_infos_.end()) {
            std::printf("already registed ipg_no:%u", ipg_no);
            return -1;
        }
        restrict_infos_.insert(std::make_pair(
            static_cast<uint32_t>(ipg_no),
            new InfoDaily(type, static_cast<uint32_t>(ipg_no),
                          static_cast<uint32_t>(limit), ToTime(begin),
                          ToTime(end), first_count, interval)));
    }
    return 0;
}

int ScriptLoader::LoadScript() {
    if (!loadRDARScriptFile("Etc/", "NewCashShop_Restrict.etc")) {
        return -1;
    }
    std::string token;
    int result = 0;
    while (ScanType(token, true) == 1) {
        if (token == "[account restrict]") {
            result = LoadScriptDaily(RESTRICT_ACCOUNT);
        } else if (token == "[character restrict]") {
            result = LoadScriptDaily(RESTRICT_CHARACTER);
        }
        if (result < 0) {
            break;
        }
    }
    unloadRDARScriptFile();
    return result;
}

Manager::Manager() : script_loader_() {}
Manager::~Manager() {}

bool Manager::Load() {
    std::printf("[!] Loading CeraShop Add Restrict Type script - ");
    return script_loader_.LoadScript() == 0;
}

int Manager::Destroy() {
    return 1;
}

}  // namespace CerashopAddRestrict

namespace ARAD {
template <>
Singleton<CerashopAddRestrict::Manager>::Singleton() {}

template <>
CerashopAddRestrict::Manager*
Singleton<CerashopAddRestrict::Manager>::Get() {
    static CerashopAddRestrict::Manager object;
    return &object;
}
}  // namespace ARAD
