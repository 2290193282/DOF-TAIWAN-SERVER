#include "old_game/world/region/world_packet_structs.h"

#include <cstring>


typedef char Packet_Game_Monitor_GM_Village_Attacked_Size_Must_Be_0x17[
    (sizeof(Packet_Game_Monitor_GM_Village_Attacked) == 0x17) ? 1 : -1];
typedef char Packet_VillageAttackedGMCommand_Size_Must_Be_0x1a[
    (sizeof(Packet_VillageAttackedGMCommand) == 0x1a) ? 1 : -1];
typedef char Packet_VillageMonsterFightResult_Size_Must_Be_0x2b[
    (sizeof(Packet_VillageMonsterFightResult) == 0x2b) ? 1 : -1];
typedef char NotiAdvanceAltarMapInfo_Size_Must_Be_0x14[
    (sizeof(NOTIPacketStruct::_NotiAdvanceAltarMapInfo) == 0x14) ? 1 : -1];
typedef char NotiAdvanceAltarSpawnUnit_Size_Must_Be_0x28[
    (sizeof(NOTIPacketStruct::_NotiAdvanceAltarSpawnUnit) == 0x28) ? 1 : -1];
typedef char NotiAdvanceAltarEplpState_Size_Must_Be_0x08[
    (sizeof(NOTIPacketStruct::_NotiAdvanceAltarEplpState) == 0x08) ? 1 : -1];
typedef char NotiAdvanceAltarClearStage_Size_Must_Be_0x1c[
    (sizeof(NOTIPacketStruct::_NotiAdvanceAltarClearStage) == 0x1c) ? 1 : -1];
typedef char MapMonsterSummary_Size_Must_Be_0x0d[
    (sizeof(advancealtar::_MapMonsterSummary) == 0x0d) ? 1 : -1];

// IDA_EA: 0x822C3C2
// IDA_NAME: _ZN39Packet_Game_Monitor_GM_Village_AttackedC2Ev
// DEMANGLED: Packet_Game_Monitor_GM_Village_Attacked::Packet_Game_Monitor_GM_Village_Attacked(void)
Packet_Game_Monitor_GM_Village_Attacked::
    Packet_Game_Monitor_GM_Village_Attacked()
    : PacketHeader(0x234a, 0x17) {}

// IDA_EA: 0x822C088
// IDA_NAME: _ZN31Packet_VillageAttackedGMCommandC2Ev
// DEMANGLED: Packet_VillageAttackedGMCommand::Packet_VillageAttackedGMCommand(void)
Packet_VillageAttackedGMCommand::Packet_VillageAttackedGMCommand()
    : PacketHeader(0x1779, 0x1a),
      uid(0),
      charac_no(0) {}

// IDA_EA: 0x86B4B84
// IDA_NAME: _ZN32Packet_VillageMonsterFightResultC2Ev
// DEMANGLED: Packet_VillageMonsterFightResult::Packet_VillageMonsterFightResult(void)
Packet_VillageMonsterFightResult::Packet_VillageMonsterFightResult()
    : PacketHeader(0x1772, 0x2b),
      monster_counts(),
      clear_counts(),
      result(0) {}

namespace NOTIPacketStruct {

// IDA_EA: 0x81346A6
// IDA_NAME: _ZN16NOTIPacketStruct26_NotiAdvanceAltarSpawnUnitC2Ev
// DEMANGLED: NOTIPacketStruct::_NotiAdvanceAltarSpawnUnit::_NotiAdvanceAltarSpawnUnit(void)
_NotiAdvanceAltarSpawnUnit::_NotiAdvanceAltarSpawnUnit() {
    std::memset(this, 0, 0x28u);
}

// IDA_EA: 0x81346FC
// IDA_NAME: _ZN16NOTIPacketStruct24_NotiAdvanceAltarMapInfoC2Ev
// DEMANGLED: NOTIPacketStruct::_NotiAdvanceAltarMapInfo::_NotiAdvanceAltarMapInfo(void)
_NotiAdvanceAltarMapInfo::_NotiAdvanceAltarMapInfo() {
    std::memset(this, 0, 0x14u);
}

// IDA_EA: 0x8134752
// IDA_NAME: _ZN16NOTIPacketStruct26_NotiAdvanceAltarEplpStateC2Ev
// DEMANGLED: NOTIPacketStruct::_NotiAdvanceAltarEplpState::_NotiAdvanceAltarEplpState(void)
_NotiAdvanceAltarEplpState::_NotiAdvanceAltarEplpState() {
    std::memset(this, 0, 0x08u);
}

// IDA_EA: 0x8134720
// IDA_NAME: _ZN16NOTIPacketStruct27_NotiAdvanceAltarClearStageC2Ev
// DEMANGLED: NOTIPacketStruct::_NotiAdvanceAltarClearStage::_NotiAdvanceAltarClearStage(void)
_NotiAdvanceAltarClearStage::_NotiAdvanceAltarClearStage() {
    std::memset(this, 0, 0x1cu);
}

}  // namespace NOTIPacketStruct

namespace advancealtar {

// IDA_EA: 0x813442E
// IDA_NAME: _ZN12advancealtar18_MapMonsterSummaryC2Ev
// DEMANGLED: advancealtar::_MapMonsterSummary::_MapMonsterSummary(void)
_MapMonsterSummary::_MapMonsterSummary() {
    std::memset(this, 0, 0x0du);
}

}  // namespace advancealtar
