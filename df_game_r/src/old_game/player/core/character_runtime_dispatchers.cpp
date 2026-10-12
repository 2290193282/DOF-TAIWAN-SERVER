#include "old_game/player/core/character_runtime_dispatchers.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"

#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/db/skill/db_request_charac_skill_info.h"
#include "old_data/db/character/db_req_change_charac_name.h"
#include "common/manager/data_manager.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/security/policy/restrict_inputting_name.h"
#include "common/manager/user_history_log_abi.h"
#include "common/manager/user.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/character_packets.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/equipment/expand_equipslot_runtime.h"
#include "old_game/world/region/game_world.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>


int StrikerSkillMinLevel() {
    return G_CDataManager()->get_striker_skill_minimum_level();
}

// IDA_EA: 0x82051D0
// IDA_NAME: _ZN29Dispatcher_CharacterStatistic12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CharacterStatistic::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CharacterStatistic::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }

    int attack_count = 0;
    int damaged_count = 0;
    int skill_count = 0;
    int attack_damage = 0;
    int damaged_value = 0;
    int clear_time = 0;
    if (packet.get_int(attack_count) != 1) {
        return LineFunc(
            40223,
            "virtual int Dispatcher_CharacterStatistic::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }
    if (packet.get_int(skill_count) != 1) {
        return LineFunc(
            40224,
            "virtual int Dispatcher_CharacterStatistic::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }
    if (packet.get_int(attack_damage) != 1) {
        return LineFunc(
            40225,
            "virtual int Dispatcher_CharacterStatistic::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }
    if (packet.get_int(damaged_count) != 1) {
        return LineFunc(
            40226,
            "virtual int Dispatcher_CharacterStatistic::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }
    if (packet.get_int(damaged_value) != 1) {
        return LineFunc(
            40227,
            "virtual int Dispatcher_CharacterStatistic::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }
    if (packet.get_int(clear_time) != 1) {
        return LineFunc(
            40228,
            "virtual int Dispatcher_CharacterStatistic::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    user->character_.hp_consume_ = static_cast<uint32_t>(attack_count);
    user->character_.hit_count_ = static_cast<uint32_t>(skill_count);
    user->character_.hit_per_average_damage_ =
        static_cast<uint32_t>(attack_damage);
    user->character_.mp_consume_ = static_cast<uint32_t>(damaged_count);
    user->character_.hp_recovery_ = static_cast<uint32_t>(damaged_value);
    user->character_.mp_recovery_ = static_cast<uint32_t>(clear_time);
    user->character_statistic_.statistic_received_ = 1u;
    return 0;
}

// IDA_EA: 0x823384E
// IDA_NAME: _ZN29Dispatcher_CharacterStatistic4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CharacterStatistic::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CharacterStatistic::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233858
// IDA_NAME: _ZN29Dispatcher_CharacterStatistic7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CharacterStatistic::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CharacterStatistic::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596118
// IDA_NAME: _ZN29Dispatcher_CharacterStatisticC2Ev
// DEMANGLED: Dispatcher_CharacterStatistic::Dispatcher_CharacterStatistic(void)
Dispatcher_CharacterStatistic::Dispatcher_CharacterStatistic()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8242A5A
// IDA_NAME: _ZN29Dispatcher_CharacterStatisticD2Ev
// DEMANGLED: Dispatcher_CharacterStatistic::~Dispatcher_CharacterStatistic()
// IDA_EA: 0x8242A8A
// IDA_NAME: _ZN29Dispatcher_CharacterStatisticD0Ev
// DEMANGLED: Dispatcher_CharacterStatistic::~Dispatcher_CharacterStatistic()
Dispatcher_CharacterStatistic::~Dispatcher_CharacterStatistic() {}

// IDA_EA: 0x81C297A
// IDA_NAME: _ZN29DisPatcher_DieCharacInDungeon4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_DieCharacInDungeon::read(PacketBuf &,MSG_BASE &)
int DisPatcher_DieCharacInDungeon::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    MSG_DIE_CHARACTER& request = static_cast<MSG_DIE_CHARACTER&>(message);
    if (packet.get_short(request.death_reason) != 1) {
        return LineFunc(
            3241,
            "virtual int DisPatcher_DieCharacInDungeon::read(PacketBuf&, MSG_BASE&)",
            0, 0);
    }
    if (packet.get_short(request.death_value) == 1) {
        return 0;
    }
    return LineFunc(
        3242,
        "virtual int DisPatcher_DieCharacInDungeon::read(PacketBuf&, MSG_BASE&)",
        0, 0);
}

// IDA_EA: 0x81C2BAA
// IDA_NAME: _ZN29DisPatcher_DieCharacInDungeon4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_DieCharacInDungeon::send(CUser *,ParamBase &)
void DisPatcher_DieCharacInDungeon::send(CUser* user, ParamBase& param) {
    (void)user;
    (void)param;
}

// IDA_EA: 0x81C2BC0
// IDA_NAME: _ZN29DisPatcher_DieCharacInDungeon11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_DieCharacInDungeon::check_error(CUser *,MSG_BASE &)
int DisPatcher_DieCharacInDungeon::check_error(CUser* user,
                                               MSG_BASE& message) {
    (void)message;
    if (user && zombie_mode::IsRevivalBlocked(user->GetPVPRoom())) {
        return 19;
    }
    const unsigned int state =
        static_cast<unsigned int>(user->get_state());
    if (Global::CGlobalFunc::Get_State_Check(state)) {
        return 0;
    }
    return -1;
}

// IDA_EA: 0x8232242
// IDA_NAME: _ZN29DisPatcher_DieCharacInDungeon11check_stateEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_DieCharacInDungeon::check_state(CUser *,PacketBuf &)
int DisPatcher_DieCharacInDungeon::check_state(CUser* user,
                                               PacketBuf& packet) {
    (void)packet;
    const unsigned int state =
        static_cast<unsigned int>(user->get_state());
    if (Global::CGlobalFunc::Get_State_Check(state)) {
        return 0;
    }
    return LineFunc(
        410,
        "virtual int DisPatcher_DieCharacInDungeon::check_state(CUser*, PacketBuf&)",
        0, 0);
}

// IDA_EA: 0x81C2A12
// IDA_NAME: _ZN29DisPatcher_DieCharacInDungeon7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_DieCharacInDungeon::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_DieCharacInDungeon::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    stDieCharacInDungeon& result =
        static_cast<stDieCharacInDungeon&>(param);
    result.dword_04 = static_cast<uint32_t>(check_error(user, message));
    const int error = static_cast<int>(result.dword_04);
    if (error > 0) {
        return 0;
    }
    if (error < 0) {
        return LineFunc(
            3259,
            "virtual int DisPatcher_DieCharacInDungeon::process(CUser*, MSG_BASE&, ParamBase&)",
            error,
            user->get_acc_id());
    }

    const char* const character_name = user->getCurCharacName();
    const MSG_DIE_CHARACTER& request =
        static_cast<const MSG_DIE_CHARACTER&>(message);
    CUser_history_log(user)->CharacDie(
        character_name,
        request.death_reason,
        request.death_value);

    CParty* const indexed_party =
        G_CGameManager()->GetParty(user->GetPartyIndex());
    if (Global::CBossTowerFunc::DieCharacInDungeon(user)) {
        return 0;
    }

    if (user->get_state() == 13) {
        advancealtar::Manager::dieCharac(user);
    }

    CParty* const current_party = user->GetParty();
    if (current_party) {
        CDungeon* const dungeon = current_party->battle_field().GetDungeon();
        if (dungeon && dungeon->get_blood_dungeon_type() > 0) {
            current_party->BloodHandleDieUser(user);
            result.dword_04 = 0x7fffffffu;
            return 0;
        }
    }

    if (indexed_party) {
        indexed_party->die_user(user);
    }
    return 0;
}

// IDA_EA: 0x82463F0
// IDA_NAME: _ZN29DisPatcher_DieCharacInDungeonD2Ev
// DEMANGLED: DisPatcher_DieCharacInDungeon::~DisPatcher_DieCharacInDungeon()
// IDA_EA: 0x8246420
// IDA_NAME: _ZN29DisPatcher_DieCharacInDungeonD0Ev
// DEMANGLED: DisPatcher_DieCharacInDungeon::~DisPatcher_DieCharacInDungeon()
DisPatcher_DieCharacInDungeon::~DisPatcher_DieCharacInDungeon() {}

// IDA_EA: 0x8595A88
// IDA_NAME: _ZN29DisPatcher_DieCharacInDungeonC2Ev
// DEMANGLED: DisPatcher_DieCharacInDungeon::DisPatcher_DieCharacInDungeon(void)
DisPatcher_DieCharacInDungeon::DisPatcher_DieCharacInDungeon()
    : IPacketDispatcher<MSG_DIE_CHARACTER,
                        stDieCharacInDungeon,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8259F68 / 0x825A0B8 / 0x825D534 / 0x825D564 / 0x825DA4C /
//         0x825DADE / 0x85984E2
// The typed CMD43 state-0 template is owned here with its original message
// pair; no MSG_BASE fallback is permitted.
template class IPacketDispatcher<MSG_DIE_CHARACTER,
                                 stDieCharacInDungeon,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x81E3914
// IDA_NAME: _ZN33Dispatcher_RequestCharacSkillInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestCharacSkillInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestCharacSkillInfo::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    MSG_REQUEST_CHARAC_SKILL_INFO& request =
        static_cast<MSG_REQUEST_CHARAC_SKILL_INFO&>(message);
    if (packet.get_byte(request.character_slot) != 1) {
        return LineFunc(
            23323,
            "virtual int Dispatcher_RequestCharacSkillInfo::read(PacketBuf&, MSG_BASE&)",
            0, 0);
    }
    if (request.character_slot < 0x24u) {
        return 0;
    }
    LogManager::logFormat(
        1,
        "PacketDispatcher_Impl_1.cpp",
        "virtual int Dispatcher_RequestCharacSkillInfo::read(PacketBuf&, MSG_BASE&)",
        23328,
        "RequestCharacSkillInfo ReadPacket ERROR! : characIdx(%d)",
        static_cast<int>(static_cast<signed char>(request.character_slot)));
    return LineFunc(
        23330,
        "virtual int Dispatcher_RequestCharacSkillInfo::read(PacketBuf&, MSG_BASE&)",
        0, 0);
}

// IDA_EA: 0x81E39E0
// IDA_NAME: _ZN33Dispatcher_RequestCharacSkillInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestCharacSkillInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestCharacSkillInfo::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    int& result = static_cast<stRequestCharacSkillInfo&>(param).dword_04;
    result = check_error(user, message);
    if (result > 0) {
        return 0;
    }
    if (result < 0) {
        return LineFunc(
            23346,
            "virtual int Dispatcher_RequestCharacSkillInfo::process(CUser*, MSG_BASE&, ParamBase&)",
            result,
            0);
    }

    const MSG_REQUEST_CHARAC_SKILL_INFO& request =
        static_cast<const MSG_REQUEST_CHARAC_SKILL_INFO&>(message);
    const unsigned char charac_idx = request.character_slot;
    _Charac_info& character = user->characters_[charac_idx];
    SkillSlot& skill_slot = character.fields_.skill_slot_;
    if (skill_slot.IsLoadStrikerSkill()) {
        const int skill_tree_index =
            static_cast<signed char>(character.fields_.skill_tree_index_);
        skill_slot.SendSkillInfo(
            charac_idx,
            static_cast<ENUM_SKILL_TREE_KIND>(skill_tree_index));
        return 0;
    }

    Stream* stream =
        GlobalData::stream_pool()->Acquire("PacketDispatcher_Impl_1.cpp",
                                           23360);
    CStreamGuard guard(stream, true);
    *guard << 765;
    *guard << user->GetUID();
    SIG_REQUEST_CHARAC_SKILL_INFO* payload =
        guard.GetInBuffer<SIG_REQUEST_CHARAC_SKILL_INFO>();
    payload->charac_no = static_cast<uint32_t>(
        character.fields_.character_no_);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x81E3C08
// IDA_NAME: _ZN33Dispatcher_RequestCharacSkillInfo11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_RequestCharacSkillInfo::check_error(CUser *,MSG_BASE &)
int Dispatcher_RequestCharacSkillInfo::check_error(CUser* user,
                                                   MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    if (user->get_state() <= 2) {
        return -1;
    }

    const MSG_REQUEST_CHARAC_SKILL_INFO& request =
        static_cast<const MSG_REQUEST_CHARAC_SKILL_INFO&>(message);
    const unsigned char charac_idx = request.character_slot;
    if (charac_idx >= user->characters_.size()) {
        return 57;
    }

    const _Charac_info& character = user->characters_[charac_idx];
    if (!character.fields_.character_no_) {
        return 21;
    }

    const int level = character.fields_.level_;
    if (level >= StrikerSkillMinLevel()) {
        return 0;
    }
    return 14;
}

// IDA_EA: 0x81E3CFA
// IDA_NAME: _ZN33Dispatcher_RequestCharacSkillInfo4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_RequestCharacSkillInfo::send(CUser *,ParamBase &)
void Dispatcher_RequestCharacSkillInfo::send(CUser* user, ParamBase& param) {
    const int result = static_cast<stRequestCharacSkillInfo&>(param).dword_04;
    if (result) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REQUEST_CHARAC_SKILL_INFO,
            static_cast<unsigned char>(result));
    }
}

// IDA_EA: 0x85958E4
// IDA_NAME: _ZN33Dispatcher_RequestCharacSkillInfoC2Ev
// DEMANGLED: Dispatcher_RequestCharacSkillInfo::Dispatcher_RequestCharacSkillInfo(void)
Dispatcher_RequestCharacSkillInfo::Dispatcher_RequestCharacSkillInfo()
    : IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                        stRequestCharacSkillInfo,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8243F12
// IDA_NAME: _ZN33Dispatcher_RequestCharacSkillInfoD2Ev
// DEMANGLED: Dispatcher_RequestCharacSkillInfo::~Dispatcher_RequestCharacSkillInfo()
// IDA_EA: 0x8243F42
// IDA_NAME: _ZN33Dispatcher_RequestCharacSkillInfoD0Ev
// DEMANGLED: Dispatcher_RequestCharacSkillInfo::~Dispatcher_RequestCharacSkillInfo()
Dispatcher_RequestCharacSkillInfo::~Dispatcher_RequestCharacSkillInfo() {}

// IDA_EA: 0x81D606E
// IDA_NAME: _ZN27Dispatcher_Equipslot_Switch4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Equipslot_Switch::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Equipslot_Switch::read(PacketBuf& packet,
                                      MSG_BASE& message) {
    MSG_EQUIPSLOT_SWITCH& request =
        static_cast<MSG_EQUIPSLOT_SWITCH&>(message);
    if (packet.get_byte(request.channel_equipslot_switch) != 1) {
        return LineFunc(
            15890,
            "virtual int Dispatcher_Equipslot_Switch::read(PacketBuf&, MSG_BASE&)",
            0, 0);
    }
    if (packet.get_byte(request.expand_equipslot_switch) != 1) {
        return LineFunc(
            15892,
            "virtual int Dispatcher_Equipslot_Switch::read(PacketBuf&, MSG_BASE&)",
            0, 0);
    }
    if (packet.get_byte(request.requested_switch) == 1) {
        return 0;
    }
    return LineFunc(
        15894,
        "virtual int Dispatcher_Equipslot_Switch::read(PacketBuf&, MSG_BASE&)",
        0, 0);
}

// IDA_EA: 0x81D614A
// IDA_NAME: _ZN27Dispatcher_Equipslot_Switch11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_Equipslot_Switch::check_error(CUser *,MSG_BASE &)
int Dispatcher_Equipslot_Switch::check_error(CUser* user,
                                             MSG_BASE& message) {
    (void)message;
    if (!user) {
        return LineFunc(15900,
                        "int Dispatcher_Equipslot_Switch::check_error(CUser*, MSG_BASE&)",
                        0, 0);
    }
    if (user->get_state() > 2) {
        return 0;
    }
    return LineFunc(15901,
                    "int Dispatcher_Equipslot_Switch::check_error(CUser*, MSG_BASE&)",
                    0, 0);
}

// IDA_EA: 0x81D61BE
// IDA_NAME: _ZN27Dispatcher_Equipslot_Switch7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Equipslot_Switch::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Equipslot_Switch::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    const int error = check_error(user, message);
    if (error > 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EQUIPSLOT_SWITCH,
                                 static_cast<unsigned char>(error));
        return -1;
    }
    if (error < 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_EQUIPSLOT_SWITCH, 21);
        return LineFunc(
            15916,
            "virtual int Dispatcher_Equipslot_Switch::process(CUser*, MSG_BASE&, ParamBase&)",
            0, 0);
    }

    const MSG_EQUIPSLOT_SWITCH& request =
        static_cast<const MSG_EQUIPSLOT_SWITCH&>(message);
    stEquipslotSwitch& response = static_cast<stEquipslotSwitch&>(param);
    CExpandEquipslot* const equipslots = static_cast<CExpandEquipslot*>(
        user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
    response.byte_04 = static_cast<uint8_t>(equipslots->EquipslotSwitch(
        user,
        static_cast<char>(request.requested_switch),
        static_cast<char>(request.channel_equipslot_switch),
        static_cast<char>(request.expand_equipslot_switch)));
    return 0;
}

// IDA_EA: 0x81D62C2
// IDA_NAME: _ZN27Dispatcher_Equipslot_Switch4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Equipslot_Switch::send(CUser *,ParamBase &)
void Dispatcher_Equipslot_Switch::send(CUser* user, ParamBase& param) {
    const stEquipslotSwitch& response =
        static_cast<const stEquipslotSwitch&>(param);
    if (response.byte_04 == 0) {
        CExpandEquipslot* const equipslots = static_cast<CExpandEquipslot*>(
            user->GetCharacExpandData(static_cast<ENUM_CHARAC_EXPAND_TYPE>(9)));
        equipslots->Send_Equip_Info(user);
        equipslots->Send_Expand_Equip_Info(user);
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_EQUIPSLOT_SWITCH);
    if (response.byte_04 != 0) {
        packet.put_byte(0);
        packet.put_byte(static_cast<char>(response.byte_04));
    } else {
        packet.put_byte(1);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8244E4E
// IDA_NAME: _ZN27Dispatcher_Equipslot_SwitchD2Ev
// DEMANGLED: Dispatcher_Equipslot_Switch::~Dispatcher_Equipslot_Switch()
// IDA_EA: 0x8244E7E
// IDA_NAME: _ZN27Dispatcher_Equipslot_SwitchD0Ev
// DEMANGLED: Dispatcher_Equipslot_Switch::~Dispatcher_Equipslot_Switch()
Dispatcher_Equipslot_Switch::~Dispatcher_Equipslot_Switch() {}

// IDA_EA: 0x85976E2
// IDA_NAME: _ZN27Dispatcher_Equipslot_SwitchC2Ev
// DEMANGLED: Dispatcher_Equipslot_Switch::Dispatcher_Equipslot_Switch(void)
Dispatcher_Equipslot_Switch::Dispatcher_Equipslot_Switch()
    : IPacketDispatcher<MSG_EQUIPSLOT_SWITCH,
                        stEquipslotSwitch,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x81D5EBA
// IDA_NAME: _ZN39Dispatcher_Expand_Equipslot_Flag_Update4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Expand_Equipslot_Flag_Update::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Expand_Equipslot_Flag_Update::read(PacketBuf& packet,
                                                  MSG_BASE& message) {
    MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE& request =
        static_cast<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE&>(message);
    if (packet.get_byte(request.requested_flag) == 1) {
        return 0;
    }
    return LineFunc(
        15844,
        "virtual int Dispatcher_Expand_Equipslot_Flag_Update::read(PacketBuf&, MSG_BASE&)",
        0, 0);
}

// IDA_EA: 0x81D5F10
// IDA_NAME: _ZN39Dispatcher_Expand_Equipslot_Flag_Update7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Expand_Equipslot_Flag_Update::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Expand_Equipslot_Flag_Update::process(CUser* user,
                                                      MSG_BASE& message,
                                                      ParamBase& param) {
    if (!user) {
        return 1;
    }
    if (user->get_state() <= 2) {
        return 19;
    }

    const MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE& request =
        static_cast<const MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE&>(message);
    const char result = static_cast<char>(
        user->UpdateEquipslotFlag(static_cast<char>(request.requested_flag)));
    if (result == 19) {
        return LineFunc(
            15859,
            "virtual int Dispatcher_Expand_Equipslot_Flag_Update::process(CUser*, MSG_BASE&, ParamBase&)",
            0,
            user->get_acc_id());
    }

    stExpandEquipslotFlagUpdate& response =
        static_cast<stExpandEquipslotFlagUpdate&>(param);
    response.byte_04 = static_cast<uint8_t>(result);
    return 0;
}

// IDA_EA: 0x81D5FAE
// IDA_NAME: _ZN39Dispatcher_Expand_Equipslot_Flag_Update4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Expand_Equipslot_Flag_Update::send(CUser *,ParamBase &)
void Dispatcher_Expand_Equipslot_Flag_Update::send(CUser* user,
                                                   ParamBase& param) {
    const stExpandEquipslotFlagUpdate& result =
        static_cast<const stExpandEquipslotFlagUpdate&>(param);
    PacketGuard packet;
    packet.clear();
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_EXPAND_EQUIPSLOT_FLAG_UPDATE);
    packet.put_byte(1);
    packet.put_byte(static_cast<char>(result.byte_04));
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8244E9C
// IDA_NAME: _ZN39Dispatcher_Expand_Equipslot_Flag_UpdateD2Ev
// DEMANGLED: Dispatcher_Expand_Equipslot_Flag_Update::~Dispatcher_Expand_Equipslot_Flag_Update()
// IDA_EA: 0x8244ECC
// IDA_NAME: _ZN39Dispatcher_Expand_Equipslot_Flag_UpdateD0Ev
// DEMANGLED: Dispatcher_Expand_Equipslot_Flag_Update::~Dispatcher_Expand_Equipslot_Flag_Update()
Dispatcher_Expand_Equipslot_Flag_Update::~Dispatcher_Expand_Equipslot_Flag_Update() {}

// IDA_EA: 0x85976FE
// IDA_NAME: _ZN39Dispatcher_Expand_Equipslot_Flag_UpdateC2Ev
// DEMANGLED: Dispatcher_Expand_Equipslot_Flag_Update::Dispatcher_Expand_Equipslot_Flag_Update(void)
Dispatcher_Expand_Equipslot_Flag_Update::Dispatcher_Expand_Equipslot_Flag_Update()
    : IPacketDispatcher<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE,
                        stExpandEquipslotFlagUpdate,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x81D63E6
// IDA_NAME: _ZN37Dispatcher_Buy_Charac_Status_Using_QP11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_Buy_Charac_Status_Using_QP::check_error(CUser *,MSG_BASE &)
int Dispatcher_Buy_Charac_Status_Using_QP::check_error(CUser* user,
                                                       MSG_BASE& message) {
    (void)message;
    if (!user) {
        return -1;
    }
    if (user->get_state() == 3) {
        return 0;
    }
    return 99;
}

// IDA_EA: 0x81D641C
// IDA_NAME: _ZN37Dispatcher_Buy_Charac_Status_Using_QP4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Buy_Charac_Status_Using_QP::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Buy_Charac_Status_Using_QP::read(PacketBuf& packet,
                                                MSG_BASE& message) {
    MSG_QUEST_SHOP& request = static_cast<MSG_QUEST_SHOP&>(message);
    if (packet.get_byte(request.status_id) != 1) {
        return LineFunc(
            16312,
            "virtual int Dispatcher_Buy_Charac_Status_Using_QP::read(PacketBuf&, MSG_BASE&)",
            0, 0);
    }
    if (packet.get_byte(request.count) == 1) {
        return 0;
    }
    return LineFunc(
        16314,
        "virtual int Dispatcher_Buy_Charac_Status_Using_QP::read(PacketBuf&, MSG_BASE&)",
        0, 0);
}

// IDA_EA: 0x81D64B4
// IDA_NAME: _ZN37Dispatcher_Buy_Charac_Status_Using_QP7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Buy_Charac_Status_Using_QP::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Buy_Charac_Status_Using_QP::process(CUser* user,
                                                   MSG_BASE& message,
                                                   ParamBase& param) {
    const int check = check_error(user, message);
    if (check > 0) {
        return -1;
    }
    if (check < 0) {
        return LineFunc(
            16329,
            "virtual int Dispatcher_Buy_Charac_Status_Using_QP::process(CUser*, MSG_BASE&, ParamBase&)",
            0, 0);
    }

    const MSG_QUEST_SHOP& request =
        static_cast<const MSG_QUEST_SHOP&>(message);
    STQuestShopBuyParam& response =
        static_cast<STQuestShopBuyParam&>(param);
    CQuestShop* quest_shop = static_cast<CQuestShop*>(
        user->GetCharacExpandData(
            static_cast<ENUM_CHARAC_EXPAND_TYPE>(11)));
    if (!quest_shop) {
        response.result = 1;
        return 0;
    }

    short value = 0;
    const int result =
        quest_shop->buyStatus(
            user,
            request.status_id,
            request.count,
            value);
    if (result) {
        response.result = result;
    } else {
        response.status_id = request.status_id;
        response.buying_count = static_cast<uint16_t>(value);
    }
    return 0;
}

// IDA_EA: 0x81D65C2
// IDA_NAME: _ZN37Dispatcher_Buy_Charac_Status_Using_QP4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_Buy_Charac_Status_Using_QP::send(CUser *,ParamBase &)
void Dispatcher_Buy_Charac_Status_Using_QP::send(CUser* user,
                                                 ParamBase& param) {
    const STQuestShopBuyParam& result =
        static_cast<const STQuestShopBuyParam&>(param);
    PacketGuard response;
    response.clear();
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_BUY_CHARAC_STATUS_USING_QP);
    if (result.result) {
        response.put_byte(0);
        response.put_short(result.result);
        response.finalize(true);
        user->Send(response);
        return;
    }

    response.put_byte(1);
    response.put_byte(result.status_id);
    response.put_short(result.buying_count);
    response.finalize(true);
    user->Send(response);
    user->sendCharacQp();
}

