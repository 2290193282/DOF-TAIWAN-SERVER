#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/dispatcher/packet_register_gm_mid.h"
#include "common/manager/system_time.h"
#include "infra/log/format/trace.h"

#include <cstring>
#include <stdint.h>


extern const char* GetPacketName(int packet_class, unsigned short packet_id);
extern int OldSrv_CUser_AntibotSend_wrap_encrypt_required_as_NOTI_DPROTO(
    CUser* user, PacketBuf* packet)
    __asm__("OldSrv_CUser_AntibotSend_wrap_encrypt_required_as_NOTI_DPROTO");
// IDA_EA: 0x8110DD0
// IDA_NAME: _ZN5CUser26incPcRoomPlayTimeRewardCntEv
// DEMANGLED: CUser::incPcRoomPlayTimeRewardCnt(void)
CUser* CUser::incPcRoomPlayTimeRewardCnt() {
    ++pc_room_reward_count_;
    return this;
}

// IDA_EA: 0x8110DEC
// IDA_NAME: _ZNK5CUser26getPcRoomPlayTimeRewardCntEv
// DEMANGLED: CUser::getPcRoomPlayTimeRewardCnt(void)const
unsigned char CUser::getPcRoomPlayTimeRewardCnt() const {
    return pc_room_reward_count_;
}

// IDA_EA: 0x8110DFE
// IDA_NAME: _ZN5CUser24resetTotalPcRoomPlayTimeEv
// DEMANGLED: CUser::resetTotalPcRoomPlayTime(void)
CUser* CUser::resetTotalPcRoomPlayTime() {
    pc_room_play_time_ = 0;
    return this;
}

// IDA_EA: 0x8110E10
// IDA_NAME: _ZNK5CUser22getTotalPcRoomPlayTimeEv
// DEMANGLED: CUser::getTotalPcRoomPlayTime(void)const
unsigned int CUser::getTotalPcRoomPlayTime() const {
    return pc_room_play_time_;
}

// IDA_EA: 0x8110E1E
// IDA_NAME: _ZN5CUser31IncCurCharacUsedFatigueQuantityEj
// DEMANGLED: CUser::IncCurCharacUsedFatigueQuantity(unsigned int)
CUser* CUser::IncCurCharacUsedFatigueQuantity(unsigned int amount) {
    used_fatigue_ += amount;
    return this;
}

// IDA_EA: 0x8110E3A
// IDA_NAME: _ZN5CUser35IncCurCharacUsedGiftFatigueQuantityEj
// DEMANGLED: CUser::IncCurCharacUsedGiftFatigueQuantity(unsigned int)
int CUser::IncCurCharacUsedGiftFatigueQuantity(unsigned int amount) {
    used_gift_fatigue_ += amount;
    return reinterpret_cast<int>(SetChangedGiftFatigueQuantity(true));
}

// IDA_EA: 0x8110E6C
// IDA_NAME: _ZNK5CUser35GetCurCharacUsedGiftFatigueQuantityEv
// DEMANGLED: CUser::GetCurCharacUsedGiftFatigueQuantity(void)const
unsigned int CUser::GetCurCharacUsedGiftFatigueQuantity() const {
    return used_gift_fatigue_;
}

// IDA_EA: 0x8110E7A
// IDA_NAME: _ZN5CUser29SetChangedGiftFatigueQuantityEb
// DEMANGLED: CUser::SetChangedGiftFatigueQuantity(bool)
CUser* CUser::SetChangedGiftFatigueQuantity(bool changed) {
    gift_fatigue_changed_ = static_cast<uint8_t>(changed);
    return this;
}

// IDA_EA: 0x84EC86C
// IDA_NAME: _ZN5CUser20getDungeonGainedGoldEv
// DEMANGLED: CUser::getDungeonGainedGold(void)
void* CUser::getDungeonGainedGold() {
    return &dungeon_gained_gold_;
}

// IDA_EA: 0x84EC8F8
// IDA_NAME: _ZN5CUser22SetAccountLastPlayTimeEl
// DEMANGLED: CUser::SetAccountLastPlayTime(long)
CUser* CUser::SetAccountLastPlayTime(long current_second) {
    account_last_play_time_ = static_cast<uint32_t>(current_second);
    return this;
}

