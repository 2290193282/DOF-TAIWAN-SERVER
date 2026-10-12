#include "infra/security/policy/service_restrict_manager_abi.h"

#include "core/config/env/environment.h"
#include "common/manager/global_data.h"
#include "old_data/Login/garena/taiwan_garena_auth_data.h"
#include "old_game/player/core/cuser.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <new>
#include <sstream>

namespace {

const char* const kMobileRestrictNames[34] = {
    "NONE",
    "DROP_ITEM",
    "MOVE_MAP",
    "BUY_ITEM",
    "MAIL_SEND",
    "AUCTION",
    "TRADE",
    "UPGRADE_ITEM",
    "PEER",
    "COMPOUND",
    "PRIVATE_STORE",
    "PRIVATE_STORE_BUY",
    "USE_STACK",
    "COMPOUND_EXTREME",
    "AVATAR_DISJOINT",
    "RANDOM_OPTION",
    "REGENERATION_RANDOM_OPTION",
    "BUY_CERASHOP_ITEM",
    "MULTIBOX_LOTTERY",
    "EXPERT_JOB_ITEM",
    "DISJOINT_AVATAR",
    "GUILD_DISPATCH",
    "USER_ACTION",
    "DELETE_CHARACTER",
    "ACCOUNT_CARGO",
    "ACCOUNT_CARGO_2",
    "ACCOUNT_CARGO_3",
    "CHARACTER_RUNTIME",
    "GUILD_CREATE",
    "GUILD_LEVEL_UP",
    "BUY_SKILL",
    "RESERVED_31",
    "CREATE_CHARACTER",
    "LOGIN",
};

bool get_mobile_auth(CUser* user) {
    if (!user) {
        return false;
    }
    return user->getGarenaAuthData()->getMobileAuth();
}

void setPolicyApplyValue(RestrictCategory::Enum category,
                         ServicePolicy& policy,
                         const std::string& value) {
    if (value.empty()) {
        return;
    }

    unsigned int out_index = 1;
    const unsigned int end = static_cast<unsigned int>(policy.values.size());
    for (std::string::const_iterator it = value.begin();
         it != value.end() && out_index < end;
         ++it, ++out_index) {
        unsigned int parsed =
            static_cast<unsigned int>(static_cast<unsigned char>(*it) - '0');
        if (parsed >= 2u) {
            LogManager::logFormat(
                1,
                "localtaiwan/System/ServiceRestrictManager.cpp",
                "void setPolicyApplyValue(RestrictCategory::Enum, ServicePolicy&, const std::string&)",
                0x22,
                "[Taiwan, Restrict] Invalid server value. restrict:%d,%d,%d",
                category,
                out_index,
                parsed);
            parsed = 0;
        }
        policy.values[out_index] = static_cast<short>(parsed);
    }
}

Taiwan::SigLoadRestrictPolicy* get_restrict_payload(
    CStreamGuard& guard) {
    return static_cast<Taiwan::SigLoadRestrictPolicy*>(
        guard.GetInBufferUntyped(sizeof(Taiwan::SigLoadRestrictPolicy)));
}

const char* mobile_restrict_name(unsigned int index) {
    if (index < (sizeof(kMobileRestrictNames) / sizeof(kMobileRestrictNames[0]))) {
        return kMobileRestrictNames[index];
    }
    return "";
}

}  // namespace

namespace RestrictCategory {

char* enumStringRestrict[] = {
    const_cast<char*>("NullPolicy"),
    const_cast<char*>("AuthMobileRestrictPolicy"),
    const_cast<char*>("CreateCharacterRestrictPolicy"),
    const_cast<char*>("SecurityRestrictPolicy"),
};

// IDA_EA: 0x816D454
// IDA_NAME: _ZN16RestrictCategory12enumToStringENS_4EnumE
// DEMANGLED: RestrictCategory::enumToString(RestrictCategory::Enum)
char* enumToString(Enum value) {
    return enumStringRestrict[value];
}

}  // namespace RestrictCategory