// IDA_EA: 0x8597752
// IDA_NAME: _ZN37Dispatcher_Buy_Charac_Status_Using_QPC2Ev
// DEMANGLED: Dispatcher_Buy_Charac_Status_Using_QP::Dispatcher_Buy_Charac_Status_Using_QP(void)
Dispatcher_Buy_Charac_Status_Using_QP::Dispatcher_Buy_Charac_Status_Using_QP()
    : IPacketDispatcher<MSG_QUEST_SHOP,
                        STQuestShopBuyParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8244E00
// IDA_NAME: _ZN37Dispatcher_Buy_Charac_Status_Using_QPD2Ev
// DEMANGLED: Dispatcher_Buy_Charac_Status_Using_QP::~Dispatcher_Buy_Charac_Status_Using_QP()
// IDA_EA: 0x8244E30
// IDA_NAME: _ZN37Dispatcher_Buy_Charac_Status_Using_QPD0Ev
// DEMANGLED: Dispatcher_Buy_Charac_Status_Using_QP::~Dispatcher_Buy_Charac_Status_Using_QP()
Dispatcher_Buy_Charac_Status_Using_QP::~Dispatcher_Buy_Charac_Status_Using_QP() {}

// IDA_EA: 0x8205898
// IDA_NAME: _ZN27Dispatcher_ChangeCharacName12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ChangeCharacName::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ChangeCharacName::dispatch_sig(CUser* user,
                                              PacketBuf& packet) {
    if (user->get_state() != 2) {
        return LineFunc(
            40627,
            "virtual int Dispatcher_ChangeCharacName::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    unsigned char charac_slot = 0;
    int src_name_len = 0;
    int dst_name_len = 0;
    char src_name[30];
    char dst_name[30];
    std::memset(src_name, 0, sizeof(src_name));
    std::memset(dst_name, 0, sizeof(dst_name));

    if (packet.get_byte(charac_slot) != 1) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                 7);
        return 0;
    }

    int read_ret = packet.get_int(src_name_len);
    if (read_ret != 1) {
        const char* acc_id_text =
            NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                           0);
        cMyTrace trace(
            "virtual int Dispatcher_ChangeCharacName::dispatch_sig(CUser*, PacketBuf&)",
            40647,
            0);
        trace("ting : m_id(%s) src char name length(%d), ret(%d)",
              acc_id_text,
              src_name_len,
              read_ret);
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                 7);
        return 0;
    }
    if (packet.get_str(src_name, sizeof(src_name), src_name_len) != 1) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                 7);
        return 0;
    }

    read_ret = packet.get_int(dst_name_len);
    if (read_ret != 1) {
        const char* acc_id_text =
            NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                           0);
        cMyTrace trace(
            "virtual int Dispatcher_ChangeCharacName::dispatch_sig(CUser*, PacketBuf&)",
            40660,
            0);
        trace("ting : m_id(%s) char name length(%d), ret(%d)",
              acc_id_text,
              dst_name_len,
              read_ret);
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                 7);
        return 0;
    }
    if (packet.get_str(dst_name, sizeof(dst_name), dst_name_len) != 1) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                 7);
        return 0;
    }

    if (dst_name_len <= 3 || dst_name_len > 18) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                 1);
        return 0;
    }

    const char* server_src_name =
        user->get_charac_name( static_cast<int>(charac_slot));
    if (!server_src_name || std::strncmp(src_name, server_src_name, 0x1Du)) {
        if (server_src_name) {
            cMyTrace trace(
                "virtual int Dispatcher_ChangeCharacName::dispatch_sig(CUser*, PacketBuf&)",
                40696,
                0);
            trace("ting : src char name(%s), src server char name(%s)",
                  src_name,
                  server_src_name);
        } else {
            const char* acc_id_text = NumberToString(
                static_cast<unsigned int>(user->get_acc_id()),
                0);
            cMyTrace trace(
                "virtual int Dispatcher_ChangeCharacName::dispatch_sig(CUser*, PacketBuf&)",
                40694,
                0);
            trace("ting : m_id(%s) srcCharacName2 is null", acc_id_text);
        }
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                 7);
        return 0;
    }

    unsigned char request_flag = 0;
    if (packet.get_byte(request_flag) != 1) {
        return LineFunc(
            40703,
            "virtual int Dispatcher_ChangeCharacName::dispatch_sig(CUser*, PacketBuf&)",
            0, 0);
    }

    int rename_request = 0;
    if (request_flag == 2) {
        if (G_CDataManager()->hasPreventPrefixCharacName(src_name,
                                                        src_name_len)) {
            rename_request = 2;
        } else {
            user->SendCmdErrorPacket(
                                     ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                     7);
            return 0;
        }
    } else if (G_CDataManager()->hasSpecialCharacter(server_src_name)) {
        rename_request = 0;
    } else if (G_CDataManager()->hasPreventString(server_src_name) ||
               G_CDataManager()->hasPreventStringName(server_src_name)) {
        rename_request = 1;
    } else {
        if (user->CheckPossibleRequestRenameState(
                static_cast<int>(charac_slot)) != true) {
            user->SendCmdErrorPacket(
                                     ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                     7);
            return 0;
        }
        if (!checkValidCharacName(dst_name)) {
            user->SendCmdErrorPacket(
                                     ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                     159);
            return 0;
        }
        user->SetCharacViewState( static_cast<int>(charac_slot), 1);
        rename_request = 3;
    }

    if (restrict_inputting_name::isUtf8StrInUnicodeRange(dst_name) != 1 ||
        !checkValidCharacName(dst_name)) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_CHANGE_CHARAC_NAME,
                                 159);
        return 0;
    }

    DB_ReqChangeCharacName::makeRequest(
        user->GetUID(),
        user->get_charac_no( static_cast<int>(charac_slot)),
        dst_name,
        rename_request);
    return 0;
}

