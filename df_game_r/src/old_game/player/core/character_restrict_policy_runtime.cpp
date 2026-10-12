#include "infra/security/policy/service_restrict_manager_abi.h"

#include "infra/log/format/trace.h"

#include <string>

namespace {

void set_create_character_policy_values(ServicePolicy& policy,
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
                RestrictCategory::CreateCharacter,
                out_index,
                parsed);
            parsed = 0;
        }
        policy.values[out_index] = static_cast<short>(parsed);
    }
}

}  // namespace

// IDA_EA: 0x816DDD2
// IDA_NAME: _ZN29CreateCharacterRestrictPolicyC2Ev
// DEMANGLED: CreateCharacterRestrictPolicy::CreateCharacterRestrictPolicy(void)
CreateCharacterRestrictPolicy::CreateCharacterRestrictPolicy()
    : InterfaceServicePolicy(RestrictCategory::CreateCharacter) {
    apply_ = 0;
    policy_.Init();
    for (int index = 0; index <= 2; ++index) {
        policy_.values.push_back(static_cast<short>(index));
    }
}

// IDA_EA: 0x816DE64
// IDA_NAME: _ZN29CreateCharacterRestrictPolicyD2Ev
// DEMANGLED: CreateCharacterRestrictPolicy::~CreateCharacterRestrictPolicy()
CreateCharacterRestrictPolicy::~CreateCharacterRestrictPolicy() {}

// IDA_EA: 0x816DEB2
// IDA_NAME: _ZN29CreateCharacterRestrictPolicy11beginPolicyERKSs
// DEMANGLED: CreateCharacterRestrictPolicy::beginPolicy(std::string const&)
int CreateCharacterRestrictPolicy::beginPolicy(const std::string& value) {
    set_create_character_policy_values(policy_, value);
    return 1;
}

// IDA_EA: 0x816DEDE
// IDA_NAME: _ZN29CreateCharacterRestrictPolicy9endPolicyEv
// DEMANGLED: CreateCharacterRestrictPolicy::endPolicy(void)
void CreateCharacterRestrictPolicy::endPolicy() {
    apply_ = 0;
}

// IDA_EA: 0x816DEEA
// IDA_NAME: _ZN29CreateCharacterRestrictPolicy7isApplyEv
// DEMANGLED: CreateCharacterRestrictPolicy::isApply(void)
int CreateCharacterRestrictPolicy::isApply() {
    return apply_;
}

// IDA_EA: 0x816DEF6
// IDA_NAME: _ZN29CreateCharacterRestrictPolicy7isApplyEi
// DEMANGLED: CreateCharacterRestrictPolicy::isApply(int)
bool CreateCharacterRestrictPolicy::isApply(int index) {
    if (static_cast<unsigned int>(index) <= 2u) {
        return policy_.values[static_cast<unsigned int>(index)] == 1;
    }
    LogManager::logFormat(
        1,
        "localtaiwan/System/ServiceRestrictManager.cpp",
        "virtual bool CreateCharacterRestrictPolicy::isApply(int)",
        0x11C,
        "[Taiwan, Restrict] Invalid AuthMobileRestrictPolicy Parameter. End:%d, Param:%d",
        3,
        index);
    return false;
}

// IDA_EA: 0x816DF6C
// IDA_NAME: _ZN29CreateCharacterRestrictPolicy13printRestrictEv
// DEMANGLED: CreateCharacterRestrictPolicy::printRestrict(void)
int CreateCharacterRestrictPolicy::printRestrict() {
    cMyTrace begin_trace(
        "virtual void CreateCharacterRestrictPolicy::printRestrict()",
        293,
        9,
        true,
        true);
    begin_trace("\tCreate Character Begin");
    for (unsigned int index = 0; index < policy_.values.size(); ++index) {
        cMyTrace value_trace(
            "virtual void CreateCharacterRestrictPolicy::printRestrict()",
            296,
            9,
            true,
            true);
        value_trace("\t- %d : %d", index, policy_.values[index]);
    }
    cMyTrace end_trace(
        "virtual void CreateCharacterRestrictPolicy::printRestrict()",
        300,
        9,
        true,
        true);
    return end_trace("\tCreate Character End");
}

// IDA_EA: 0x816E0A6
// IDA_NAME: _ZN29CreateCharacterRestrictPolicy16getRestrictValueERN6Taiwan21SigLoadRestrictPolicyE
// DEMANGLED: CreateCharacterRestrictPolicy::getRestrictValue(Taiwan::SigLoadRestrictPolicy &)
void CreateCharacterRestrictPolicy::getRestrictValue(
    Taiwan::SigLoadRestrictPolicy& value) {
    (void)value;
}