namespace SecurityRestrict {

// IDA data: _ZN16SecurityRestrictL18enumStringSecurityE @0x937015C.
// The first three entries resolve to RestrictBegin, DropRate, and RestrictEnd.
// It has internal linkage in the old binary, matching the `L` component of
// its original name.
static char* enumStringSecurity[] = {
    const_cast<char*>("RestrictBegin"),
    const_cast<char*>("DropRate"),
    const_cast<char*>("RestrictEnd"),
};

// IDA_EA: 0x816D481
// IDA_NAME: _ZN16SecurityRestrict12enumToStringENS_4EnumE
// DEMANGLED: SecurityRestrict::enumToString(SecurityRestrict::Enum)
char* enumToString(Enum value) {
    return enumStringSecurity[static_cast<int>(value)];
}

}  // namespace SecurityRestrict

namespace Taiwan {

// IDA_EA: 0x815DDA8
// IDA_NAME: _ZN6Taiwan21SigLoadRestrictPolicy4initEv
// DEMANGLED: Taiwan::SigLoadRestrictPolicy::init(void)
void* SigLoadRestrictPolicy::init() {
    code = 0;
    count = 0;
    std::memset(names, 0, sizeof(names));
    return std::memset(values, 0, sizeof(values));
}

// IDA_EA: 0x815DE02
// IDA_NAME: _ZN6Taiwan21SigLoadRestrictPolicy3setEiic
// DEMANGLED: Taiwan::SigLoadRestrictPolicy::set(int,int,char)
SigLoadRestrictPolicy* SigLoadRestrictPolicy::set(int code_value,
                                                  int index,
                                                  char value) {
    code = code_value;
    values[index] = value;
    ++count;
    return this;
}

// IDA_EA: 0x815DE38
// IDA_NAME: _ZN6Taiwan21SigLoadRestrictPolicy3setERS0_
// DEMANGLED: Taiwan::SigLoadRestrictPolicy::set(Taiwan::SigLoadRestrictPolicy &)
void* SigLoadRestrictPolicy::set(SigLoadRestrictPolicy& source) {
    code = source.code;
    count = source.count;
    std::memcpy(names, source.names, sizeof(names));
    return std::memcpy(values, source.values, sizeof(values));
}

// IDA_EA: 0x8163950
// IDA_NAME: _ZN6Taiwan15internal_streamER12CStreamGuardN18TaiwanInternalPack1TEi
// DEMANGLED: Taiwan::internal_stream(CStreamGuard &,TaiwanInternalPack::T,int)
int internal_stream(CStreamGuard& guard, int code, int src) {
    // IDA 0x8163950 calls CStreamGuard::operator<<(int) three times:
    // outer Taiwan dispatcher signal 0x34e, source uid, then inner code.
    // Narrowing the latter two fields to char desynchronizes DBTask, which
    // unconditionally reads the outer signal and user index as two ints.
    // The following payload must therefore be written at +0x0c, exactly as
    // in df_game_r; otherwise neither policy nor GeoIP replies can reach the
    // internal dispatcher.
    // 0x8163962 is `mov dword ptr [esp+4], 34Eh`; it is *not* decimal
    // 78.  846 is the DBTask/InterDispatcher slot occupied by the original
    // Taiwan aggregate dispatcher.  78 routes to DB_FatigueResetTimer and
    // silently discards the following Taiwan payload.
    (*guard) << 846;
    (*guard) << src;
    (*guard) << code;
    return 1;
}

}  // namespace Taiwan

// IDA_EA: 0x816ED2E
// IDA_NAME: _ZN13ServicePolicyC2Ev
// DEMANGLED: ServicePolicy::ServicePolicy(void)
ServicePolicy::ServicePolicy() : values() {
    values.clear();
}

// IDA_EA: 0x816ED70
// IDA_NAME: _ZN13ServicePolicy4InitEv
// DEMANGLED: ServicePolicy::Init(void)
int ServicePolicy::Init() {
    values.clear();
    return 0;
}

