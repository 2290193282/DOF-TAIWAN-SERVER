#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_DEATH_TOWER_PACKET_HANDLER_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_DEATH_TOWER_PACKET_HANDLER_RUNTIME_H_

// This is the concrete packet builder nested in CDeathTower.  It was kept
// local to the stage source during the first recovery pass, which made the
// remaining original packet makers impossible to provide from their real
// gameplay owner.  The old object is exactly a PacketGuard-derived builder;
// it has no synthetic state or separate allocation protocol.

#include "old_game/pvp/battlefield/battle_field.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// The value table is outside this recovered unit, but these packet builders
// carry the real enum identity in their exported ABI.
enum ENUM_ERROR : int;

namespace WongWork {

class CDeathTower::CPacketHandler : public PacketGuard {
public:
    // IDA_EA: 0x8469A8C / 0x8469AA0
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandlerC2Ev /
    //           _ZN8WongWork11CDeathTower14CPacketHandlerD2Ev
    CPacketHandler();
    ~CPacketHandler();

    // IDA_EA: 0x845FC6C
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler20makePickupItemHeaderEtt
    int makePickupItemHeader(unsigned short picker_unique_id,
                             unsigned short drop_unique_id);

    // IDA_EA: 0x845FCC4
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler17appendPickupMoneyEPiS2_P6CParty
    int appendPickupMoney(int* earned_money,
                          int* routed_money,
                          CParty* party);

    // IDA_EA: 0x845FDC0
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler16appendPickupItemEPctii
    int appendPickupItem(char* picked_by,
                         unsigned short winner_unique_id,
                         int inserted_slot,
                         int pickup_category);

    // IDA_EA: 0x845FE4C
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler15makeCommonErrorE14ENUM_CMDPACKET10ENUM_ERROR
    int makeCommonError(ENUM_CMDPACKET command, ENUM_ERROR error);

    // IDA_EA: 0x845FEA6
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler17makeDropItemErrorE14ENUM_CMDPACKET10ENUM_ERRORc
    int makeDropItemError(ENUM_CMDPACKET command,
                          ENUM_ERROR error,
                          char client_rarity);

    // IDA_EA: 0x845FF1A
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler18makeDropItemNotifyEtiitRK8map_item
    int makeDropItemNotify(unsigned short picker_unique_id,
                           int x,
                           int y,
                           unsigned short unique_id,
                           const map_item& item);

    // IDA_EA: 0x84600A6
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler12makeDropItemEcii
    int makeDropItem(char client_rarity, int inventory_slot, int item_id);

    // IDA_EA: 0x846012C
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler17makeFinishLoadingEv
    int makeFinishLoading();

    // IDA_EA: 0x84601DA
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler19makeDisplayRangkingEjjjjbjj
    // The original exported spelling is "Rangking".
    int makeDisplayRangking(unsigned int member_count,
                            unsigned int charac_no,
                            unsigned int current_stage,
                            unsigned int play_time,
                            bool renewed_my_record,
                            unsigned int dungeon_index,
                            unsigned int virtual_rank);

    // IDA_EA: 0x8460176
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler12makeDieStateEt
    int makeDieState(unsigned short unique_id);

    // IDA_EA: 0x84602C4
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler17makeDisplayRewardEiiPK10Inven_ItemiS4_iS4_iS4_
    int makeDisplayReward(int reward_exp_0,
                          int reward_count_0,
                          const Inven_Item* reward_items_0,
                          int reward_count_1,
                          const Inven_Item* reward_items_1,
                          int reward_count_2,
                          const Inven_Item* reward_items_2,
                          int reward_count_3,
                          const Inven_Item* reward_items_3);

    // IDA_EA: 0x84604B0
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler15makeDisplayEPLPEb
    int makeDisplayEPLP(bool selected);

    // IDA_EA: 0x846050E
    // IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler18makeUpdateItemListEP5CUser14ENUM_ITEMSPACEi
    int makeUpdateItemList(CUser* user,
                           ENUM_ITEMSPACE item_space,
                           int slot);
};

}  // namespace WongWork

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_DEATH_TOWER_PACKET_HANDLER_RUNTIME_H_