// IDA_EA: 0x84ECAD4
// IDA_NAME: _ZN5CUser19GetTutorialSkipableEv
// DEMANGLED: CUser::GetTutorialSkipable(void)
unsigned char CUser::GetTutorialSkipable() {
    return tutorial_skipable_;
}

// IDA_EA: 0x84ECAE4
// IDA_NAME: _ZN5CUser19SetTutorialSkipableEh
// DEMANGLED: CUser::SetTutorialSkipable(unsigned char)
CUser* CUser::SetTutorialSkipable(unsigned char value) {
    tutorial_skipable_ = value;
    return this;
}

// IDA_EA: 0x8568E84
// IDA_NAME: _ZN5CUser9GetRatingEv
// DEMANGLED: CUser::GetRating(void)
float CUser::GetRating() {
    float rating = 0.0f;
    std::memcpy(&rating, &character_statistic_.rating_storage_,
                sizeof(rating));
    return rating;
}

// IDA_EA: 0x868F7BA
// IDA_NAME: _ZN5CUser21collectPcRoomPlayTimeEv
// DEMANGLED: CUser::collectPcRoomPlayTime(void)
CUser* CUser::collectPcRoomPlayTime() {
    const unsigned int current_second = static_cast<unsigned int>(
        GlobalData::system_time()->getCurSec());
    pc_room_play_time_ += current_second - pc_room_play_start_time_;
    pc_room_play_start_time_ = current_second;
    return this;
}

// IDA_EA: 0x868F808
// IDA_NAME: _ZN5CUser21setPcRoomPlayTimeDataEjjh
// DEMANGLED: CUser::setPcRoomPlayTimeData(unsigned int,unsigned int,unsigned char)
CUser* CUser::setPcRoomPlayTimeData(unsigned int start_time,
                                    unsigned int total_time,
                                    unsigned char reward_count) {
    pc_room_play_start_time_ = start_time;
    pc_room_play_time_ = total_time;
    pc_room_reward_count_ = reward_count;
    return this;
}

// IDA_EA: 0x868F8D8
// IDA_NAME: _ZN5CUser21resetInformNoticeFlagEv
// DEMANGLED: CUser::resetInformNoticeFlag(void)
void* CUser::resetInformNoticeFlag() {
    // The original 0x868F8D8 memset length is one byte, not the complete
    // sixteen-byte bank.  Preserve that precise reset contract.
    inform_notice_.flags[0] = 0;
    return inform_notice_.flags;
}

// IDA_EA: 0x86912B0
// IDA_NAME: _ZN5CUser12getBingoDataEv
// DEMANGLED: CUser::getBingoData(void)
BingoData* CUser::getBingoData() {
    return &bingo_data_;
}

// IDA_EA: 0x869736E
// IDA_NAME: _ZN5CUser15getLinkCharacNoEc
// DEMANGLED: CUser::getLinkCharacNo(char)
unsigned int CUser::getLinkCharacNo(char slot) {
    const int slot_index = static_cast<signed char>(slot);
    if (slot_index >= get_charac_count() || slot_index < 0) {
        return 0;
    }
    return static_cast<unsigned int>(get_charac_no(slot_index));
}

// IDA_EA: 0x86975CE
// IDA_NAME: _ZN5CUser33ResetCurCharacUsedFatigueQuantityEv
// DEMANGLED: CUser::ResetCurCharacUsedFatigueQuantity(void)
CUser* CUser::ResetCurCharacUsedFatigueQuantity() {
    used_fatigue_ = 0;
    return this;
}

// IDA_EA: 0x85BFD1A
// IDA_NAME: _ZN5CUser24setBreakAwayDungeonClearEb
// DEMANGLED: CUser::setBreakAwayDungeonClear(bool)
CUser* CUser::setBreakAwayDungeonClear(bool value) {
    break_away_dungeon_clear_ = static_cast<uint8_t>(value);
    return this;
}

// IDA_EA: 0x822FC6E
// IDA_NAME: _ZN5CUser27getBreakAwayDungeonClearCntEv
// DEMANGLED: CUser::getBreakAwayDungeonClearCnt(void)
unsigned int CUser::getBreakAwayDungeonClearCnt() {
    return static_cast<unsigned int>(break_away_dungeon_clear_count_);
}

