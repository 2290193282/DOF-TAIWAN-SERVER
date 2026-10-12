#include "old_game/appearance/pet/creature.h"

#include "infra/log/format/trace.h"
#include "infra/network/packet/PacketEnum.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"

namespace user_creature {

// IDA_EA: 0x8340256
// IDA_NAME: _ZN13user_creature17CPacketRespondent10IsGateAreaEP5CUseri
// DEMANGLED: user_creature::CPacketRespondent::IsGateArea(CUser *,int)
bool CPacketRespondent::IsGateArea(CUser* user, int area_index) {
    GameWorld* const world = G_GameWorld();
    if (!world) {
        cMyTrace trace(
            "bool user_creature::CPacketRespondent::IsGateArea(CUser*, int)",
            641,
            5);
        trace("G_GameWorld call failed");
        return false;
    }

    Village* const village = world->getUserVillage(user);
    if (village) {
        return village->get_gate_area() == area_index;
    }

    cMyTrace trace("bool user_creature::CPacketRespondent::IsGateArea(CUser*, int)",
                   647,
                   5);
    trace("getUserVillage call failed");
    return false;
}

// IDA_EA: 0x8340404
// IDA_NAME: _ZN13user_creature17CPacketRespondent9BroadcastEii
// DEMANGLED: user_creature::CPacketRespondent::Broadcast(int,int)
int CPacketRespondent::Broadcast(int village_index, int area_index) {
    send_on_destroy_ = false;
    if (!village_index && !area_index) {
        return 0;
    }
    if (IsGateArea(user_, area_index)) {
        return user_->Send(*this);
    }
    G_GameWorld()->send_to_area(village_index, area_index, *this);
    return 1;
}

// IDA_EA: 0x834097E
// IDA_NAME: _ZN13user_creature17CPacketRespondent30MakeNotipacketResponseCreatureEv
// DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketResponseCreature(void)
int CPacketRespondent::MakeNotipacketResponseCreature() {
    clear();
    put_header(0, 104);
    put_short(user_->get_unique_id());
    return finalize(true);
}

// IDA_EA: 0x8340538
// IDA_NAME: _ZN13user_creature17CPacketRespondent8SetErrorEi
// DEMANGLED: user_creature::CPacketRespondent::SetError(int)
int CPacketRespondent::SetError(int error) {
    clear();
    put_header(packet_class_, packet_id_);
    put_byte(0);
    put_byte(error);
    return finalize(true);
}

// IDA_EA: 0x83405A2
// IDA_NAME: _ZN13user_creature17CPacketRespondent5SetOkEv
// DEMANGLED: user_creature::CPacketRespondent::SetOk(void)
int CPacketRespondent::SetOk() {
    clear();
    put_header(packet_class_, packet_id_);
    put_byte(1);
    return finalize(true);
}

// IDA_EA: 0x83405FA
// IDA_NAME: _ZN13user_creature17CPacketRespondent27MakeCmdpacketRenameCreatureEii
// DEMANGLED: user_creature::CPacketRespondent::MakeCmdpacketRenameCreature(int,int)
int CPacketRespondent::MakeCmdpacketRenameCreature(int slot, int rename_mode) {
    clear();
    put_header(1, ENUM_CMDPACKET_RENAME_CREATURE);
    put_byte(1);
    put_short(slot);
    put_byte(rename_mode);
    return finalize(true);
}

// IDA_EA: 0x8340672
// IDA_NAME: _ZN13user_creature17CPacketRespondent28MakeNotipacketRenameCreatureEPKci
// DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketRenameCreature(char const*,int)
int CPacketRespondent::MakeNotipacketRenameCreature(const char* name,
                                                     int name_length) {
    clear();
    put_header(0, 101);
    put_short(user_->get_unique_id());
    put_int(name_length);
    put_str(name, name_length);
    return finalize(true);
}

// IDA_EA: 0x833A6D8
// IDA_NAME: _ZN13user_creature12CCreatureMgr16ResponseCreatureEv
// DEMANGLED: user_creature::CCreatureMgr::ResponseCreature(void)
int CCreatureMgr::ResponseCreature() {
    CPacketRespondent packet(owner_, 0, 0);
    packet.MakeNotipacketResponseCreature();
    return packet.Broadcast(owner_->getCurCharacVill(), owner_->get_area(false));
}

}  // namespace user_creature
