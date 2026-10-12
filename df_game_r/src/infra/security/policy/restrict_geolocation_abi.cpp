#include "infra/security/policy/restrict_geolocation_abi.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/legacy_dispatch/geo_reject_payload.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>

// CentOS 6 supplies the old libGeoIP runtime but not its development header.
// These are the three imported C boundaries proven by the original i386 call
// sites at 0x8170A5C, 0x8170A96, and 0x8170FB0.
extern "C" {
GeoIP* GeoIP_new(int flags);
void GeoIP_delete(GeoIP* geo_ip);
const char* GeoIP_country_code_by_addr(GeoIP* geo_ip, const char* peer_ip);
}

ARAD::Singleton<RestrictGeolocation>::Singleton() {}

// IDA_EA: 0x8162679
// IDA_NAME: _ZN4ARAD9SingletonI19RestrictGeolocationE3GetEv
RestrictGeolocation* ARAD::Singleton<RestrictGeolocation>::Get() {
    static RestrictGeolocation instance;
    return &instance;
}

// IDA_EA: 0x817097C
// IDA_NAME: _ZN19RestrictGeolocationC2Ev
RestrictGeolocation::RestrictGeolocation()
    : ARAD::Singleton<RestrictGeolocation>(),
      geo_ip_(0),
      allow_country_codes_(11) {}

// IDA_EA: 0x81709FE
// IDA_NAME: _ZN19RestrictGeolocationD2Ev
RestrictGeolocation::~RestrictGeolocation() {
    closeGeolocation();
}

// IDA_EA: 0x8170A46
// IDA_NAME: _ZN19RestrictGeolocation15initGeolocationEv
bool RestrictGeolocation::initGeolocation() {
    if (!geo_ip_) {
        geo_ip_ = GeoIP_new(0);
        if (!geo_ip_) {
            return false;
        }
    }

    loadData();
    return true;
}

// IDA_EA: 0x8170A88
// IDA_NAME: _ZN19RestrictGeolocation16closeGeolocationEv
void RestrictGeolocation::closeGeolocation() {
    GeoIP_delete(geo_ip_);
}

// IDA_EA: 0x8170A9E
// IDA_NAME: _ZN19RestrictGeolocation15addAllowCountryESs
void RestrictGeolocation::addAllowCountry(std::string country_code) {
    if (allow_country_codes_.find(country_code) == allow_country_codes_.end()) {
        allow_country_codes_.insert(country_code);
    }
}

// IDA_EA: 0x8170B16
// IDA_NAME: _ZN19RestrictGeolocation17clearAllowCountryEv
void RestrictGeolocation::clearAllowCountry() {
    allow_country_codes_.clear();
}

// IDA_EA: 0x8170B2C
// IDA_NAME: _ZN19RestrictGeolocation7isAllowESsSs
bool RestrictGeolocation::isAllow(std::string account_name, std::string peer_ip) {
    if (!geo_ip_) {
        cMyTrace trace("bool RestrictGeolocation::isAllow(std::string, std::string)",
                       68, 5);
        trace("[Taiwan, GeoIP] Geo instance is null.");
        return true;
    }

    char country_code[16] = {};
    if (GeoIP_country_code_by_addr(geo_ip_, peer_ip.c_str())) {
        std::strcpy(country_code,
                    GeoIP_country_code_by_addr(geo_ip_, peer_ip.c_str()));
    }

    if (isAllowCountryCode(country_code)) {
        cMyTrace trace("bool RestrictGeolocation::isAllow(std::string, std::string)",
                       78, 0);
        trace("[Taiwan, GeoIP] Success Account:%s, IP:%s, CountryCode:%s",
              account_name.c_str(), peer_ip.c_str(), country_code);
        return true;
    }

    if (G_CEnvironment()->get_server_group() == 3) {
        cMyTrace trace("bool RestrictGeolocation::isAllow(std::string, std::string)",
                       85, 0);
        trace("[Taiwan, GeoIP] Success Global server group. Account:%s, IP:%s, "
              "CountryCode:%s",
              account_name.c_str(), peer_ip.c_str(), country_code);
        return true;
    }

    cMyTrace trace("bool RestrictGeolocation::isAllow(std::string, std::string)",
                   90, 0);
    trace("[Taiwan, GeoIP] Fail Account:%s, IP:%s, CountryCode:%s",
          account_name.c_str(), peer_ip.c_str(), country_code);

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire(
            "localtaiwan/System/RestrictGeolocation.cpp", 92),
        true);
    *guard << 0x11b;
    *guard << -1;
    SIG_SAVE_GEO_REJECT* const payload = guard->GetInBuffer<SIG_SAVE_GEO_REJECT>();
    std::strncpy(payload->country_code, country_code, sizeof(payload->country_code));
    std::strncpy(payload->reject_ip, peer_ip.c_str(), sizeof(payload->reject_ip));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return false;
}

// IDA_EA: 0x8170ECC
// IDA_NAME: _ZN19RestrictGeolocation7isAllowESs
bool RestrictGeolocation::isAllow(std::string country_code) {
    return isAllowCountryCode(country_code);
}

// IDA_EA: 0x8170F3A
// IDA_NAME: _ZN19RestrictGeolocation14getCountryCodeEPcjSs
uint8_t RestrictGeolocation::getCountryCode(char* output,
                                            unsigned int output_size,
                                            std::string peer_ip) {
    if (!geo_ip_) {
        cMyTrace trace(
            "void RestrictGeolocation::getCountryCode(char*, unsigned int, std::string)",
            126, 5);
        trace("[Taiwan, GeoIP] Geo instance is null.");
    }

    std::memset(output, 0, output_size);
    if (!GeoIP_country_code_by_addr(geo_ip_, peer_ip.c_str())) {
        return 0;
    }

    // The original i386 body returns the low byte left in EAX by strcpy.
    return static_cast<uint8_t>(reinterpret_cast<uintptr_t>(
        std::strcpy(output, GeoIP_country_code_by_addr(geo_ip_, peer_ip.c_str()))));
}

// IDA_EA: 0x8170FEC
// IDA_NAME: _ZN19RestrictGeolocation8loadDataEv
void RestrictGeolocation::loadData() {
    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire(
            "localtaiwan/System/RestrictGeolocation.cpp", 139),
        true);
    Taiwan::internal_stream(guard, 4, -1);
    clearAllowCountry();
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x817109A
// IDA_NAME: _ZN19RestrictGeolocation18isAllowCountryCodeESs
bool RestrictGeolocation::isAllowCountryCode(std::string country_code) {
    return allow_country_codes_.find(country_code) != allow_country_codes_.end();
}