// IDA_EA: 0x823388A
// IDA_NAME: _ZN27Dispatcher_ChangeCharacName4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeCharacName::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ChangeCharacName::read(PacketBuf& packet,
                                      MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233894
// IDA_NAME: _ZN27Dispatcher_ChangeCharacName7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ChangeCharacName::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ChangeCharacName::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85961A4
// IDA_NAME: _ZN27Dispatcher_ChangeCharacNameC2Ev
// DEMANGLED: Dispatcher_ChangeCharacName::Dispatcher_ChangeCharacName(void)
Dispatcher_ChangeCharacName::Dispatcher_ChangeCharacName()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8242970
// IDA_NAME: _ZN27Dispatcher_ChangeCharacNameD2Ev
// DEMANGLED: Dispatcher_ChangeCharacName::~Dispatcher_ChangeCharacName()
// IDA_EA: 0x82429A0
// IDA_NAME: _ZN27Dispatcher_ChangeCharacNameD0Ev
// DEMANGLED: Dispatcher_ChangeCharacName::~Dispatcher_ChangeCharacName()
Dispatcher_ChangeCharacName::~Dispatcher_ChangeCharacName() {}

// IDA_EA: 0x85973B6
// IDA_NAME: _ZN40Dispatcher_police_queryCharacName_useMidC2Ev
// DEMANGLED: Dispatcher_police_queryCharacName_useMid::Dispatcher_police_queryCharacName_useMid(void)
Dispatcher_police_queryCharacName_useMid::
    Dispatcher_police_queryCharacName_useMid()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240270
