#include "old_game/gm/player/game_master_character_select_commands.h"

#include <cstdio>
#include <cstring>

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "common/manager/play_info_public_abi.h"
#include "old_data/db/character/db_req_change_charac_name.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

namespace game_master {

// IDA_EA: 0x82A7FFE
// IDA_NAME: _ZN11game_master14CMoveCharacCmdC2Ev
// DEMANGLED: game_master::CMoveCharacCmd::CMoveCharacCmd(void)
CMoveCharacCmd::CMoveCharacCmd() : CCommand(), character_name_() {}

// IDA_EA: 0x84A8CCE
// IDA_NAME: _ZN11game_master14CMoveCharacCmd5parseEv
// DEMANGLED: game_master::CMoveCharacCmd::parse(void)
void CMoveCharacCmd::parse() {
    std::string& command_tail = parser_->command_tail();
    trim(command_tail);
    character_name_ = command_tail;
}

// IDA_EA: 0x84A8D02
// IDA_NAME: _ZN11game_master14CMoveCharacCmd7executeEv
// DEMANGLED: game_master::CMoveCharacCmd::execute(void)
void CMoveCharacCmd::execute() {
    CUser* const target_user =
        G_GameWorld()->find_user_by_charac_name(character_name_.c_str());
    if (!target_user || !target_user->getCurCharacR()) {
        return;
    }

    const int target_village =
        static_cast<int>(static_cast<signed char>(target_user->getCurCharacVill()));
    const int target_area = target_user->get_area(false);
    const unsigned short target_x =
        static_cast<unsigned short>(target_user->get_posX());
    const unsigned short target_y =
        static_cast<unsigned short>(target_user->get_posY());
    CUser* const user = GetUser();

    if (static_cast<int>(static_cast<signed char>(user->getCurCharacVill())) !=
            target_village ||
        user->get_area(false) != target_area) {
        G_GameWorld()->move_area(user,
                                 target_village,
                                 target_area,
                                 target_x,
                                 target_y,
                                 0,
                                 false,
                                 0,
                                 0,
                                 0);
    } else {
        user->set_position(target_x, target_y, 0);
    }

    PacketGuard packet;
    packet.put_header(0, 138);
    packet.put_short(static_cast<short>(user->get_unique_id()));
    packet.put_short(static_cast<short>(target_x));
    packet.put_short(static_cast<short>(target_y));
    packet.put_byte(0);
    packet.finalize(true);
    if (target_village || target_area) {
        G_GameWorld()->send_to_area(target_village, target_area, packet);
    }
}

// IDA_EA: 0x82A8098
// IDA_NAME: _ZN11game_master14CCharacStatCmdC2Ev
// DEMANGLED: game_master::CCharacStatCmd::CCharacStatCmd(void)
CCharacStatCmd::CCharacStatCmd() : CCommand() {}

// IDA_EA: 0x84A905E
// IDA_NAME: _ZN11game_master14CCharacStatCmd7executeEv
// DEMANGLED: game_master::CCharacStatCmd::execute(void)
void CCharacStatCmd::execute() {
    char text[256] = {};
    CUser* const user = GetUser();
    const _Additioal_info* const stat = user->getCurCharacAddInfoR();
    std::sprintf(text,
                 "charac_stat move_speed=%d,attack_speed=%d,cast_speed=%d",
                 static_cast<int>(stat->move_speed),
                 static_cast<int>(stat->attack_speed),
                 static_cast<int>(stat->cast_speed));

    PacketGuard packet;
    packet.put_header(0, 12);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_byte(0);
    const int text_length = static_cast<int>(std::strlen(text));
    packet.put_int(text_length);
    packet.put_str(text, text_length);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x82A8418
// IDA_NAME: _ZN11game_master17CChangeCharacNameC2Ev
// DEMANGLED: game_master::CChangeCharacName::CChangeCharacName(void)
CChangeCharacName::CChangeCharacName() : CCommand(), character_name_() {}

// IDA_EA: 0x84AB524
// IDA_NAME: _ZN11game_master17CChangeCharacName5parseEv
// DEMANGLED: game_master::CChangeCharacName::parse(void)
void CChangeCharacName::parse() {
    std::string& command_tail = parser_->command_tail();
    trim(command_tail);
    character_name_ = command_tail;
}

// IDA_EA: 0x84AB558
// IDA_NAME: _ZN11game_master17CChangeCharacName7executeEv
// DEMANGLED: game_master::CChangeCharacName::execute(void)
void CChangeCharacName::execute() {
    CUser* const user = GetUser();
    DB_ReqChangeCharacName::makeRequest(user->GetUID(),
                                        user->getCurCharacNo(),
                                        character_name_.c_str(),
                                        1);
}

// IDA_EA: 0x82A855A
// IDA_NAME: _ZN11game_master17CAdjustCharacStatC2Ev
// DEMANGLED: game_master::CAdjustCharacStat::CAdjustCharacStat(void)
CAdjustCharacStat::CAdjustCharacStat() : CCommand() {}

// IDA_EA: 0x84AC58C
// IDA_NAME: _ZN11game_master17CAdjustCharacStat7executeEv
// DEMANGLED: game_master::CAdjustCharacStat::execute(void)
void CAdjustCharacStat::execute() {
    CUser* const user = GetUser();
    user->adjust_charac_stat();

    PacketGuard packet;
    packet.put_header(0, 2);
    packet.put_byte(1);
    packet.put_short(1);
    user->make_basic_info(packet, 1);
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace game_master
