#ifndef OLDSRV_GLOBAL_RESTRICT_GEOLOCATION_ABI_H_
#define OLDSRV_GLOBAL_RESTRICT_GEOLOCATION_ABI_H_

#include <stdint.h>

#include <boost/unordered_set.hpp>

#include <string>


class GeoIP;
class RestrictGeolocation;

namespace ARAD {

#ifndef OLDSRV_ARAD_SINGLETON_TEMPLATE_DECLARATION_H_
#define OLDSRV_ARAD_SINGLETON_TEMPLATE_DECLARATION_H_

template <typename T>
class Singleton {
protected:
    Singleton();

public:
    static T* Get();
};

#endif  // OLDSRV_ARAD_SINGLETON_TEMPLATE_DECLARATION_H_

template <>
class Singleton<RestrictGeolocation> {
protected:
    Singleton();

public:
    static RestrictGeolocation* Get();
};

}  // namespace ARAD

// IDA 0x817097C: GeoIP* is at +0x00 and the Boost 1.48 unordered set starts
// at +0x04.  On the original i386 ABI the set is 0x18 bytes, so the complete
// public object is exactly 0x1c bytes.
class RestrictGeolocation : public ARAD::Singleton<RestrictGeolocation> {
public:
    RestrictGeolocation();
    ~RestrictGeolocation();

    bool initGeolocation();
    void closeGeolocation();
    void addAllowCountry(std::string country_code);
    void clearAllowCountry();
    bool isAllow(std::string account_name, std::string peer_ip);
    bool isAllow(std::string country_code);
    uint8_t getCountryCode(char* output, unsigned int output_size,
                           std::string peer_ip);

private:
    void loadData();
    bool isAllowCountryCode(std::string country_code);

    GeoIP* geo_ip_;  // +0x00
    boost::unordered_set<std::string> allow_country_codes_;  // +0x04
};

static_assert(sizeof(RestrictGeolocation) == 0x1cu,
              "RestrictGeolocation must retain the original 28-byte i386 ABI");

#endif  // OLDSRV_GLOBAL_RESTRICT_GEOLOCATION_ABI_H_
