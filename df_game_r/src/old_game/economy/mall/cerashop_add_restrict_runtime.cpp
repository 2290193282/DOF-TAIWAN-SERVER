#include "old_game/economy/mall/cerashop_add_restrict_runtime.h"

#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "core/config/env/environment.h"
#include "old_data/db/auction_cashshop/db_load_cerashop_add_restrict_type.h"
#include "old_data/db/auction_cashshop/db_save_cerashop_add_restrict_type.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

#include <cstdio>
#include <ctime>
#include <stdexcept>
#include <string>
#include <utility>

namespace CerashopAddRestrict {

// IDA_EA: 0x8ABF826
// IDA_NAME: _ZN19CerashopAddRestrict9InfoDailyC2ENS_12RestrictType1TEjjllii
InfoDaily::InfoDaily(RestrictType::T type_value,
                     uint32_t ipg_no_value,
                     uint32_t limit_value,
                     long begin_time_value,
                     long end_time_value,
                     int interval_value,
                     int count_value)
    : type(type_value),
      ipg_no(ipg_no_value),
      limit(limit_value),
      begin_time(begin_time_value),
      end_time(end_time_value),
      interval(interval_value),
      count(count_value) {}

// IDA_EA: 0x80E243C
// IDA_NAME: _ZN19CerashopAddRestrict11DBInputDataC2Ev
DBInputData::DBInputData() : parameter(0), restrict_info(0) {}

// IDA_EA: 0x80E0A43
// IDA_NAME: _ZN19CerashopAddRestrictL18FindUserInfoDetailERSt3mapIjjSt4lessIjESaISt4pairIKjjEEERS0_IjPNS_10paramDailyES2_SaIS3_IS4_SA_EEEjPNS_9InfoDailyEj
static bool FindUserInfoDetail(RestrictValueMap& values,
                               ParamDailyMap& parameters,
                               unsigned int ipg_no,
                               InfoDaily* restrict_info,
                               unsigned int count) {
    unsigned int& pending_count = values[ipg_no];
    pending_count += count;

    ParamDailyMap::iterator parameter = parameters.find(ipg_no);
    if (parameter == parameters.end()) {
        return pending_count <= restrict_info->limit;
    }

    const time_t now = time(0);
    if (parameter->second->next_date <= static_cast<uint32_t>(now)) {
        parameter->second->count = 0;
        return pending_count <= restrict_info->limit;
    }
    return parameter->second->count + pending_count <= restrict_info->limit;
}

// IDA_EA: 0x80E10D7
// IDA_NAME: _ZN19CerashopAddRestrictL10GetCalcDayEPKNS_9InfoDailyE
static long GetCalcDay(const InfoDaily* restrict_info) {
    int days = static_cast<int>(
                   (restrict_info->end_time - restrict_info->begin_time) /
                   86400L) +
               1;
    if (restrict_info->interval > 0) {
        days = restrict_info->interval;
    }

    time_t next = static_cast<time_t>(restrict_info->begin_time);
    const time_t now = time(0);
    while (next <= now) {
        next += static_cast<time_t>(86400 * days);
    }

    struct tm local_time;
    localtime_r(&next, &local_time);
    local_time.tm_hour = G_CEnvironment()->get_fatigue_time();
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    return static_cast<long>(mktime(&local_time));
}

// IDA_EA: 0x80E1196
// IDA_NAME: _ZN19CerashopAddRestrict18InputBuyInfoDetailERSt3mapIjPNS_10paramDailyESt4lessIjESaISt4pairIKjS2_EEEPKNS_9InfoDailyEjj
paramDaily* InputBuyInfoDetail(ParamDailyMap& parameters,
                               const InfoDaily* restrict_info,
                               unsigned int ipg_no,
                               unsigned int count) {
    paramDaily* parameter = 0;
    ParamDailyMap::iterator found = parameters.find(ipg_no);
    if (found != parameters.end()) {
        parameter = found->second;
        parameter->count += count;
    } else {
        parameter = new paramDaily;
        parameter->ipg_no = ipg_no;
        parameter->count = count;
        parameter->next_date = 0;
        parameters.insert(std::make_pair(ipg_no, parameter));
    }
    parameter->next_date = static_cast<uint32_t>(GetCalcDay(restrict_info));
    return parameter;
}

// IDA_EA: 0x8ABF164
// IDA_NAME: _ZN19CerashopAddRestrict8readTimeEPs
int readTime(short values[4]) {
    static const char* const labels[4] = {
        "start year", "start month", "start day", "start time",
    };
    for (int index = 0; index != 4; ++index) {
        bool success = false;
        values[index] = static_cast<short>(ScanInt(&success));
        if (!success) {
            throw labels[index];
        }
    }
    return 1;
}

// IDA_EA: 0x8ABF33F
// IDA_NAME: _ZN19CerashopAddRestrict10uint32TimeEPs
long uint32Time(short values[4]) {
    time_t now = time(0);
    struct tm* local_time = localtime(&now);
    local_time->tm_year = values[0] - 1900;
    local_time->tm_mon = values[1] - 1;
    local_time->tm_mday = values[2];
    local_time->tm_hour = values[3];
    local_time->tm_min = 0;
    local_time->tm_sec = 0;
    return static_cast<long>(mktime(local_time));
}

// IDA_EA: 0x80E2414
// IDA_NAME: _ZN19CerashopAddRestrict12ScriptLoaderC2Ev
ScriptLoader::ScriptLoader() : restrict_infos_() {}

// IDA_EA: 0x80E2428
// IDA_NAME: _ZN19CerashopAddRestrict12ScriptLoaderD2Ev
ScriptLoader::~ScriptLoader() {}

// IDA_EA: 0x8ABF3CC
// IDA_NAME: _ZN19CerashopAddRestrict12ScriptLoader10LoadScriptEv
int ScriptLoader::LoadScript() {
    if (!loadRDARScriptFile("Etc/", "NewCashShop_Restrict.etc")) {
        return -1;
    }

    std::string token;
    int result = 0;
    while (ScanType(token, true) == 1) {
        if (token == "[account restrict]") {
            if (LoadScriptDaily(RestrictType::ACCOUNT) < 0) {
                result = -1;
                break;
            }
        } else if (token == "[character restrict]") {
            if (LoadScriptDaily(RestrictType::CHARACTER) < 0) {
                result = -1;
                break;
            }
        }
    }
    return result;
}

// IDA_EA: 0x8ABF4DE
// IDA_NAME: _ZN19CerashopAddRestrict12ScriptLoader15LoadScriptDailyENS_12RestrictType1TE
int ScriptLoader::LoadScriptDaily(RestrictType::T type) {
    std::string token;
    bool is_type = false;
    bool success = false;
    while (true) {
        const int ipg_no = ScanTypeOrInt(is_type, token, success, true);
        if (is_type || !success) {
            break;
        }

        const int limit = ScanInt(static_cast<bool*>(0));
        const int interval = ScanInt(static_cast<bool*>(0));
        short begin[4] = {0, 0, 0, 0};
        short end[4] = {0, 0, 0, 0};
        if (readTime(begin) != 1 || readTime(end) != 1) {
            return -1;
        }
        const int count = ScanInt(static_cast<bool*>(0));
        if (restrict_infos_.find(static_cast<uint32_t>(ipg_no)) !=
            restrict_infos_.end()) {
            std::printf("already registed ipg_no:%u", ipg_no);
            return -1;
        }

        InfoDaily* const info = new InfoDaily(
            type, static_cast<uint32_t>(ipg_no),
            static_cast<uint32_t>(limit), uint32Time(begin), uint32Time(end),
            interval, count);
        restrict_infos_.insert(std::make_pair(static_cast<uint32_t>(ipg_no),
                                              info));
    }
    return 0;
}

// IDA_EA: 0x8ABF704
// IDA_NAME: _ZN19CerashopAddRestrict12ScriptLoader11ClearScriptEv
void ScriptLoader::ClearScript() {}

// IDA_EA: 0x8ABF70A
// IDA_NAME: _ZN19CerashopAddRestrict12ScriptLoader9FindIpgNoEj
bool ScriptLoader::FindIpgNo(unsigned int ipg_no) {
    return restrict_infos_.find(ipg_no) != restrict_infos_.end();
}

// IDA_EA: 0x8ABF766
// IDA_NAME: _ZN19CerashopAddRestrict12ScriptLoader15GetRestrictInfoEj
InfoDaily* ScriptLoader::GetRestrictInfo(unsigned int ipg_no) {
    std::map<unsigned int, InfoDaily*>::iterator found =
        restrict_infos_.find(ipg_no);
    return found == restrict_infos_.end() ? 0 : found->second;
}

// IDA_EA: 0x80E09BC
// IDA_NAME: _ZN19CerashopAddRestrict7ManagerC2Ev
Manager::Manager()
    : ARAD::ScriptInterface(), ARAD::Singleton<Manager>(), script_loader_() {}

// IDA_EA: 0x80E0A24
// IDA_NAME: _ZN19CerashopAddRestrict7ManagerD2Ev
Manager::~Manager() {}

// IDA_EA: 0x80E0E08
// IDA_NAME: _ZN19CerashopAddRestrict7Manager4LoadEv
bool Manager::Load() {
    std::printf("[!] Loading CeraShop Add Restrict Type script - ");
    return script_loader_.LoadScript() == 0;
}

// IDA_EA: 0x80E0E40
// IDA_NAME: _ZN19CerashopAddRestrict7Manager7DestroyEv
int Manager::Destroy() { return 1; }

// IDA_EA: 0x80E0BEE
// IDA_NAME: _ZN19CerashopAddRestrict7Manager19CheckBuyableProductEP5CUserjji
bool Manager::CheckBuyableProduct(CUser* user,
                                  unsigned int ipg_no,
                                  unsigned int count,
                                  int index) {
    if (!index) {
        user->cerashop_restrict_.value_map_0.clear();
        user->cerashop_restrict_.value_map_1.clear();
    }

    InfoDaily* const restrict_info = script_loader_.GetRestrictInfo(ipg_no);
    if (!restrict_info) {
        return true;
    }
    if (restrict_info->end_time < static_cast<long>(time(0))) {
        return false;
    }
    if (restrict_info->type == RestrictType::ACCOUNT) {
        return FindUserInfoDetail(user->cerashop_restrict_.value_map_0,
                                  user->cerashop_restrict_.parameter_map_0,
                                  ipg_no, restrict_info, count);
    }
    if (restrict_info->type == RestrictType::CHARACTER) {
        return FindUserInfoDetail(user->cerashop_restrict_.value_map_1,
                                  user->cerashop_restrict_.parameter_map_1,
                                  ipg_no, restrict_info, count);
    }
    return false;
}

// IDA_EA: 0x80E0CE2
// IDA_NAME: _ZN19CerashopAddRestrict7Manager21IsBuyableRestrictItemEP5CUserjji
int Manager::IsBuyableRestrictItem(CUser* user,
                                   unsigned int ipg_no,
                                   unsigned int count,
                                   int index) {
    return ARAD::Singleton<Manager>::Get()->CheckBuyableProduct(
        user, ipg_no, count, index);
}

// IDA_EA: 0x80E0E4A
// IDA_NAME: _ZN19CerashopAddRestrict7Manager9FindIpgNoEj
bool Manager::FindIpgNo(unsigned int ipg_no) {
    return script_loader_.FindIpgNo(ipg_no);
}

// IDA_EA: 0x80E0D14
// IDA_NAME: _ZN19CerashopAddRestrict7Manager25UpdateBuyableRestrictItemEP5CUserjj
int Manager::UpdateBuyableRestrictItem(CUser* user,
                                       unsigned int ipg_no,
                                       unsigned int count) {
    Manager* const manager = ARAD::Singleton<Manager>::Get();
    if (!manager->FindIpgNo(ipg_no)) {
        return 0;
    }

    DBInputData input;
    if (!manager->InputBuyInfo(user, ipg_no, count, input)) {
        return 0;
    }
    DB_SaveCerashopAddRestrictType::makeRequest(user, input);
    SendSyncPacket(user);
    return 1;
}

// IDA_EA: 0x80E0D90
// IDA_NAME: _ZN19CerashopAddRestrict7Manager26LoadRequestBuyRestrictItemEP5CUser
int Manager::LoadRequestBuyRestrictItem(CUser* user) {
    DB_LoadCerashopAddRestrictType::makeRequest(user);
    return 1;
}

// IDA_EA: 0x80E0DA4
// IDA_NAME: _ZN19CerashopAddRestrict7Manager20ClearBuyRestrictItemEP5CUser
void Manager::ClearBuyRestrictItem(CUser* user) {
    user->cerashop_restrict_.parameter_map_0.clear();
    user->cerashop_restrict_.parameter_map_1.clear();
    user->cerashop_restrict_.value_map_0.clear();
    user->cerashop_restrict_.value_map_1.clear();
}

// IDA_EA: 0x80E0DEC
// IDA_NAME: _ZN19CerashopAddRestrict7Manager14SendSyncPacketEP5CUser
void Manager::SendSyncPacket(CUser* user) {
    ARAD::Singleton<Manager>::Get()->makeSyncPacket(user);
}

// IDA_EA: 0x80E0E68
// IDA_NAME: _ZN19CerashopAddRestrict7Manager20GetItemListForPacketERSt6vectorIjSaIjEERSt3mapIjPNS_10paramDailyESt4lessIjESaISt4pairIKjS7_EEE
void Manager::GetItemListForPacket(std::vector<unsigned int>& items,
                                   ParamDailyMap& parameters) {
    for (ParamDailyMap::iterator it = parameters.begin();
         it != parameters.end(); ++it) {
        InfoDaily* const restrict_info = script_loader_.GetRestrictInfo(it->first);
        if (restrict_info && restrict_info->limit <= it->second->count) {
            items.push_back(it->first);
        }
    }
}

// IDA_EA: 0x80E0F3A
// IDA_NAME: _ZN19CerashopAddRestrict7Manager14makeSyncPacketEP5CUser
void Manager::makeSyncPacket(CUser* user) {
    UserInfo& info = user->cerashop_restrict_;
    const size_t parameter_count = info.parameter_map_0.size() +
                                   info.parameter_map_1.size();
    if (!parameter_count) {
        return;
    }

    std::vector<unsigned int> items;
    GetItemListForPacket(items, info.parameter_map_0);
    GetItemListForPacket(items, info.parameter_map_1);
    if (items.empty()) {
        return;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(0,
                      ENUM_NOTIPACKET_USER_BUY_RESTRICT_UPDATE_ITEM_LIST);
    packet.put_int(static_cast<int>(items.size()));
    for (std::vector<unsigned int>::const_iterator it = items.begin();
         it != items.end(); ++it) {
        packet.put_int(static_cast<int>(*it));
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x80E129A
// IDA_NAME: _ZN19CerashopAddRestrict7Manager12InputBuyInfoEP5CUserjjRNS_11DBInputDataE
bool Manager::InputBuyInfo(CUser* user,
                           unsigned int ipg_no,
                           unsigned int count,
                           DBInputData& input) {
    InfoDaily* const restrict_info = script_loader_.GetRestrictInfo(ipg_no);
    if (!restrict_info) {
        return false;
    }

    input.restrict_info = restrict_info;
    if (restrict_info->type == RestrictType::CHARACTER) {
        user->cerashop_restrict_.value_map_1.erase(ipg_no);
        input.parameter = InputBuyInfoDetail(
            user->cerashop_restrict_.parameter_map_1, restrict_info,
            ipg_no, count);
    } else if (restrict_info->type == RestrictType::ACCOUNT) {
        user->cerashop_restrict_.value_map_0.erase(ipg_no);
        input.parameter = InputBuyInfoDetail(
            user->cerashop_restrict_.parameter_map_0, restrict_info,
            ipg_no, count);
    } else {
        return false;
    }
    return input.parameter != 0;
}

// IDA_EA: 0x80E142A
// IDA_NAME: _ZN19CerashopAddRestrict7Manager11LoadProcessERSt3mapIjjSt4lessIjESaISt4pairIKjjEEERS1_IjPNS_10paramDailyES3_SaIS4_IS5_SB_EEEP35SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE
bool Manager::LoadProcess(RestrictValueMap& values,
                          ParamDailyMap& parameters,
                          SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE* load) {
    values.clear();
    for (uint32_t index = 0; index < load->row_count; ++index) {
        const SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE::RestrictEntry& entry =
            load->entries[index];
        paramDaily* const parameter = new paramDaily;
        parameter->ipg_no = entry.ipg_no;
        parameter->count = entry.count;
        parameter->next_date = entry.next_date;
        parameters.insert(std::make_pair(parameter->ipg_no, parameter));
    }
    return true;
}

}  // namespace CerashopAddRestrict

namespace ARAD {

// IDA_EA: 0x80E25F0
// IDA_NAME: _ZN4ARAD9SingletonIN19CerashopAddRestrict7ManagerEEC2Ev
template <>
Singleton<CerashopAddRestrict::Manager>::Singleton() {}

// IDA_EA: 0x80E274D
// IDA_NAME: _ZN4ARAD9SingletonIN19CerashopAddRestrict7ManagerEE3GetEv
// DEMANGLED: ARAD::Singleton<CerashopAddRestrict::Manager>::Get(void)
// The original is the usual guarded function-local singleton: construction,
// atexit destruction, then the address of the same Manager object.
template <>
CerashopAddRestrict::Manager*
Singleton<CerashopAddRestrict::Manager>::Get() {
    static CerashopAddRestrict::Manager object;
    return &object;
}

}  // namespace ARAD
