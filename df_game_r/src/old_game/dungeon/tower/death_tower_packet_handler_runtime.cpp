#include "old_game/dungeon/tower/death_tower_packet_handler_runtime.h"

#include "infra/network/packet/PacketEnum.h"

#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/player/core/cuser.h"

namespace WongWork {

// IDA_EA: 0x845FC6C
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler20makePickupItemHeaderEtt
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makePickupItemHeader(
//     unsigned short,unsigned short)
int CDeathTower::CPacketHandler::makePickupItemHeader(
    unsigned short picker_unique_id,
    unsigned short drop_unique_id) {
    put_header(0, ENUM_NOTIPACKET_GET_ITEM);
    put_short(drop_unique_id);
    return put_short(picker_unique_id);
}

// IDA_EA: 0x845FCC4
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler17appendPickupMoneyEPiS2_P6CParty
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::appendPickupMoney(
//     int *,int *,CParty *)
int CDeathTower::CPacketHandler::appendPickupMoney(int* earned_money,
                                                   int* routed_money,
                                                   CParty* party) {
    for (int slot = 0; slot <= 3; ++slot) {
        if (!party->get_user(slot) || party->checkValidUser(slot) != 1) {
            put_byte(0);
            put_int(0);
        } else {
            put_byte(1);
            put_int(earned_money[slot]);
            put_byte(1);
            put_int(routed_money[slot]);
            // CLIENT_IDA_EVIDENCE: sub_5A4A20@0x5A517C/0x5A527C
            // reads one trailing u32 whenever this count is nonzero.  The
            // routed amount above is the sole counted component. DeathTower
            // has no producer for the trailing guild-fund component.
            put_int(0);
        }
    }
    for (int slot = 4; slot < 8; ++slot) {
        put_byte(0);
        put_int(0);
    }
    return finalize(true);
}

// IDA_EA: 0x845FDC0
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler16appendPickupItemEPctii
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::appendPickupItem(
//     char *,unsigned short,int,int)
int CDeathTower::CPacketHandler::appendPickupItem(char* picked_by,
                                                  unsigned short winner_unique_id,
                                                  int inserted_slot,
                                                  int pickup_category) {
    for (int slot = 0; slot <= 3; ++slot) {
        put_byte(picked_by[slot]);
    }
    for (int slot = 4; slot < 8; ++slot) {
        put_byte(0);
    }
    put_short(winner_unique_id);
    put_short(inserted_slot);
    put_byte(pickup_category);
    return finalize(true);
}

// IDA_EA: 0x845FEA6
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler17makeDropItemErrorE14ENUM_CMDPACKET10ENUM_ERRORc
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makeDropItemError(
//     ENUM_CMDPACKET,ENUM_ERROR,char)
int CDeathTower::CPacketHandler::makeDropItemError(ENUM_CMDPACKET command,
                                                    ENUM_ERROR error,
                                                    char client_rarity) {
    put_header(1, static_cast<int>(command));
    put_byte(0);
    put_byte(static_cast<int>(error));
    put_byte(client_rarity);
    return finalize(true);
}

// IDA_EA: 0x845FF1A
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler18makeDropItemNotifyEtiitRK8map_item
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makeDropItemNotify(
//     unsigned short,int,int,unsigned short,map_item const&)
int CDeathTower::CPacketHandler::makeDropItemNotify(
    unsigned short picker_unique_id,
    int x,
    int y,
    unsigned short unique_id,
    const map_item& item) {
    put_header(0, ENUM_NOTIPACKET_DROP_ITEM);
    put_short(picker_unique_id);
    put_short(x);
    put_short(y);
    put_short(unique_id);
    put_int(item.item.item_id_);
    put_byte(item.item.GetItemAttr());
    put_int(item.item.get_add_info());
    put_short(item.item.avatar_select_);
    put_int(item.item.extend_info_);
    put_byte(item.item.amplify_option_.getAbilityType());
    put_short(item.item.amplify_option_.getAbilityValue());
    put_packet(item.item);
    put_byte(item.routing_eligible_ == 0);
    return finalize(true);
}

// IDA_EA: 0x84600A6
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler12makeDropItemEcii
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makeDropItem(char,int,int)
int CDeathTower::CPacketHandler::makeDropItem(char client_rarity,
                                              int inventory_slot,
                                              int item_id) {
    put_header(1, ENUM_CMDPACKET_DROP_ITEM);
    put_byte(1);
    put_byte(client_rarity);
    put_short(inventory_slot);
    put_int(item_id);
    return finalize(true);
}

// IDA_EA: 0x846012C
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler17makeFinishLoadingEv
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makeFinishLoading(void)
int CDeathTower::CPacketHandler::makeFinishLoading() {
    put_header(0, ENUM_NOTIPACKET_FINISH_LOADING);
    put_int(0);
    put_byte(0);
    return finalize(true);
}

// IDA_EA: 0x84601DA
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler19makeDisplayRangkingEjjjjbjj
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makeDisplayRangking(
//     unsigned int,unsigned int,unsigned int,unsigned int,bool,unsigned int,
//     unsigned int)
int CDeathTower::CPacketHandler::makeDisplayRangking(
    unsigned int member_count,
    unsigned int charac_no,
    unsigned int current_stage,
    unsigned int play_time,
    bool renewed_my_record,
    unsigned int dungeon_index,
    unsigned int virtual_rank) {
    put_header(0, 144);
    put_int(play_time);
    put_int(current_stage);
    put_byte(renewed_my_record);
    put_int(virtual_rank);

    const unsigned int rank_index = static_cast<unsigned int>(
        CDeathTowerRanking::makeDungeonIdx2TowerIdx(dungeon_index));
    CDeathTowerRanking* const rank_table =
        CDeathTowerRanking::getRankTable(member_count);
    rank_table->makeMyBestRecordPacket(rank_index, charac_no, *this);
    rank_table->makeTopRankerRecordPacket(rank_index, *this);
    return finalize(true);
}

namespace {

void appendRewardItems(PacketGuard& packet,
                       int count,
                       const Inven_Item* items) {
    packet.put_byte(count);
    for (int index = 0; index < count; ++index) {
        packet.put_int(items[index].item_id_);
        packet.put_int(items[index].get_add_info());
    }
}

}  // namespace

// IDA_EA: 0x84602C4
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler17makeDisplayRewardEiiPK10Inven_ItemiS4_iS4_iS4_
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makeDisplayReward(
//     int,int,Inven_Item const*,int,Inven_Item const*,int,Inven_Item const*,
//     int,Inven_Item const*)
int CDeathTower::CPacketHandler::makeDisplayReward(
    int reward_exp_0,
    int reward_count_0,
    const Inven_Item* reward_items_0,
    int reward_count_1,
    const Inven_Item* reward_items_1,
    int reward_count_2,
    const Inven_Item* reward_items_2,
    int reward_count_3,
    const Inven_Item* reward_items_3) {
    put_header(0, 145);
    put_int(reward_exp_0);
    appendRewardItems(*this, reward_count_0, reward_items_0);
    appendRewardItems(*this, reward_count_1, reward_items_1);
    appendRewardItems(*this, reward_count_2, reward_items_2);
    appendRewardItems(*this, reward_count_3, reward_items_3);
    return finalize(true);
}

// IDA_EA: 0x84604B0
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler15makeDisplayEPLPEb
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makeDisplayEPLP(bool)
int CDeathTower::CPacketHandler::makeDisplayEPLP(bool selected) {
    put_header(0, 146);
    put_byte(selected);
    return finalize(true);
}

// IDA_EA: 0x846050E
// IDA_NAME: _ZN8WongWork11CDeathTower14CPacketHandler18makeUpdateItemListEP5CUser14ENUM_ITEMSPACEi
// DEMANGLED: WongWork::CDeathTower::CPacketHandler::makeUpdateItemList(
//     CUser *,ENUM_ITEMSPACE,int)
int CDeathTower::CPacketHandler::makeUpdateItemList(CUser* user,
                                                    ENUM_ITEMSPACE item_space,
                                                    int slot) {
    put_header(0, 14);
    put_byte(static_cast<int>(item_space));
    put_short(1);
    user->getCurCharacInvenR()->MakeItemPacket(
        static_cast<INVEN_TYPE>(GetInvenTypeFromItemSpace(item_space)),
        slot,
        *this);
    return finalize(true);
}

}  // namespace WongWork
