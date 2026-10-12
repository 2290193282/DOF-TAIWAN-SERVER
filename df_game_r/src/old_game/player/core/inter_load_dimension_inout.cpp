#include "old_data/db/taiwan/inter_slots_331_353.h"

#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"


// IDA_EA: 0x84D83D2
// IDA_NAME: _ZN24Inter_LoadDimensionInout12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_LoadDimensionInout::dispatch_sig(CUser *,char *,int)
int Inter_LoadDimensionInout::dispatch_sig(CUser* user,
                                           char* payload,
                                           int payload_size) {
    if (!payload || payload_size != static_cast<int>(sizeof(SIG_LOAD_DIMENSION))) {
        return 16114;
    }

    const SIG_LOAD_DIMENSION* const reply =
        reinterpret_cast<const SIG_LOAD_DIMENSION*>(payload);
    bool changed_dimension = false;
    bool changed_blood = false;
    bool changed_tournament = false;

    if (!user->get_state()) {
        return 16115;
    }

    const int character_index = user->get_charac_idx(
        static_cast<int>(reply->charac_no));
    if (character_index < 0) {
        return 16121;
    }

    _Charac_info& character = user->characters_[character_index];
    if (character.fields_.character_no_ !=
        static_cast<int32_t>(reply->charac_no)) {
        return 16123;
    }

    const bool current_character =
        user->getCurCharacR() &&
        user->getCurCharacNo() == static_cast<int>(reply->charac_no);
    const bool reset_for_today =
        current_character && user->IsFirstTimeLogin();
    if (!dungeon_runtime::ApplyInCountLoad(user,
                                        reply->charac_no,
                                        reply->limited_inout,
                                        reset_for_today)) {
        return 16124;
    }

    character.fields_.character_no_ =
        static_cast<int32_t>(reply->charac_no);
    for (int index = 0; index <= 5; ++index) {
        character.fields_.dimension_inout_[index] = reply->dimension[index];
        if (character.fields_.dimension_inout_[index] !=
            static_cast<uint8_t>(
                G_CDataManager()->get_dimensionInout(index))) {
            changed_dimension = true;
        }
    }

    character.fields_.ultimate_inout_[0] = reply->tournament_inout;
    CDungeon* const black_dragon = G_CDataManager()->find_dungeon(122);
    const uint8_t tournament_limit = black_dragon
        ? static_cast<uint8_t>(black_dragon->get_limit_Inout_count())
        : 0;
    if (character.fields_.ultimate_inout_[0] != tournament_limit) {
        changed_tournament = true;
    }

    character.fields_.blood_inout_count_ = reply->blood_inout;
    if (character.fields_.blood_inout_count_ !=
        static_cast<uint8_t>(G_CDataManager()->get_limit_inout_count(2))) {
        changed_blood = true;
    }
    character.fields_.dimension_dirty_ = 0;
    character.fields_.ultimate_inout_dirty_ = 0;

    if (current_character && user->IsFirstTimeLogin() &&
        user->get_charac_level() > 59) {
        user->DimensionInoutUpdate(
            changed_dimension, changed_blood || changed_tournament);
    }
    if (current_character) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             ENUM_NOTIPACKET_USERINFO,
                             1);
    }
    return 0;
}


// IDA_EA: 0x84EDF50
// IDA_NAME: _ZN24Inter_LoadDimensionInoutC2Ev
// DEMANGLED: Inter_LoadDimensionInout::Inter_LoadDimensionInout(void)
Inter_LoadDimensionInout::Inter_LoadDimensionInout() {}

// IDA_EA: 0x84F84A8 (D2), 0x84F84D8 (D0)
Inter_LoadDimensionInout::~Inter_LoadDimensionInout() {}
