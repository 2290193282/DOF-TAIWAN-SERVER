#ifndef OLDSRV_GLOBAL_CHARACTER_SCRIPT_PUBLIC_ABI_H_
#define OLDSRV_GLOBAL_CHARACTER_SCRIPT_PUBLIC_ABI_H_

#include <map>
#include <string>

#include "common/config.h"
#include "common/manager/global_tools.h"

// IDA_EA: 0x88BE60E
// IDA_NAME: _Z19getCharacterJobName17ENUM_CHARACTERJOB
// DEMANGLED: getCharacterJobName(ENUM_CHARACTERJOB)
char* getCharacterJobName(ENUM_CHARACTERJOB job)
    __asm__("_Z19getCharacterJobName17ENUM_CHARACTERJOB");

// IDA_EA: 0x88BE04C
// IDA_NAME: _Z19convertBinaryToTextPKc
// DEMANGLED: convertBinaryToText(char const*)
int convertBinaryToText(const char* source)
    __asm__("_Z19convertBinaryToTextPKc");

// IDA_EA: 0x8A3DF64
// IDA_NAME: _Z11GetRoundIntf
// DEMANGLED: GetRoundInt(float)
int GetRoundInt(float value) __asm__("_Z11GetRoundIntf");

// IDA_EA: 0x8A39866
// IDA_NAME: _Z31getUpgradeWeaponAttackPowerRate17ENUM_CHARACTERJOBjj
// DEMANGLED: getUpgradeWeaponAttackPowerRate(ENUM_CHARACTERJOB,unsigned int,unsigned int)
float getUpgradeWeaponAttackPowerRate(ENUM_CHARACTERJOB job,
                                      unsigned int grade,
                                      unsigned int value_type)
    __asm__("_Z31getUpgradeWeaponAttackPowerRate17ENUM_CHARACTERJOBjj");

// IDA globals written by ImportCharacterScript @0x8A0CE06.
extern std::map<ENUM_CHARACTERJOB, std::string> s_bodyImagePath
    __asm__("_ZL15s_bodyImagePath");
extern float upgradeWeaponAttackPowerRate_[
    oldsrv::config::kCharacterJobCount * 12]
    __asm__("upgradeWeaponAttackPowerRate_");
extern float g_weaponDurabilityDecreaseRate_[
    oldsrv::config::kCharacterJobCount * 6]
    __asm__("g_weaponDurabilityDecreaseRate_");

// IDA_EA: 0x8A0AEB8
// IDA_NAME: _Z20initObjectScriptBasev
// DEMANGLED: initObjectScriptBase(void)
bool initObjectScriptBase() __asm__("_Z20initObjectScriptBasev");

// IDA_EA: 0x8A0AED8
// IDA_NAME: _Z18initObjectCategoryv
// DEMANGLED: initObjectCategory(void)
int initObjectCategory() __asm__("_Z18initObjectCategoryv");

// IDA_EA: 0x8A3CA1B
// IDA_NAME: _Z17getObjectCategorySs
// DEMANGLED: getObjectCategory(std::string)
int getObjectCategory(std::string category)
    __asm__("_Z17getObjectCategorySs");

// IDA_EA: 0x8A3CA81
// IDA_NAME: _Z23getObjectCategoryString13ENUM_CATEGORY
// DEMANGLED: getObjectCategoryString(ENUM_CATEGORY)
std::string getObjectCategoryString(int category)
    __asm__("_Z23getObjectCategoryString13ENUM_CATEGORY");

#endif  // OLDSRV_GLOBAL_CHARACTER_SCRIPT_PUBLIC_ABI_H_