// IDA_NAME: _ZN40Dispatcher_police_queryCharacName_useMidD2Ev
// DEMANGLED: Dispatcher_police_queryCharacName_useMid::~Dispatcher_police_queryCharacName_useMid()
// IDA_EA: 0x82402A0
// IDA_NAME: _ZN40Dispatcher_police_queryCharacName_useMidD0Ev
// DEMANGLED: Dispatcher_police_queryCharacName_useMid::~Dispatcher_police_queryCharacName_useMid()
Dispatcher_police_queryCharacName_useMid::
    ~Dispatcher_police_queryCharacName_useMid() {}

// IDA_EA: 0x82273AE
// IDA_NAME: _ZN40Dispatcher_police_queryCharacName_useMid12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_police_queryCharacName_useMid::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_police_queryCharacName_useMid::dispatch_sig(CUser* user,
                                                            PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR() ||
        user->isGMUser() != 1) {
        return 0;
    }

    Pakcet_FindUserName_use_Charac_no monitor_packet;
    monitor_packet.requester_uid = static_cast<uint32_t>(user->GetUID());
    packet.get_int(monitor_packet.charac_no);

    CUser* const target = G_GameWorld()->find_from_world(
        static_cast<uint16_t>(monitor_packet.charac_no));
    if (target) {
        const char* const target_name = target->getCurCharacName();
        monitor_packet.charac_name_length =
            static_cast<uint32_t>(std::strlen(target_name));
        if (monitor_packet.charac_name_length >=
            sizeof(monitor_packet.charac_name)) {
            monitor_packet.charac_name_length = 0;
        }
        if (monitor_packet.charac_name_length) {
            std::strncpy(monitor_packet.charac_name,
                         target_name,
                         monitor_packet.charac_name_length);
            PacketGuard response;
            response.put_header(ENUM_PACKETCLASS_CMDPACKET, ENUM_CMDPACKET_FIND_CHAR_NAME_USE_CHARAC_NO);
            response.put_int(static_cast<int>(monitor_packet.charac_no));
            response.put_int(
                static_cast<int>(monitor_packet.charac_name_length));
            response.put_str(
                monitor_packet.charac_name,
                static_cast<int>(monitor_packet.charac_name_length));
            response.finalize(true);
            user->Send(response);
        }
    } else {
        CMonitorServerProxy* const proxy =
            GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        proxy->SendTcpPacket(reinterpret_cast<char*>(&monitor_packet), 0x34);
    }
    return 0;
}