// IDA_EA: 0x816ED84
// IDA_NAME: _ZN13ServicePolicyD2Ev
// DEMANGLED: ServicePolicy::~ServicePolicy()
ServicePolicy::~ServicePolicy() {}

// IDA_EA: 0x816ED98
// IDA_NAME: _ZN22InterfaceServicePolicyC2EN16RestrictCategory4EnumE
// DEMANGLED: InterfaceServicePolicy::InterfaceServicePolicy(RestrictCategory::Enum)
InterfaceServicePolicy::InterfaceServicePolicy(RestrictCategory::Enum code)
    : apply_(0),
      reserved_05_(),
      code_(code),
      policy_() {}

// IDA_EA: 0x816EDC0
// IDA_NAME: _ZN22InterfaceServicePolicyD2Ev
// DEMANGLED: InterfaceServicePolicy::~InterfaceServicePolicy()
InterfaceServicePolicy::~InterfaceServicePolicy() {}

// IDA_EA: 0x816EE12
// IDA_NAME: _ZN22InterfaceServicePolicy7getCodeEv
// DEMANGLED: InterfaceServicePolicy::getCode(void)
int InterfaceServicePolicy::getCode() {
    return code_;
}

// IDA_EA: 0x816D5AA
// IDA_NAME: _ZN10NullPolicyC2Ev
// DEMANGLED: NullPolicy::NullPolicy(void)
NullPolicy::NullPolicy()
    : InterfaceServicePolicy(RestrictCategory::NullPolicy) {
    apply_ = 0;
    policy_.Init();
}

// IDA_EA: 0x816D608
// IDA_NAME: _ZN10NullPolicyD2Ev
// DEMANGLED: NullPolicy::~NullPolicy()
NullPolicy::~NullPolicy() {}

// IDA_EA: 0x816D656
// IDA_NAME: _ZN10NullPolicy11beginPolicyERKSs
// DEMANGLED: NullPolicy::beginPolicy(std::string const&)
int NullPolicy::beginPolicy(const std::string& value) {
    (void)value;
    return 1;
}

// IDA_EA: 0x816D660
// IDA_NAME: _ZN10NullPolicy9endPolicyEv
// DEMANGLED: NullPolicy::endPolicy(void)
void NullPolicy::endPolicy() {}

// IDA_EA: 0x816D666
// IDA_NAME: _ZN10NullPolicy7isApplyEv
// DEMANGLED: NullPolicy::isApply(void)
int NullPolicy::isApply() {
    return 1;
}

// IDA_EA: 0x816D670
// IDA_NAME: _ZN10NullPolicy7isApplyEi
// DEMANGLED: NullPolicy::isApply(int)
bool NullPolicy::isApply(int index) {
    (void)index;
    return true;
}

// IDA_EA: 0x816D67A
// IDA_NAME: _ZN10NullPolicy13printRestrictEv
// DEMANGLED: NullPolicy::printRestrict(void)
int NullPolicy::printRestrict() {
    cMyTrace trace("virtual void NullPolicy::printRestrict()",
                           82,
                           9,
                           true,
                           true);
    return trace("\tDummy Restrict");
}

// IDA_EA: 0x816D6C8
// IDA_NAME: _ZN10NullPolicy16getRestrictValueERN6Taiwan21SigLoadRestrictPolicyE
// DEMANGLED: NullPolicy::getRestrictValue(Taiwan::SigLoadRestrictPolicy &)
void NullPolicy::getRestrictValue(Taiwan::SigLoadRestrictPolicy& value) {
    (void)value;
}

// IDA_EA: 0x816D6CE
// IDA_NAME: _ZN24AuthMobileRestrictPolicyC2Ev
// DEMANGLED: AuthMobileRestrictPolicy::AuthMobileRestrictPolicy(void)
AuthMobileRestrictPolicy::AuthMobileRestrictPolicy()
    : InterfaceServicePolicy(RestrictCategory::AuthMobile) {
    apply_ = 0;
    policy_.Init();
    for (int index = 0; index <= 33; ++index) {
        policy_.values.push_back(1);
    }
}