// IDA_EA: 0x86971EE
// IDA_NAME: _ZN5CUser25setUsedBreakAwayLuckPointEb
// DEMANGLED: CUser::setUsedBreakAwayLuckPoint(bool)
CUser* CUser::setUsedBreakAwayLuckPoint(bool value) {
    used_break_away_luck_point_ = static_cast<uint8_t>(value);
    return this;
}

// IDA_EA: 0x80DA38C
// IDA_NAME: _ZN5CUser9get_stateEv
// DEMANGLED: CUser::get_state(void)
int CUser::get_state() {
    return static_cast<int>(state_);
}

// IDA_EA: 0x84EC99C
// IDA_NAME: _ZN5CUser14setHangameUserEb
// DEMANGLED: CUser::setHangameUser(bool)
CUser* CUser::setHangameUser(bool hangame) {
    hangame_user_ = static_cast<uint8_t>(hangame);
    return this;
}

// IDA_EA: 0x84EC9B8
// IDA_NAME: _ZN5CUser13isHangameUserEv
// DEMANGLED: CUser::isHangameUser(void)
int CUser::isHangameUser() {
    return hangame_user_;
}

// IDA_EA: 0x86796DC
// IDA_NAME: _ZN5CUser17SetGameMasterModeEb
// DEMANGLED: CUser::SetGameMasterMode(bool)
int CUser::SetGameMasterMode(bool enabled) {
    game_master_mode_ = static_cast<uint8_t>(enabled);

    Packet_Register_GM_MID packet;
    if (enabled) {
        packet.charac_no = this->getCurCharacNo();
        CMonitorServerProxy* const proxy =
            GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(GetServerGroup()));
        proxy->SendTcpPacket(reinterpret_cast<char*>(&packet),
            static_cast<int>(packet.size));
    }
    return 1;
}

// IDA_EA: 0x833F4BC
// IDA_NAME: _ZN5CUser22get_cera_item_order_noEv
// DEMANGLED: CUser::get_cera_item_order_no(void)
int CUser::get_cera_item_order_no() {
    return static_cast<int>(cera_item_order_no_);
}

// IDA_EA: 0x867CB90
// IDA_NAME: _ZN5CUser23getDungeonIdxAfterClearEv
// DEMANGLED: CUser::getDungeonIdxAfterClear(void)
int CUser::getDungeonIdxAfterClear() {
    CParty* const party = GetParty();
    if (!party) {
        return -1;
    }
    return party->battle_field().get_dungeon_index();
}

// IDA_EA: 0x86485BA
// IDA_NAME: _ZN5CUser4SendER11PacketGuard
// DEMANGLED: CUser::Send(PacketGuard &)
int CUser::Send(PacketGuard& packet) {
    PacketBuf* const packet_buf = packet.get();
    if (packet_buf->is_finallized()) {
        const SC_Header* const sc_header =
            reinterpret_cast<const SC_Header*>(packet_buf->get_packet(0));
        const unsigned short packet_id = sc_header->type;
        const unsigned char packet_class = sc_header->cmd;
        (void)GetPacketName(packet_class, packet_id);
        if (static_cast<unsigned char>(sc_header->type) !=
            static_cast<unsigned char>(ENUM_NOTIPACKET_USER_LEAVE)) {
            last_packet_identity_.value =
                (last_packet_identity_.value & 0xffff0000u) |
                static_cast<uint32_t>(
                    static_cast<unsigned char>(sc_header->type)) |
                (static_cast<uint32_t>(sc_header->cmd) << 8);
        }
        return OldSrv_CUser_AntibotSend_wrap_encrypt_required_as_NOTI_DPROTO(
            this, packet_buf);
    }

    const SC_Header* const sc_header =
        reinterpret_cast<const SC_Header*>(packet_buf->get_packet(0));
    const unsigned short packet_id = sc_header->type;
    LogManager::logFormat(
        1,
        "user.cpp",
        "bool CUser::Send(PacketGuard&)",
        0x8A0,
        "User %s - Packet.is_finallized packet_id(%d) return false",
        this->getCurCharacName(),
        packet_id);
    return 0;
}