// IDA_EA: 0x82342B2
// IDA_NAME: _ZN40Dispatcher_police_queryCharacName_useMid4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_police_queryCharacName_useMid::read(PacketBuf &,MSG_BASE &)
int Dispatcher_police_queryCharacName_useMid::read(PacketBuf& packet,
                                                   MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82342BC
// IDA_NAME: _ZN40Dispatcher_police_queryCharacName_useMid7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_police_queryCharacName_useMid::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_police_queryCharacName_useMid::process(CUser* user,
                                                      MSG_BASE& message,
                                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// The two state-3 template families have one canonical object/vtable owner:
// this translation unit.  They remain intentionally unregistered until the
// original PacketDispatcher constructor and 606-slot lifecycle are restored.
// IDA_EA: 0x82504C0
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_EQUIPSLOT_SWITCH17stEquipslotSwitchL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_EQUIPSLOT_SWITCH,stEquipslotSwitch,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x8250610
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_EQUIPSLOT_SWITCH17stEquipslotSwitchL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_EQUIPSLOT_SWITCH,stEquipslotSwitch,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x825061A
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_EQUIPSLOT_SWITCH17stEquipslotSwitchL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_EQUIPSLOT_SWITCH,stEquipslotSwitch,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825C250
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_EQUIPSLOT_SWITCH17stEquipslotSwitchL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_EQUIPSLOT_SWITCH,stEquipslotSwitch,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C280
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_EQUIPSLOT_SWITCH17stEquipslotSwitchL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_EQUIPSLOT_SWITCH,stEquipslotSwitch,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D72E
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_EQUIPSLOT_SWITCH17stEquipslotSwitchL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_EQUIPSLOT_SWITCH,stEquipslotSwitch,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598C1A
// IDA_NAME: _ZN17IPacketDispatcherI20MSG_EQUIPSLOT_SWITCH17stEquipslotSwitchL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_EQUIPSLOT_SWITCH,stEquipslotSwitch,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_EQUIPSLOT_SWITCH,
                                 stEquipslotSwitch,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x82506AE
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE27stExpandEquipslotFlagUpdateL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE,stExpandEquipslotFlagUpdate,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x82507FE
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE27stExpandEquipslotFlagUpdateL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE,stExpandEquipslotFlagUpdate,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x8250808
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE27stExpandEquipslotFlagUpdateL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE,stExpandEquipslotFlagUpdate,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825C29E
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE27stExpandEquipslotFlagUpdateL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE,stExpandEquipslotFlagUpdate,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C2CE
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE27stExpandEquipslotFlagUpdateL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE,stExpandEquipslotFlagUpdate,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D734
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE27stExpandEquipslotFlagUpdateL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE,stExpandEquipslotFlagUpdate,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598C36
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE27stExpandEquipslotFlagUpdateL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE,stExpandEquipslotFlagUpdate,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_EXPAND_EQUIPSLOT_FLAG_UPDATE,
                                 stExpandEquipslotFlagUpdate,
                                 static_cast<ch_state>(3)>;