// IDA_EA: 0x816D75E
// IDA_NAME: _ZN24AuthMobileRestrictPolicyD2Ev
// IDA_EA: 0x816D78E
// IDA_NAME: _ZN24AuthMobileRestrictPolicyD0Ev
// DEMANGLED: AuthMobileRestrictPolicy::~AuthMobileRestrictPolicy()
AuthMobileRestrictPolicy::~AuthMobileRestrictPolicy() {}

// IDA_EA: 0x816D7AC
// IDA_NAME: _ZN24AuthMobileRestrictPolicy11beginPolicyERKSs
// DEMANGLED: AuthMobileRestrictPolicy::beginPolicy(std::string const&)
int AuthMobileRestrictPolicy::beginPolicy(const std::string& value) {
    setPolicyApplyValue(code_, policy_, value);
    return 1;
}

// IDA_EA: 0x816D7D8
// IDA_NAME: _ZN24AuthMobileRestrictPolicy9endPolicyEv
// DEMANGLED: AuthMobileRestrictPolicy::endPolicy(void)
void AuthMobileRestrictPolicy::endPolicy() {
    apply_ = 0;
}

// IDA_EA: 0x816D7E4
// IDA_NAME: _ZN24AuthMobileRestrictPolicy7isApplyEv
// DEMANGLED: AuthMobileRestrictPolicy::isApply(void)
int AuthMobileRestrictPolicy::isApply() {
    return apply_;
}

// IDA_EA: 0x816D7F0
// IDA_NAME: _ZN24AuthMobileRestrictPolicy7isApplyEi
// DEMANGLED: AuthMobileRestrictPolicy::isApply(int)
bool AuthMobileRestrictPolicy::isApply(int index) {
    if (static_cast<unsigned int>(index) < 0x22u) {
        return policy_.values[static_cast<unsigned int>(index)] == 1;
    }
    LogManager::logFormat(
        1,
        "localtaiwan/System/ServiceRestrictManager.cpp",
        "virtual bool AuthMobileRestrictPolicy::isApply(int)",
        0x84,
        "[Taiwan, Restrict] Invalid AuthMobileRestrictPolicy Parameter. End:%d, Param:%d",
        0x22,
        index);
    return false;
}

// IDA_EA: 0x816D866
// IDA_NAME: _ZN24AuthMobileRestrictPolicy13printRestrictEv
// DEMANGLED: AuthMobileRestrictPolicy::printRestrict(void)
int AuthMobileRestrictPolicy::printRestrict() {
    cMyTrace begin_trace(
        "virtual void AuthMobileRestrictPolicy::printRestrict()",
        141,
        9,
        true,
        true);
    begin_trace("\tMobile Auth Begin");
    for (unsigned int index = 0; index < policy_.values.size(); ++index) {
        cMyTrace value_trace(
            "virtual void AuthMobileRestrictPolicy::printRestrict()",
            144,
            9,
            true,
            true);
        value_trace("\t- %s : %d",
                    mobile_restrict_name(index),
                    policy_.values[index]);
    }
    cMyTrace end_trace(
        "virtual void AuthMobileRestrictPolicy::printRestrict()",
        148,
        9,
        true,
        true);
    return end_trace("\tMobile Auth End");
}

// IDA_EA: 0x816D9A0
// IDA_NAME: _ZN24AuthMobileRestrictPolicy16getRestrictValueERN6Taiwan21SigLoadRestrictPolicyE
// DEMANGLED: AuthMobileRestrictPolicy::getRestrictValue(Taiwan::SigLoadRestrictPolicy &)
void AuthMobileRestrictPolicy::getRestrictValue(
    Taiwan::SigLoadRestrictPolicy& value) {
    value.code = getCode();
    value.count = 0;
    for (unsigned int index = 1; index < policy_.values.size(); ++index) {
        std::strncpy(value.names + 45 * index,
                     mobile_restrict_name(index),
                     0x2D);
        value.values[index] = static_cast<char>(policy_.values[index]);
        ++value.count;
    }
}

