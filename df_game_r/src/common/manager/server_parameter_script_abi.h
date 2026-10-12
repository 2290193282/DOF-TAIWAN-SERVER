#ifndef OLDSRV_GLOBAL_SERVER_PARAMETER_SCRIPT_ABI_H_
#define OLDSRV_GLOBAL_SERVER_PARAMETER_SCRIPT_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

// IDA unit: 12_pvf_datamanager_scripts/ServerParameterScript.
// The two old std::vector<sItemGenRef> slots at ServerParameterScript+0x3C0
// and +0x400 are also reached by CDataManager::SetItemDropRate @0x83607D8
// through CDataManager+0x68+0x3C0 and +0x68+0x400.
struct sItemGenRef {
    int32_t value_00;
    int32_t value_04;
    int32_t value_08;
    int32_t value_0c;
    int32_t rate;
};

typedef std::vector<sItemGenRef> ItemGenRefVector;

static_assert(sizeof(sItemGenRef) == 0x14u,
              "sItemGenRef IDA ABI size must stay 0x14");
static_assert(offsetof(sItemGenRef, rate) == 0x10u,
              "sItemGenRef rate offset must stay 0x10");

// IDA unit: 99_misc_global/stBurningFatigueParam_t.
// IDA_EA: stBurningFatigueParam_t::stBurningFatigueParam_t @0x8A8CC64,
// stBurningFatigueParam_t::~stBurningFatigueParam_t @0x8A91BB0,
// ServerParameterScript::ServerParameterScript @0x836FCA4 constructs
// std::vector<stBurningFatigueParam_t> at +0x48C, and
// CBurningFatigueEvent::CBurningFatigueEvent @0x8109FA4 stores
// G_CDataManager()+0x68+0x48C for the event runtime readers.
struct stBurningFatigueParam_t {
    stBurningFatigueParam_t();
    ~stBurningFatigueParam_t();

    uint16_t fatigue;
    uint16_t pad_02;
    int32_t bonus_exp_rate;
    uint16_t refill_fatigue;
    uint16_t pad_0a;
    int32_t value_0c;
    float value_10;
    int32_t value_14;
    uint8_t raw_18[4];
};

typedef std::vector<stBurningFatigueParam_t> BurningFatigueParamVector;

static_assert(sizeof(stBurningFatigueParam_t) == 0x1Cu,
              "stBurningFatigueParam_t IDA ABI size must stay 0x1c");
static_assert(offsetof(stBurningFatigueParam_t, bonus_exp_rate) == 0x04u,
              "stBurningFatigueParam_t bonus_exp_rate offset must stay 0x04");
static_assert(offsetof(stBurningFatigueParam_t, refill_fatigue) == 0x08u,
              "stBurningFatigueParam_t refill_fatigue offset must stay 0x08");
static_assert(offsetof(stBurningFatigueParam_t, value_10) == 0x10u,
              "stBurningFatigueParam_t value_10 offset must stay 0x10");
static_assert(offsetof(stBurningFatigueParam_t, raw_18) == 0x18u,
              "stBurningFatigueParam_t raw_18 offset must stay 0x18");


using ::BurningFatigueParamVector;
using ::ItemGenRefVector;
using ::sItemGenRef;
using ::stBurningFatigueParam_t;

class ServerParameterScript;

// IDA_EA evidence:
// CDataManager::CDataManager @0x8355B30 calls ctor at CDataManager+0x68.
// CDataManager::~CDataManager @0x8356B68 calls dtor at CDataManager+0x68.

#endif  // OLDSRV_GLOBAL_SERVER_PARAMETER_SCRIPT_ABI_H_
