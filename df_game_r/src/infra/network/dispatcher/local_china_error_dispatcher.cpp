#include "infra/network/dispatcher/local_china_error_dispatcher.h"

#include "infra/network/packet/PacketEnum.h"

#include <utility>


// IDA_EA: 0x812B14C
// IDA_NAME: _ZN26CLocalChinaErrorDispatcher17_PacketErrorCheckC2Ebbbb
// DEMANGLED: CLocalChinaErrorDispatcher::_PacketErrorCheck::_PacketErrorCheck(bool,bool,bool,bool)
CLocalChinaErrorDispatcher::_PacketErrorCheck::_PacketErrorCheck(
    bool anti_addiction_value,
    bool safe_mode_user_value,
    bool safe_mode_after_login_value,
    bool lock_in_seria_room_value)
    : anti_addiction(anti_addiction_value),
      safe_mode_user(safe_mode_user_value),
      safe_mode_after_login(safe_mode_after_login_value),
      lock_in_seria_room(lock_in_seria_room_value) {}

// IDA_EA: 0x8128C34
// IDA_NAME: _ZN26CLocalChinaErrorDispatcherC2Ev
// DEMANGLED: CLocalChinaErrorDispatcher::CLocalChinaErrorDispatcher(void)
CLocalChinaErrorDispatcher::CLocalChinaErrorDispatcher() {
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_DROP_ITEM,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_COMPOUND_ITEM,
        _PacketErrorCheck(0, 0, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_COMPOUND_AVATAR,
        _PacketErrorCheck(0, 0, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_DELETE_CHARACTER,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_RECOVER_STAMINA,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_INCREASE_STATUS,
        _PacketErrorCheck(1, 0, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_SELL_ITEM,
        _PacketErrorCheck(1, 1, 1, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_SET_ITEMTRADE_STATE,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_DISJOINT_ITEM,
        _PacketErrorCheck(1, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_CALL_GUILD_CREATE_RIGHT,
        _PacketErrorCheck(0, 0, 1, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_CALL_GUILD_LEVEL_UP,
        _PacketErrorCheck(0, 0, 1, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_UPGRADE_ITEM,
        _PacketErrorCheck(1, 1, 1, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_RESET_ITEM_ATTR,
        _PacketErrorCheck(0, 0, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_BUY_PRIVATE_STORE_ITEM,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_CREATE_PRIVATE_STORE,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_MAILBOX_SEND,
        _PacketErrorCheck(0, 0, 1, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_HATCH_CREATURE,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_CHANGE_LETTER_STAT,
        _PacketErrorCheck(0, 0, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_TRY_ENCHANT,
        _PacketErrorCheck(1, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_REQ_GUILD_SECEDE,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_USE_RANDOMBOX_ITEM,
        _PacketErrorCheck(1, 0, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_AUCTION_REGIST_ITEM,
        _PacketErrorCheck(0, 1, 1, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_AUCTION_BUY_ITEM_APIECE,
        _PacketErrorCheck(0, 1, 1, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_AUCTION_REGIST_CANCEL,
        _PacketErrorCheck(0, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_AUCTION_BIDDING,
        _PacketErrorCheck(0, 1, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_GIVE_GIFT_TO_NPC,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_REQUEST_DISJOINT_ITEM,
        _PacketErrorCheck(1, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_COMPOUND_ITEM_BY_EXPERT_JOB,
        _PacketErrorCheck(1, 0, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
        _PacketErrorCheck(1, 0, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM,
        _PacketErrorCheck(1, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_MULTI_MAILBOX_SEND,
        _PacketErrorCheck(0, 1, 1, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_BUY_ITEM,
        _PacketErrorCheck(1, 0, 1, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_BUY_SKILL,
        _PacketErrorCheck(0, 0, 1, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_ENTER_PVP_ROOM,
        _PacketErrorCheck(0, 0, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_MOVE_MAP,
        _PacketErrorCheck(0, 0, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_SELECT_DUNGEON,
        _PacketErrorCheck(0, 0, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_SET_USER_AREA,
        _PacketErrorCheck(0, 0, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_ENTER_WARROOM,
        _PacketErrorCheck(0, 0, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_QUICK_JOIN_ROOM,
        _PacketErrorCheck(0, 0, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_MAILBOX_SEND,
        _PacketErrorCheck(0, 0, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_CHECK_GUILD_NAME_DOUBLE,
        _PacketErrorCheck(0, 0, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_CHECK_GUILD_ADDREASS_DOUBLE,
        _PacketErrorCheck(0, 0, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_SELL_ITEM,
        _PacketErrorCheck(0, 0, 0, 1)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_GET_ITEM,
        _PacketErrorCheck(1, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_ACCEPT_QUEST,
        _PacketErrorCheck(1, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_GIVEUP_QUEST,
        _PacketErrorCheck(1, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_SET_QUEST_TRIGGER,
        _PacketErrorCheck(1, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_FINISH_QUEST,
        _PacketErrorCheck(1, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_REPAIR_EQUIPMENT,
        _PacketErrorCheck(1, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_USE_LOTTERY_ITEM,
        _PacketErrorCheck(1, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_PUT_ITEM_FOR_ENCHANT,
        _PacketErrorCheck(1, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_CREATE_DISJOINT_STORE,
        _PacketErrorCheck(1, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_ENTER_DISJOINT_STORE,
        _PacketErrorCheck(1, 0, 0, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_BREAK_GUILD,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_COMPOUND_ITEM_BIND_SHPERE,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_USE_EMBLEM_FOR_EQUIPMENT,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_ADD_EQUIPMENT_SOCKET,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_CONVERT_EMBLEM,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_DISJOINT_AVATAR,
        _PacketErrorCheck(0, 1, 1, 0)));
    error_checks_.insert(std::make_pair(
        ENUM_CMDPACKET_COMPOUND_EMBLEM,
        _PacketErrorCheck(0, 1, 1, 0)));
}

// IDA_EA: 0x812AF6A
// IDA_NAME: _ZNK26CLocalChinaErrorDispatcher17doErrorDispatcherEP5CUsert
// DEMANGLED: CLocalChinaErrorDispatcher::doErrorDispatcher(CUser *,unsigned short)const
int CLocalChinaErrorDispatcher::doErrorDispatcher(CUser* user,
                                                   unsigned short command) const {
    if (user == 0) {
        return 0;
    }

    const ENUM_CMDPACKET packet = static_cast<ENUM_CMDPACKET>(command);
    const std::map<ENUM_CMDPACKET, _PacketErrorCheck>::const_iterator found =
        error_checks_.find(packet);
    if (found == error_checks_.end()) {
        return 0;
    }

    const _PacketErrorCheck& check = found->second;
    int result = 0;
    if (check.anti_addiction) {
        result = ProcAntiAddiction(*user, packet);
        if (result != 0) {
            return result;
        }
    }
    if (check.safe_mode_user) {
        result = ProcSafeModeUser(*user, packet);
        if (result != 0) {
            return result;
        }
    }
    if (check.safe_mode_after_login) {
        result = ProcSaefModeAfterLogin(*user, packet);
        if (result != 0) {
            return result;
        }
    }
    if (check.lock_in_seria_room) {
        result = ProcLockInSeriaRoom(*user, packet);
        if (result != 0) {
            return result;
        }
    }
    return 0;
}

// IDA_EA: 0x812B0C8
// IDA_NAME: _ZNK26CLocalChinaErrorDispatcher17ProcAntiAddictionER5CUser14ENUM_CMDPACKET
// DEMANGLED: CLocalChinaErrorDispatcher::ProcAntiAddiction(CUser &,ENUM_CMDPACKET)const
int CLocalChinaErrorDispatcher::ProcAntiAddiction(CUser& user,
                                                   ENUM_CMDPACKET command) const {
    (void)user;
    (void)command;
    return 0;
}

// IDA_EA: 0x812B0D2
// IDA_NAME: _ZNK26CLocalChinaErrorDispatcher16ProcSafeModeUserER5CUser14ENUM_CMDPACKET
// DEMANGLED: CLocalChinaErrorDispatcher::ProcSafeModeUser(CUser &,ENUM_CMDPACKET)const
int CLocalChinaErrorDispatcher::ProcSafeModeUser(CUser& user,
                                                  ENUM_CMDPACKET command) const {
    (void)user;
    (void)command;
    return 0;
}

// IDA_EA: 0x812B0DC
// IDA_NAME: _ZNK26CLocalChinaErrorDispatcher22ProcSaefModeAfterLoginER5CUser14ENUM_CMDPACKET
// DEMANGLED: CLocalChinaErrorDispatcher::ProcSaefModeAfterLogin(CUser &,ENUM_CMDPACKET)const
int CLocalChinaErrorDispatcher::ProcSaefModeAfterLogin(
    CUser& user,
    ENUM_CMDPACKET command) const {
    (void)user;
    (void)command;
    return 0;
}

// IDA_EA: 0x812B0E6
// IDA_NAME: _ZNK26CLocalChinaErrorDispatcher19ProcLockInSeriaRoomER5CUser14ENUM_CMDPACKET
// DEMANGLED: CLocalChinaErrorDispatcher::ProcLockInSeriaRoom(CUser& user,
// ENUM_CMDPACKET)const
int CLocalChinaErrorDispatcher::ProcLockInSeriaRoom(
    CUser& user,
    ENUM_CMDPACKET command) const {
    (void)user;
    (void)command;
    return 0;
}

// IDA_EA: 0x8595094
// IDA_NAME: _ZN26CLocalChinaErrorDispatcherD2Ev
// DEMANGLED: CLocalChinaErrorDispatcher::~CLocalChinaErrorDispatcher()
CLocalChinaErrorDispatcher::~CLocalChinaErrorDispatcher() {}