// IDA_EA: 0x816DA50
// IDA_NAME: _ZN22SecurityRestrictPolicyC2Ev
// DEMANGLED: SecurityRestrictPolicy::SecurityRestrictPolicy(void)
SecurityRestrictPolicy::SecurityRestrictPolicy()
    : InterfaceServicePolicy(RestrictCategory::Security) {
    apply_ = 0;
    policy_.Init();
    for (int index = 0; index <= 1; ++index) {
        policy_.values.push_back(1);
    }
}

// IDA_EA: 0x816DAE0
// IDA_NAME: _ZN22SecurityRestrictPolicyD2Ev
// IDA_EA: 0x816DB10
// IDA_NAME: _ZN22SecurityRestrictPolicyD0Ev
// DEMANGLED: SecurityRestrictPolicy::~SecurityRestrictPolicy()
SecurityRestrictPolicy::~SecurityRestrictPolicy() {}

// IDA_EA: 0x816DB2E
// IDA_NAME: _ZN22SecurityRestrictPolicy11beginPolicyERKSs
// DEMANGLED: SecurityRestrictPolicy::beginPolicy(std::string const&)
int SecurityRestrictPolicy::beginPolicy(const std::string& value) {
    setPolicyApplyValue(code_, policy_, value);
    return 1;
}

// IDA_EA: 0x816DB5A
// IDA_NAME: _ZN22SecurityRestrictPolicy9endPolicyEv
// DEMANGLED: SecurityRestrictPolicy::endPolicy(void)
void SecurityRestrictPolicy::endPolicy() {
    apply_ = 0;
}

// IDA_EA: 0x816DB66
// IDA_NAME: _ZN22SecurityRestrictPolicy7isApplyEv
// DEMANGLED: SecurityRestrictPolicy::isApply(void)
int SecurityRestrictPolicy::isApply() {
    return apply_;
}

// IDA_EA: 0x816DB72
// IDA_NAME: _ZN22SecurityRestrictPolicy7isApplyEi
// DEMANGLED: SecurityRestrictPolicy::isApply(int)
bool SecurityRestrictPolicy::isApply(int index) {
    if (static_cast<unsigned int>(index) < 2u) {
        return policy_.values[static_cast<unsigned int>(index)] == 1;
    }
    LogManager::logFormat(
        1,
        "localtaiwan/System/ServiceRestrictManager.cpp",
        "virtual bool SecurityRestrictPolicy::isApply(int)",
        0xCD,
        "[Taiwan, Restrict] Invalid SecurityRestrictPolicy Parameter. End:%d, Param:%d",
        2,
        index);
    return false;
}

// IDA_EA: 0x816DBE8
// IDA_NAME: _ZN22SecurityRestrictPolicy13printRestrictEv
// DEMANGLED: SecurityRestrictPolicy::printRestrict(void)
int SecurityRestrictPolicy::printRestrict() {
    cMyTrace begin_trace(
        "virtual void SecurityRestrictPolicy::printRestrict()",
        215,
        9,
        true,
        true);
    begin_trace("\tSecurity Restrict Begin");
    for (unsigned int index = 0; index < policy_.values.size(); ++index) {
        cMyTrace value_trace(
            "virtual void SecurityRestrictPolicy::printRestrict()",
            218,
            9,
            true,
            true);
        value_trace("\t- %s : %d",
                    SecurityRestrict::enumToString(
                        static_cast<SecurityRestrict::Enum>(index)),
                    policy_.values[index]);
    }
    cMyTrace end_trace(
        "virtual void SecurityRestrictPolicy::printRestrict()",
        222,
        9,
        true,
        true);
    return end_trace("\tSecurity Restrict End");
}

