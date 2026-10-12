#include "old_game/pvp/battlefield/pvp_reference_public_abi.h"

#include "common/manager/global_data.h"

RefPvpGrade* GlobalData::s_ref_pvp_grade = 0;

// IDA data: contiguous dwords at 0x93F4EE0..0x93F4EF4.
// IDA_EA: writes @0x8908816/@0x8908843/@0x8908870/
// @0x890889D/@0x89088CA/@0x89088F7; reads @0x85D9B9E/@0x85D9BC4.
int g_pvp_battle_time_storage[6] __asm__("PVP_NORMAL_BATTLE_TIME") = {
    180000,
    180000,
    600000,
    600000,
    120000,
    120000,
};

__asm__(
    ".globl PVP_DM_BATTLE_TIME\n"
    ".set PVP_DM_BATTLE_TIME, PVP_NORMAL_BATTLE_TIME+8\n"
    ".globl PVP_RELAY_BATTLE_TIME\n"
    ".set PVP_RELAY_BATTLE_TIME, PVP_NORMAL_BATTLE_TIME+16\n"
    ".globl dword_93F4EE4\n"
    ".set dword_93F4EE4, PVP_NORMAL_BATTLE_TIME+4\n"
    ".globl dword_93F4EEC\n"
    ".set dword_93F4EEC, PVP_NORMAL_BATTLE_TIME+12\n"
    ".globl dword_93F4EF4\n"
    ".set dword_93F4EF4, PVP_NORMAL_BATTLE_TIME+20\n");
