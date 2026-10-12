#ifndef OLDSRV_OLD_GAME_USERINFO_CALC_CHARAC_STAT_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_CALC_CHARAC_STAT_RUNTIME_H_

#include "common/manager/play_info_public_abi.h"

namespace WongWork {

// IDA unit: 04_character_select_create/WongWork_CCalcCharacStat.
// These are static helpers: the first machine argument is the target
// _Additioal_info, as confirmed by all three CUser::UseQuestItem call sites.
class CCalcCharacStat {
public:
    // IDA_EA: 0x831E85C
    // IDA_NAME: _ZN8WongWork15CCalcCharacStat15ApplyElixerItemEP15_Additioal_infoii
    // DEMANGLED: WongWork::CCalcCharacStat::ApplyElixerItem(_Additioal_info *,int,int)
    static void ApplyElixerItem(_Additioal_info* info,
                                int ability,
                                int count_delta);

    // IDA_EA: 0x831E9CA
    // IDA_NAME: _ZN8WongWork15CCalcCharacStat14InitializeStatEP15_Additioal_infoi
    // DEMANGLED: WongWork::CCalcCharacStat::InitializeStat(_Additioal_info *,int)
    static void InitializeStat(_Additioal_info* info, int charac_job);

    // IDA_EA: 0x831EC4A
    // IDA_NAME: _ZN8WongWork15CCalcCharacStat8CalcStatEP15_Additioal_infoiiii
    // DEMANGLED: WongWork::CCalcCharacStat::CalcStat(_Additioal_info *,int,int,int,int)
    static void CalcStat(_Additioal_info* info,
                         int charac_job,
                         int first_grow_type,
                         int second_grow_type,
                         int charac_level);

    // IDA_EA: 0x831ED6C
    // IDA_NAME: _ZN8WongWork15CCalcCharacStat13_CalcSwordmanEP15_Additioal_infoii
    // DEMANGLED: WongWork::CCalcCharacStat::_CalcSwordman(_Additioal_info *,int,int)
    static void _CalcSwordman(_Additioal_info* info,
                              int grow_type,
                              int charac_level);

    // IDA_EA: 0x831F282
    // IDA_NAME: _ZN8WongWork15CCalcCharacStat12_CalcFighterEP15_Additioal_infoii
    // DEMANGLED: WongWork::CCalcCharacStat::_CalcFighter(_Additioal_info *,int,int)
    static void _CalcFighter(_Additioal_info* info,
                             int grow_type,
                             int charac_level);

    // IDA_EA: 0x831F75C
    // IDA_NAME: _ZN8WongWork15CCalcCharacStat11_CalcGunnerEP15_Additioal_infoii
    // DEMANGLED: WongWork::CCalcCharacStat::_CalcGunner(_Additioal_info *,int,int)
    static void _CalcGunner(_Additioal_info* info,
                            int grow_type,
                            int charac_level);

    // IDA_EA: 0x831FC3A
    // IDA_NAME: _ZN8WongWork15CCalcCharacStat9_CalcMageEP15_Additioal_infoii
    // DEMANGLED: WongWork::CCalcCharacStat::_CalcMage(_Additioal_info *,int,int)
    static void _CalcMage(_Additioal_info* info,
                          int grow_type,
                          int charac_level);
};

}  // namespace WongWork

#endif  // OLDSRV_OLD_GAME_USERINFO_CALC_CHARAC_STAT_RUNTIME_H_