// IDA_EA: 0x816DD22
// IDA_NAME: _ZN22SecurityRestrictPolicy16getRestrictValueERN6Taiwan21SigLoadRestrictPolicyE
// DEMANGLED: SecurityRestrictPolicy::getRestrictValue(Taiwan::SigLoadRestrictPolicy &)
void SecurityRestrictPolicy::getRestrictValue(
    Taiwan::SigLoadRestrictPolicy& value) {
    value.code = getCode();
    value.count = 0;
    for (unsigned int index = 1; index < policy_.values.size(); ++index) {
        std::strncpy(value.names + 45 * index,
                     SecurityRestrict::enumToString(
                         static_cast<SecurityRestrict::Enum>(index)),
                     0x2D);
        value.values[index] = static_cast<char>(policy_.values[index]);
        ++value.count;
    }
}

// IDA_EA: 0x816E0AC
// IDA_NAME: _ZN22ServiceRestrictManager26serviceRestrictStringQueryEv
// DEMANGLED: ServiceRestrictManager::serviceRestrictStringQuery(void)
std::string ServiceRestrictManager::serviceRestrictStringQuery() {
    std::ostringstream query;
    for (int index = 0; index <= 33; ++index) {
        query << "(," << 1 << ",)" << index << ",)"
              << mobile_restrict_name(static_cast<unsigned int>(index))
              << ")";
    }
    for (int index = 0; index <= 2; ++index) {
        query << "(," << 2 << ",)" << index << ",)" << index << ")";
    }
    return query.str();
}

// IDA_EA: 0x816E272
// IDA_NAME: _ZN22ServiceRestrictManagerC2Ev
// DEMANGLED: ServiceRestrictManager::ServiceRestrictManager(void)
ServiceRestrictManager::ServiceRestrictManager()
    : policies_(),
      initialized_(0),
      channel_one_(0) {}

// IDA_EA: 0x816E29E
// IDA_NAME: _ZN22ServiceRestrictManagerD2Ev
// DEMANGLED: ServiceRestrictManager::~ServiceRestrictManager()
ServiceRestrictManager::~ServiceRestrictManager() {
    for (PolicyMap::iterator it = policies_.begin(); it != policies_.end();
         ++it) {
        delete it->second;
    }
    policies_.clear();
}

// IDA_EA: 0x816E2B2
// IDA_NAME: _ZN22ServiceRestrictManager10initializeEv
// DEMANGLED: ServiceRestrictManager::initialize(void)
int ServiceRestrictManager::initialize() {
    if (initialized_) {
        return 1;
    }
    if (G_CEnvironment() &&
        G_CEnvironment()->get_channel_no() == 1) {
        channel_one_ = 1;
    }

    InterfaceServicePolicy* policy = new (std::nothrow) NullPolicy();
    if (!policy) {
        LogManager::logFormat(
            1,
            "localtaiwan/System/ServiceRestrictManager.cpp",
            "bool ServiceRestrictManager::initialize()",
            0x169,
            "[Taiwan, Restrict] Fail create NullPolicy.");
        return 0;
    }
    policies_[static_cast<RestrictCategory::Enum>(policy->getCode())] =
        policy;

    policy = new (std::nothrow) AuthMobileRestrictPolicy();
    if (!policy) {
        LogManager::logFormat(
            1,
            "localtaiwan/System/ServiceRestrictManager.cpp",
            "bool ServiceRestrictManager::initialize()",
            0x171,
            "[Taiwan, Restrict] Fail create AuthMobileRestrictPolicy.");
        return 0;
    }
    policies_[static_cast<RestrictCategory::Enum>(policy->getCode())] =
        policy;

    policy = new (std::nothrow) CreateCharacterRestrictPolicy();
    if (!policy) {
        LogManager::logFormat(
            1,
            "localtaiwan/System/ServiceRestrictManager.cpp",
            "bool ServiceRestrictManager::initialize()",
            0x17A,
            "[Taiwan, Restrict] Fail create CreateCharacterRestrictPolicy.");
        return 0;
    }
    policies_[static_cast<RestrictCategory::Enum>(policy->getCode())] =
        policy;

    policy = new (std::nothrow) SecurityRestrictPolicy();
    if (!policy) {
        LogManager::logFormat(
            1,
            "localtaiwan/System/ServiceRestrictManager.cpp",
            "bool ServiceRestrictManager::initialize()",
            0x183,
            "[Taiwan, Restrict] Fail create SecurityRestrictPolicy.");
        return 0;
    }
    policies_[static_cast<RestrictCategory::Enum>(policy->getCode())] =
        policy;
    initialized_ = 1;
    return 1;
}

