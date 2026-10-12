#include "old_game/player/core/calc_charac_stat_runtime.h"

#include "common/config.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/character_growth_tables.h"

namespace WongWork {

// IDA_EA: 0x831E85C
// IDA_NAME: _ZN8WongWork15CCalcCharacStat15ApplyElixerItemEP15_Additioal_infoii
// DEMANGLED: WongWork::CCalcCharacStat::ApplyElixerItem(_Additioal_info *,int,int)
void CCalcCharacStat::ApplyElixerItem(_Additioal_info* info,
                                      int ability,
                                      int count_delta) {
    const uint32_t count = static_cast<uint32_t>(count_delta);

    switch (ability) {
    case 2:
        info->max_hp += 250u * count;
        break;
    case 3:
        info->max_mp += 250u * count;
        break;
    case 4:
        info->physical_attack = static_cast<uint16_t>(
            info->physical_attack + static_cast<uint16_t>(50u * count));
        break;
    case 5:
        info->physical_defense = static_cast<uint16_t>(
            info->physical_defense + static_cast<uint16_t>(50u * count));
        break;
    case 6:
        info->magical_attack = static_cast<uint16_t>(
            info->magical_attack + static_cast<uint16_t>(50u * count));
        break;
    case 7:
        info->magical_defense = static_cast<uint16_t>(
            info->magical_defense + static_cast<uint16_t>(50u * count));
        break;
    case 8:
        info->move_speed += 10u * count;
        break;
    case 9:
        for (int index = 0; index <= 3; ++index) {
            info->element_resist[index] = static_cast<uint16_t>(
                info->element_resist[index] +
                static_cast<uint16_t>(10u * count));
        }
        break;
    default:
        break;
    }
}

// IDA_EA: 0x831E9CA
// IDA_NAME: _ZN8WongWork15CCalcCharacStat14InitializeStatEP15_Additioal_infoi
// DEMANGLED: WongWork::CCalcCharacStat::InitializeStat(_Additioal_info *,int)
void CCalcCharacStat::InitializeStat(_Additioal_info* info, int charac_job) {
    info->clear();

    if (charac_job == oldsrv::config::kCharacterJobFighter) {
        info->max_hp = 1800u;
        info->max_mp = 1500u;
        info->physical_attack = 60u;
        info->physical_defense = 60u;
        info->magical_attack = 50u;
        info->magical_defense = 50u;
        info->element_resist[2] = static_cast<uint16_t>(-200);
        info->element_resist[3] = 200u;
        info->inventory_weight = 300000;
        info->mp_regen_speed = 130;
        info->move_speed = 9100u;
        info->attack_speed = 9500u;
        info->cast_speed = 10000u;
        info->hit_recovery = 6000;
        info->jump_power = 4700;
        info->character_weight = 500000;
    } else if (charac_job > oldsrv::config::kCharacterJobFighter) {
        if (charac_job == oldsrv::config::kCharacterJobGunner) {
            info->max_hp = 1250u;
            info->max_mp = 1800u;
            info->physical_attack = 40u;
            info->physical_defense = 40u;
            info->magical_attack = 70u;
            info->magical_defense = 60u;
            info->inventory_weight = 330000;
            info->mp_regen_speed = 175;
            info->move_speed = 8200u;
            info->attack_speed = 9500u;
            info->cast_speed = 8000u;
            info->hit_recovery = 6000;
            info->jump_power = 4900;
            info->character_weight = 600000;
        } else if (charac_job == oldsrv::config::kCharacterJobMage) {
            info->max_hp = 1000u;
            info->max_mp = 2000u;
            info->physical_attack = 30u;
            info->physical_defense = 30u;
            info->magical_attack = 80u;
            info->magical_defense = 70u;
            info->inventory_weight = 280000;
            info->mp_regen_speed = 200;
            info->move_speed = 8000u;
            info->attack_speed = 10000u;
            info->cast_speed = 10000u;
            info->hit_recovery = 5000;
            info->jump_power = 3500;
            info->character_weight = 400000;
        }
    } else if (charac_job == oldsrv::config::kCharacterJobSwordman) {
        info->max_hp = 1500u;
        info->max_mp = 1500u;
        info->physical_attack = 50u;
        info->physical_defense = 50u;
        info->magical_attack = 60u;
        info->magical_defense = 50u;
        info->element_resist[2] = 200u;
        info->element_resist[3] = static_cast<uint16_t>(-200);
        info->inventory_weight = 400000;
        info->mp_regen_speed = 150;
        info->move_speed = 8500u;
        info->attack_speed = 8500u;
        info->cast_speed = 7000u;
        info->hit_recovery = 6000;
        info->jump_power = 4300;
        info->character_weight = 680000;
    }
}

// IDA_EA: 0x831EC4A
// IDA_NAME: _ZN8WongWork15CCalcCharacStat8CalcStatEP15_Additioal_infoiiii
// DEMANGLED: WongWork::CCalcCharacStat::CalcStat(_Additioal_info *,int,int,int,int)
void CCalcCharacStat::CalcStat(_Additioal_info* info,
                               int charac_job,
                               int first_grow_type,
                               int second_grow_type,
                               int charac_level) {
    info->clear();

    CCharacList* const base_character_list =
        G_CDataManager()->get_character_list();
    info->add(base_character_list->characters_[charac_job].base_status_[0]);

    CCharacList* const first_stage_character_list =
        G_CDataManager()->get_character_list();
    const _Additioal_info* const first_stage_stat =
        first_stage_character_list->characters_[charac_job]
            .get_growtype_stat(0, 0);

    CCharacList* const second_stage_character_list =
        G_CDataManager()->get_character_list();
    const _Additioal_info* const second_stage_stat =
        second_stage_character_list->characters_[charac_job]
            .get_growtype_stat(first_grow_type, 0);

    CCharacList* const third_stage_character_list =
        G_CDataManager()->get_character_list();
    const _Additioal_info* const third_stage_stat =
        third_stage_character_list->characters_[charac_job]
            .get_growtype_stat(first_grow_type, second_grow_type);

    for (int level = 1; level < charac_level; ++level) {
        if (level > 14) {
            if (level > 49) {
                info->add(*third_stage_stat);
            } else {
                info->add(*second_stage_stat);
            }
        } else {
            info->add(*first_stage_stat);
        }
    }
}

// IDA_EA: 0x831ED6C
// IDA_NAME: _ZN8WongWork15CCalcCharacStat13_CalcSwordmanEP15_Additioal_infoii
// DEMANGLED: WongWork::CCalcCharacStat::_CalcSwordman(_Additioal_info *,int,int)
void CCalcCharacStat::_CalcSwordman(_Additioal_info* info,
                                    int grow_type,
                                    int charac_level) {
    switch (static_cast<unsigned int>(grow_type)) {
    case 0u:
        info->max_hp = 525 * (charac_level - 1) + 1500;
        info->max_mp = 175 * (charac_level - 1) + 1500;
        info->physical_attack = static_cast<uint16_t>(
            30 * (charac_level - 1) + 50);
        info->physical_defense = static_cast<uint16_t>(
            30 * (charac_level - 1) + 50);
        info->magical_attack = static_cast<uint16_t>(
            35 * (charac_level - 1) + 60);
        info->magical_defense = static_cast<uint16_t>(
            25 * (charac_level - 1) + 50);
        info->inventory_weight = 3000 * (charac_level - 1) + 400000;
        info->hit_recovery = static_cast<int16_t>(
            20 * (charac_level - 1) + 6000);
        break;
    case 1u:
        info->max_hp = 400 * charac_level + 3225;
        info->max_mp = 300 * charac_level - 925;
        info->physical_attack = static_cast<uint16_t>(35 * charac_level - 70);
        info->physical_defense = static_cast<uint16_t>(30 * charac_level + 20);
        info->magical_attack = static_cast<uint16_t>(30 * charac_level + 115);
        info->magical_defense = static_cast<uint16_t>(25 * (charac_level + 1));
        info->inventory_weight = 3000 * (charac_level - 1) + 400000;
        info->attack_speed = static_cast<uint16_t>(10 * charac_level + 8320);
        info->hit_recovery = static_cast<int16_t>(30 * charac_level + 5800);
        break;
    case 2u:
        info->max_hp = 350 * charac_level + 4125;
        info->max_mp = 350 * charac_level - 1825;
        info->physical_attack = static_cast<uint16_t>(20 * charac_level + 200);
        info->physical_defense = static_cast<uint16_t>(20 * charac_level + 200);
        info->magical_attack = static_cast<uint16_t>(40 * charac_level - 65);
        info->magical_defense = static_cast<uint16_t>(40 * charac_level - 245);
        info->element_resist[2] = static_cast<uint16_t>(
            8 * (charac_level + 7));
        info->inventory_weight = 3000 * (charac_level - 1) + 400000;
        info->mp_regen_speed = static_cast<int16_t>(20 * charac_level - 210);
        info->cast_speed = static_cast<uint16_t>(20 * charac_level + 6640);
        info->hit_recovery = static_cast<int16_t>(10 * charac_level + 6160);
        break;
    case 3u:
        info->max_hp = 550 * charac_level + 525;
        info->max_mp = 150 * charac_level + 1775;
        info->physical_attack = static_cast<uint16_t>(40 * charac_level - 160);
        info->physical_defense = static_cast<uint16_t>(45 * charac_level - 250);
        info->magical_attack = static_cast<uint16_t>(15 * charac_level + 385);
        info->magical_defense = static_cast<uint16_t>(20 * charac_level + 115);
        info->inventory_weight = 3000 * (charac_level - 1) + 400000;
        info->move_speed = static_cast<uint32_t>(10 * charac_level + 8320);
        info->hit_recovery = static_cast<int16_t>(40 * charac_level + 5620);
        break;
    case 4u:
        info->max_hp = 525 * charac_level + 975;
        info->max_mp = 175 * charac_level + 1325;
        info->physical_attack = static_cast<uint16_t>(30 * charac_level + 20);
        info->physical_defense = static_cast<uint16_t>(30 * charac_level + 20);
        info->magical_attack = static_cast<uint16_t>(30 * charac_level + 115);
        info->magical_defense = static_cast<uint16_t>(30 * charac_level - 65);
        info->inventory_weight = 3000 * (charac_level - 1) + 400000;
        info->mp_regen_speed = static_cast<int16_t>(5 * charac_level + 60);
        info->cast_speed = static_cast<uint16_t>(10 * charac_level + 6820);
        info->hit_recovery = static_cast<int16_t>(20 * charac_level + 5640);
        break;
    default:
        break;
    }
}

// IDA_EA: 0x831F282
// IDA_NAME: _ZN8WongWork15CCalcCharacStat12_CalcFighterEP15_Additioal_infoii
// DEMANGLED: WongWork::CCalcCharacStat::_CalcFighter(_Additioal_info *,int,int)
void CCalcCharacStat::_CalcFighter(_Additioal_info* info,
                                   int grow_type,
                                   int charac_level) {
    switch (static_cast<unsigned int>(grow_type)) {
    case 0u:
        info->max_hp = 450 * (charac_level - 1) + 1800;
        info->max_mp = 250 * (charac_level - 1) + 1500;
        info->physical_attack = static_cast<uint16_t>(
            32 * (charac_level - 1) + 60);
        info->physical_defense = static_cast<uint16_t>(
            35 * (charac_level - 1) + 60);
        info->magical_attack = static_cast<uint16_t>(
            30 * (charac_level - 1) + 50);
        info->magical_defense = static_cast<uint16_t>(
            23 * (charac_level - 1) + 50);
        info->inventory_weight = 2500 * (charac_level - 1) + 300000;
        info->hit_recovery = static_cast<int16_t>(
            20 * (charac_level - 1) + 6000);
        break;
    case 1u:
        info->max_hp = 400 * charac_level + 2250;
        info->max_mp = 300 * charac_level + 350;
        info->physical_attack = static_cast<uint16_t>(20 * charac_level + 244);
        info->physical_defense = static_cast<uint16_t>(20 * charac_level + 295);
        info->magical_attack = static_cast<uint16_t>(40 * charac_level - 160);
        info->magical_defense = static_cast<uint16_t>(40 * charac_level - 279);
        info->inventory_weight = 2500 * (charac_level - 1) + 300000;
        info->mp_regen_speed = static_cast<int16_t>(20 * charac_level - 230);
        info->cast_speed = static_cast<uint16_t>(20 * charac_level + 9640);
        info->hit_recovery = static_cast<int16_t>(10 * charac_level + 6160);
        break;
    case 2u:
        info->max_hp = 500 * charac_level + 450;
        info->max_mp = 200 * charac_level + 2150;
        info->physical_attack = static_cast<uint16_t>(35 * charac_level - 26);
        info->physical_defense = static_cast<uint16_t>(30 * charac_level + 115);
        info->magical_attack = static_cast<uint16_t>(35 * charac_level - 70);
        info->magical_defense = static_cast<uint16_t>(20 * charac_level + 81);
        info->inventory_weight = 2500 * (charac_level - 1) + 300000;
        info->attack_speed = static_cast<uint16_t>(10 * charac_level + 9320);
        info->hit_recovery = static_cast<int16_t>(40 * charac_level + 5620);
        break;
    case 3u:
        info->max_hp = 600 * charac_level - 1350;
        info->max_mp = 100 * charac_level + 3950;
        info->physical_attack = static_cast<uint16_t>(38 * charac_level - 80);
        info->physical_defense = static_cast<uint16_t>(40 * charac_level - 65);
        info->magical_attack = static_cast<uint16_t>(27 * charac_level + 74);
        info->magical_defense = static_cast<uint16_t>(15 * charac_level + 171);
        info->element_resist[2] = static_cast<uint16_t>(
            6 * charac_level - 308);
        info->inventory_weight = 3000 * charac_level + 288500;
        info->move_speed = static_cast<uint32_t>(10 * charac_level + 8920);
        info->hit_recovery = static_cast<int16_t>(30 * charac_level + 5800);
        break;
    case 4u:
        info->max_hp = 450 * charac_level + 1350;
        info->max_mp = 250 * charac_level + 1250;
        info->physical_attack = static_cast<uint16_t>(
            32 * (charac_level - 18) + 604);
        info->physical_defense = static_cast<uint16_t>(35 * charac_level + 25);
        info->magical_attack = static_cast<uint16_t>(30 * charac_level + 20);
        info->magical_defense = static_cast<uint16_t>(23 * charac_level + 27);
        info->inventory_weight = 2500 * (charac_level - 1) + 300000;
        info->hit_recovery = static_cast<int16_t>(30 * charac_level + 5800);
        break;
    default:
        break;
    }
}

// IDA_EA: 0x831F75C
// IDA_NAME: _ZN8WongWork15CCalcCharacStat11_CalcGunnerEP15_Additioal_infoii
// DEMANGLED: WongWork::CCalcCharacStat::_CalcGunner(_Additioal_info *,int,int)
void CCalcCharacStat::_CalcGunner(_Additioal_info* info,
                                  int grow_type,
                                  int charac_level) {
    switch (static_cast<unsigned int>(grow_type)) {
    case 0u:
        info->max_hp = 400 * (charac_level - 1) + 1250;
        info->max_mp = 300 * (charac_level - 1) + 1800;
        info->physical_attack = static_cast<uint16_t>(
            25 * (charac_level - 1) + 40);
        info->physical_defense = static_cast<uint16_t>(
            23 * (charac_level - 1) + 40);
        info->magical_attack = static_cast<uint16_t>(
            37 * (charac_level - 1) + 70);
        info->magical_defense = static_cast<uint16_t>(
            35 * (charac_level - 1) + 60);
        info->inventory_weight = 3000 * (charac_level - 1) + 330000;
        info->hit_recovery = static_cast<int16_t>(
            20 * (charac_level - 1) + 6000);
        break;
    case 1u:
        info->max_hp = 350 * charac_level + 1750;
        info->max_mp = 350 * charac_level + 600;
        info->physical_attack = static_cast<uint16_t>(35 * charac_level - 165);
        info->physical_defense = static_cast<uint16_t>(25 * charac_level - 19);
        info->magical_attack = static_cast<uint16_t>(30 * charac_level + 159);
        info->magical_defense = static_cast<uint16_t>(30 * charac_level + 115);
        info->inventory_weight = 3000 * (charac_level - 1) + 330000;
        info->attack_speed = static_cast<uint16_t>(10 * charac_level + 9320);
        info->move_speed = static_cast<uint32_t>(10 * charac_level + 8020);
        info->hit_recovery = static_cast<int16_t>(40 * charac_level + 5620);
        break;
    case 2u:
        info->max_hp = 425 * charac_level + 400;
        info->max_mp = 275 * charac_level + 1950;
        info->physical_attack = static_cast<uint16_t>(40 * charac_level - 255);
        info->physical_defense = static_cast<uint16_t>(45 * charac_level - 379);
        info->magical_attack = static_cast<uint16_t>(15 * charac_level + 429);
        info->magical_defense = static_cast<uint16_t>(20 * charac_level + 295);
        info->inventory_weight = 3000 * (charac_level - 1) + 330000;
        info->hit_recovery = static_cast<int16_t>(30 * charac_level + 5800);
        break;
    case 3u:
        info->max_hp = 300 * charac_level + 2650;
        info->max_mp = 400 * charac_level - 300;
        info->physical_attack = static_cast<uint16_t>(20 * charac_level + 105);
        info->physical_defense = static_cast<uint16_t>(20 * charac_level + 71);
        info->magical_attack = static_cast<uint16_t>(40 * charac_level - 21);
        info->magical_defense = static_cast<uint16_t>(40 * charac_level - 65);
        info->inventory_weight = 3000 * (charac_level - 1) + 330000;
        info->mp_regen_speed = static_cast<int16_t>(20 * charac_level - 185);
        info->cast_speed = static_cast<uint16_t>(20 * charac_level + 7640);
        info->hit_recovery = static_cast<int16_t>(10 * charac_level + 6160);
        break;
    case 4u:
        info->max_hp = 400 * charac_level + 850;
        info->max_mp = 300 * charac_level + 1500;
        info->physical_attack = static_cast<uint16_t>(30 * charac_level - 75);
        info->physical_defense = static_cast<uint16_t>(25 * charac_level - 19);
        info->magical_attack = static_cast<uint16_t>(35 * charac_level + 69);
        info->magical_defense = static_cast<uint16_t>(30 * charac_level + 115);
        info->inventory_weight = 3000 * (charac_level - 1) + 330000;
        info->hit_recovery = static_cast<int16_t>(30 * charac_level + 5800);
        break;
    default:
        break;
    }
}

// IDA_EA: 0x831FC3A
// IDA_NAME: _ZN8WongWork15CCalcCharacStat9_CalcMageEP15_Additioal_infoii
// DEMANGLED: WongWork::CCalcCharacStat::_CalcMage(_Additioal_info *,int,int)
void CCalcCharacStat::_CalcMage(_Additioal_info* info,
                                int grow_type,
                                int charac_level) {
    switch (static_cast<unsigned int>(grow_type)) {
    case 0u:
        info->max_hp = 325 * (charac_level - 1) + 1000;
        info->max_mp = 375 * (charac_level - 1) + 2000;
        info->physical_attack = static_cast<uint16_t>(
            20 * (charac_level - 1) + 30);
        info->physical_defense = static_cast<uint16_t>(
            20 * (charac_level - 1) + 30);
        info->magical_attack = static_cast<uint16_t>(
            40 * (charac_level - 1) + 80);
        info->magical_defense = static_cast<uint16_t>(
            40 * (charac_level - 1) + 70);
        info->inventory_weight = 2500 * (charac_level - 1) + 280000;
        info->mp_regen_speed = static_cast<int16_t>(
            10 * (charac_level - 1) + 200);
        info->hit_recovery = static_cast<int16_t>(
            10 * (charac_level - 1) + 5000);
        break;
    case 1u:
        info->max_hp = 275 * charac_level + 1575;
        info->max_mp = 425 * charac_level + 725;
        info->physical_attack = static_cast<uint16_t>(15 * charac_level + 100);
        info->physical_defense = static_cast<uint16_t>(15 * charac_level + 100);
        info->magical_attack = static_cast<uint16_t>(45 * charac_level - 50);
        info->magical_defense = static_cast<uint16_t>(45 * charac_level - 60);
        info->inventory_weight = 2500 * charac_level + 277500;
        info->mp_regen_speed = static_cast<int16_t>(20 * charac_level + 10);
        info->cast_speed = static_cast<uint16_t>(20 * charac_level + 9640);
        info->hit_recovery = static_cast<int16_t>(10 * charac_level + 4990);
        break;
    case 2u:
        info->max_hp = 300 * charac_level + 1125;
        info->max_mp = 400 * charac_level + 1175;
        info->physical_attack = static_cast<uint16_t>(20 * charac_level + 10);
        info->physical_defense = static_cast<uint16_t>(20 * charac_level + 10);
        info->magical_attack = static_cast<uint16_t>(40 * (charac_level + 1));
        info->magical_defense = static_cast<uint16_t>(40 * charac_level + 30);
        info->inventory_weight = 2500 * charac_level + 277500;
        info->mp_regen_speed = static_cast<int16_t>(20 * charac_level + 10);
        info->cast_speed = static_cast<uint16_t>(20 * charac_level + 9640);
        info->hit_recovery = static_cast<int16_t>(10 * charac_level + 4990);
        break;
    case 3u:
        info->max_hp = 400 * charac_level - 675;
        info->max_mp = 300 * charac_level + 2975;
        info->physical_attack = static_cast<uint16_t>(35 * charac_level - 260);
        info->physical_defense = static_cast<uint16_t>(25 * charac_level - 80);
        info->magical_attack = static_cast<uint16_t>(35 * charac_level + 130);
        info->magical_defense = static_cast<uint16_t>(25 * charac_level + 300);
        info->inventory_weight = 2500 * charac_level + 277500;
        info->attack_speed = static_cast<uint16_t>(10 * charac_level + 9820);
        info->move_speed = static_cast<uint32_t>(10 * charac_level + 9820);
        info->hit_recovery = static_cast<int16_t>(30 * charac_level + 4630);
        info->mp_regen_speed = static_cast<int16_t>(10 * charac_level + 190);
        break;
    case 4u:
        info->max_hp = 350 * charac_level + 225;
        info->max_mp = 350 * charac_level + 2075;
        info->physical_attack = static_cast<uint16_t>(25 * charac_level - 80);
        info->physical_defense = static_cast<uint16_t>(25 * charac_level - 80);
        info->magical_attack = static_cast<uint16_t>(35 * charac_level + 130);
        info->magical_defense = static_cast<uint16_t>(35 * charac_level + 120);
        info->inventory_weight = 2500 * charac_level + 277500;
        info->mp_regen_speed = static_cast<int16_t>(20 * charac_level + 10);
        info->cast_speed = static_cast<uint16_t>(10 * charac_level + 9820);
        info->hit_recovery = static_cast<int16_t>(20 * charac_level + 4810);
        break;
    default:
        break;
    }
}

}  // namespace WongWork
