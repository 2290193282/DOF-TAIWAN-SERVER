#ifndef OLDSRV_GLOBAL_SERVICE_RESTRICT_MANAGER_ABI_H_
#define OLDSRV_GLOBAL_SERVICE_RESTRICT_MANAGER_ABI_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

class CUser;

class CStreamGuard;

namespace RestrictCategory {

enum Enum {
    NullPolicy = 0,
    AuthMobile = 1,
    CreateCharacter = 2,
    Security = 3,
};

char* enumToString(Enum value);

extern char* enumStringRestrict[];

}  // namespace RestrictCategory

namespace SecurityRestrict {

// IDA_EA: SecurityRestrict::enumToString @0x816D481.
// The old enum is passed as a four-byte value. Its named constants are not
// needed by consumers of this narrow string conversion ABI.
enum Enum : int;

char* enumToString(Enum value);

}  // namespace SecurityRestrict

namespace Taiwan {

// IDA unit: 99_misc_global/Taiwan/SigLoadRestrictPolicy.
// IDA_EA: init @0x815DDA8, set(int,int,char) @0x815DE02,
// set(SigLoadRestrictPolicy&) @0x815DE38.
struct SigLoadRestrictPolicy {
    int32_t code;
    int32_t count;
    char names[0x1194];
    char values[0x64];

    void* init();
    SigLoadRestrictPolicy* set(int code_value, int index, char value);
    void* set(SigLoadRestrictPolicy& source);
};

int internal_stream(CStreamGuard& guard, int code, int src);

}  // namespace Taiwan

static_assert(sizeof(Taiwan::SigLoadRestrictPolicy) == 0x1200u,
              "Taiwan::SigLoadRestrictPolicy IDA ABI size must stay 0x1200");

// IDA unit: 99_misc_global/ServicePolicy.
// IDA_EA: ctor @0x816ED2E, Init @0x816ED70, dtor @0x816ED84.
class ServicePolicy {
public:
    ServicePolicy();
    int Init();
    ~ServicePolicy();

    std::vector<short> values;
};

static_assert(sizeof(ServicePolicy) == 0x0Cu,
              "ServicePolicy IDA ABI size must stay 0x0C");

// IDA unit: 99_misc_global/InterfaceServicePolicy.
// IDA_EA: ctor @0x816ED98, dtor @0x816EDC0/@0x816EDF4,
// getCode @0x816EE12.
class InterfaceServicePolicy {
public:
    explicit InterfaceServicePolicy(RestrictCategory::Enum code);
    virtual ~InterfaceServicePolicy();

    virtual int beginPolicy(const std::string& value) = 0;
    virtual void endPolicy() = 0;
    virtual int isApply() = 0;
    virtual bool isApply(int index) = 0;
    virtual int printRestrict() = 0;
    virtual void getRestrictValue(Taiwan::SigLoadRestrictPolicy& value) = 0;

    int getCode();

protected:
    uint8_t apply_;
    uint8_t reserved_05_[3];
    RestrictCategory::Enum code_;
    ServicePolicy policy_;
};

static_assert(sizeof(InterfaceServicePolicy) == 0x18u,
              "InterfaceServicePolicy IDA ABI size must stay 0x18");

class NullPolicy : public InterfaceServicePolicy {
public:
    NullPolicy();
    virtual ~NullPolicy();

    virtual int beginPolicy(const std::string& value);
    virtual void endPolicy();
    virtual int isApply();
    virtual bool isApply(int index);
    virtual int printRestrict();
    virtual void getRestrictValue(Taiwan::SigLoadRestrictPolicy& value);
};

class AuthMobileRestrictPolicy : public InterfaceServicePolicy {
public:
    AuthMobileRestrictPolicy();
    virtual ~AuthMobileRestrictPolicy();

    virtual int beginPolicy(const std::string& value);
    virtual void endPolicy();
    virtual int isApply();
    virtual bool isApply(int index);
    virtual int printRestrict();
    virtual void getRestrictValue(Taiwan::SigLoadRestrictPolicy& value);
};

class SecurityRestrictPolicy : public InterfaceServicePolicy {
public:
    SecurityRestrictPolicy();
    virtual ~SecurityRestrictPolicy();

    virtual int beginPolicy(const std::string& value);
    virtual void endPolicy();
    virtual int isApply();
    virtual bool isApply(int index);
    virtual int printRestrict();
    virtual void getRestrictValue(Taiwan::SigLoadRestrictPolicy& value);
};

class CreateCharacterRestrictPolicy : public InterfaceServicePolicy {
public:
    CreateCharacterRestrictPolicy();
    virtual ~CreateCharacterRestrictPolicy();

    virtual int beginPolicy(const std::string& value);
    virtual void endPolicy();
    virtual int isApply();
    virtual bool isApply(int index);
    virtual int printRestrict();
    virtual void getRestrictValue(Taiwan::SigLoadRestrictPolicy& value);
};

static_assert(sizeof(NullPolicy) == 0x18u,
              "NullPolicy IDA ABI size must stay 0x18");
static_assert(sizeof(AuthMobileRestrictPolicy) == 0x18u,
              "AuthMobileRestrictPolicy IDA ABI size must stay 0x18");
static_assert(sizeof(SecurityRestrictPolicy) == 0x18u,
              "SecurityRestrictPolicy IDA ABI size must stay 0x18");
static_assert(sizeof(CreateCharacterRestrictPolicy) == 0x18u,
              "CreateCharacterRestrictPolicy IDA ABI size must stay 0x18");

class ServiceRestrictManager {
public:
    ServiceRestrictManager();
    ~ServiceRestrictManager();

    std::string serviceRestrictStringQuery();
    int initialize();
    int beginPolicy(RestrictCategory::Enum category,
                    const std::string& value);
    int endPolicy(RestrictCategory::Enum category);
    int isApply(RestrictCategory::Enum category);
    int isRestricted(CUser* user, RestrictCategory::Enum category,
                     int restrict_id);
    int isRestricted(RestrictCategory::Enum category, int restrict_id);
    void loadPolicy();
    void savePolicy();
    int printPolicy(RestrictCategory::Enum category);
    void updatePolicy(RestrictCategory::Enum category);

private:
    typedef std::map<RestrictCategory::Enum, InterfaceServicePolicy*> PolicyMap;

    PolicyMap policies_;
    uint8_t initialized_;
    uint8_t channel_one_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ServiceRestrictManager) == 0x1Cu,
              "ServiceRestrictManager restored layout must stay 0x1C on i386");
#endif

namespace ARAD {

#ifndef OLDSRV_ARAD_SINGLETON_TEMPLATE_DECLARATION_H_
#define OLDSRV_ARAD_SINGLETON_TEMPLATE_DECLARATION_H_

// This is the one declaration shared by ARAD's many explicit Singleton<T>
// instantiations.  It has no storage; the protected constructor is required
// by the old ARAD data-manager base hierarchy.
template <typename T>
class Singleton {
protected:
    Singleton();

public:
    static T* Get();
};

#endif  // OLDSRV_ARAD_SINGLETON_TEMPLATE_DECLARATION_H_

template <>
ServiceRestrictManager* Singleton<ServiceRestrictManager>::Get();

template <>
Singleton<ServiceRestrictManager>::Singleton();

}  // namespace ARAD

#endif  // OLDSRV_GLOBAL_SERVICE_RESTRICT_MANAGER_ABI_H_