// IDA_EA: 0x816E54A
// IDA_NAME: _ZN22ServiceRestrictManager11beginPolicyEN16RestrictCategory4EnumERKSs
// DEMANGLED: ServiceRestrictManager::beginPolicy(RestrictCategory::Enum,std::string const&)
int ServiceRestrictManager::beginPolicy(RestrictCategory::Enum category,
                                        const std::string& value) {
    PolicyMap::iterator it = policies_.find(category);
    if (it != policies_.end() && it->second) {
        it->second->beginPolicy(value);
    }
    printPolicy(category);
    updatePolicy(category);
    return 1;
}

// IDA_EA: 0x816E5DE
// IDA_NAME: _ZN22ServiceRestrictManager9endPolicyEN16RestrictCategory4EnumE
// DEMANGLED: ServiceRestrictManager::endPolicy(RestrictCategory::Enum)
int ServiceRestrictManager::endPolicy(RestrictCategory::Enum category) {
    PolicyMap::iterator it = policies_.find(category);
    if (it != policies_.end() && it->second) {
        it->second->endPolicy();
        return 1;
    }
    return 0;
}

// IDA_EA: 0x816E648
// IDA_NAME: _ZN22ServiceRestrictManager7isApplyEN16RestrictCategory4EnumE
// DEMANGLED: ServiceRestrictManager::isApply(RestrictCategory::Enum)
int ServiceRestrictManager::isApply(RestrictCategory::Enum category) {
    PolicyMap::iterator it = policies_.find(category);
    if (it == policies_.end() || !it->second) {
        return 1;
    }
    return it->second->isApply();
}

// IDA_EA: 0x816E6B8
// IDA_NAME: _ZN22ServiceRestrictManager12isRestrictedEP5CUserN16RestrictCategory4EnumEi
// DEMANGLED: ServiceRestrictManager::isRestricted(CUser *,RestrictCategory::Enum,int)
int ServiceRestrictManager::isRestricted(CUser* user,
                                         RestrictCategory::Enum category,
                                         int restrict_id) {
    if (!user) {
        return 0;
    }
    if (get_mobile_auth(user)) {
        return 0;
    }
    PolicyMap::iterator it = policies_.find(category);
    if (it == policies_.end() || !it->second) {
        return 1;
    }
    return it->second->isApply(restrict_id);
}

// IDA_EA: 0x816E75E
// IDA_NAME: _ZN22ServiceRestrictManager12isRestrictedEN16RestrictCategory4EnumEi
// DEMANGLED: ServiceRestrictManager::isRestricted(RestrictCategory::Enum,int)
int ServiceRestrictManager::isRestricted(RestrictCategory::Enum category,
                                         int restrict_id) {
    PolicyMap::iterator it = policies_.find(category);
    if (it == policies_.end() || !it->second) {
        return 1;
    }
    return it->second->isApply(restrict_id);
}

// IDA_EA: 0x816E7D6
// IDA_NAME: _ZN22ServiceRestrictManager10loadPolicyEv
// DEMANGLED: ServiceRestrictManager::loadPolicy(void)
void ServiceRestrictManager::loadPolicy() {
    Stream* stream =
        GlobalData::stream_pool()->Acquire(
            "localtaiwan/System/ServiceRestrictManager.cpp",
            453);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 1, -1);
    Taiwan::SigLoadRestrictPolicy* payload = get_restrict_payload(guard);
    if (payload) {
        payload->init();
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q,
                                            guard);
    } else {
        LogManager::logFormat(
            1,
            "localtaiwan/System/ServiceRestrictManager.cpp",
            "void ServiceRestrictManager::loadPolicy()",
            0x1CC,
            "[Taiwan, Restrict] SigLoadRestrictPolicy is null.");
    }
}

// IDA_EA: 0x816E8CE
// IDA_NAME: _ZN22ServiceRestrictManager10savePolicyEv
// DEMANGLED: ServiceRestrictManager::savePolicy(void)
void ServiceRestrictManager::savePolicy() {
    if (channel_one_ != 1) {
        return;
    }
    for (PolicyMap::iterator it = policies_.begin(); it != policies_.end();
         ++it) {
        Stream* stream =
            GlobalData::stream_pool()->Acquire(
                "localtaiwan/System/ServiceRestrictManager.cpp",
                478);
        CStreamGuard guard(stream, true);
        Taiwan::internal_stream(guard, 2, -1);
        Taiwan::SigLoadRestrictPolicy* payload = get_restrict_payload(guard);
        if (payload) {
            payload->init();
            if (it->second) {
                it->second->getRestrictValue(*payload);
            }
            GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q,
                                                guard);
        } else {
            cMyTrace trace(
                "void ServiceRestrictManager::savePolicy()",
                483,
                5);
            trace("[Taiwan, Restrict] SigSaveRestrictPolicy is null.");
            break;
        }
    }
}

// IDA_EA: 0x816EA6A
// IDA_NAME: _ZN22ServiceRestrictManager11printPolicyEN16RestrictCategory4EnumE
// DEMANGLED: ServiceRestrictManager::printPolicy(RestrictCategory::Enum)
int ServiceRestrictManager::printPolicy(RestrictCategory::Enum category) {
    cMyTrace trace(
        "void ServiceRestrictManager::printPolicy(RestrictCategory::Enum)",
        495,
        9,
        true,
        true);
    trace("[!] Service Restrict List ... ");
    PolicyMap::iterator it = policies_.find(category);
    if (it != policies_.end() && it->second) {
        return it->second->printRestrict();
    }
    cMyTrace end_trace(
        "void ServiceRestrictManager::printPolicy(RestrictCategory::Enum)",
        499,
        9,
        true,
        true);
    return end_trace("[!] Service Restrict List ... End");
}

// IDA_EA: 0x816EB62
// IDA_NAME: _ZN22ServiceRestrictManager12updatePolicyEN16RestrictCategory4EnumE
// DEMANGLED: ServiceRestrictManager::updatePolicy(RestrictCategory::Enum)
void ServiceRestrictManager::updatePolicy(RestrictCategory::Enum category) {
    PolicyMap::iterator it = policies_.find(category);
    if (it == policies_.end() || !it->second) {
        return;
    }
    Stream* stream =
        GlobalData::stream_pool()->Acquire(
            "localtaiwan/System/ServiceRestrictManager.cpp",
            509);
    CStreamGuard guard(stream, true);
    Taiwan::internal_stream(guard, 2, -1);
    Taiwan::SigLoadRestrictPolicy* payload = get_restrict_payload(guard);
    if (payload) {
        payload->init();
        it->second->getRestrictValue(*payload);
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q,
                                            guard);
    } else {
        cMyTrace trace(
            "void ServiceRestrictManager::updatePolicy(RestrictCategory::Enum)",
            514,
            5);
        trace("[Taiwan, Restrict] SigSaveRestrictPolicy is null.");
    }
}

namespace ARAD {

// IDA_EA: 0x81625E6
// IDA_NAME: _ZN4ARAD9SingletonI22ServiceRestrictManagerE3GetEv
// DEMANGLED: ARAD::Singleton<ServiceRestrictManager>::Get(void)
template <>
ServiceRestrictManager* Singleton<ServiceRestrictManager>::Get() {
    static ServiceRestrictManager obj;
    return &obj;
}

// IDA_EA: 0x816EF26
// IDA_NAME: _ZN4ARAD9SingletonI22ServiceRestrictManagerEC2Ev
// DEMANGLED: ARAD::Singleton<ServiceRestrictManager>::Singleton(void)
template <>
Singleton<ServiceRestrictManager>::Singleton() {}

}  // namespace ARAD
