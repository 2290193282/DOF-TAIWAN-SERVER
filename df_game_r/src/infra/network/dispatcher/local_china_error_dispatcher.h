#ifndef OLDSRV_PROTOCOL_LOCAL_CHINA_ERROR_DISPATCHER_H_
#define OLDSRV_PROTOCOL_LOCAL_CHINA_ERROR_DISPATCHER_H_

#include <map>

#include "infra/network/packet/PacketEnum.h"


class CUser;

// IDA unit: 13_database_dispatch/CLocalChinaErrorDispatcher.
//
// The old object is a command-to-policy map.  The four policy bits are named
// from the only four call sites that consume them in doErrorDispatcher().
class CLocalChinaErrorDispatcher {
public:
    struct _PacketErrorCheck {
        // IDA_EA: 0x812B14C
        // IDA_NAME: _ZN26CLocalChinaErrorDispatcher17_PacketErrorCheckC2Ebbbb
        // DEMANGLED: CLocalChinaErrorDispatcher::_PacketErrorCheck::_PacketErrorCheck(bool,bool,bool,bool)
        _PacketErrorCheck(bool anti_addiction,
                          bool safe_mode_user,
                          bool safe_mode_after_login,
                          bool lock_in_seria_room);

        bool anti_addiction;
        bool safe_mode_user;
        bool safe_mode_after_login;
        bool lock_in_seria_room;
    };

    // IDA_EA: 0x8128C34
    // IDA_NAME: _ZN26CLocalChinaErrorDispatcherC2Ev
    // DEMANGLED: CLocalChinaErrorDispatcher::CLocalChinaErrorDispatcher(void)
    CLocalChinaErrorDispatcher();

    // IDA_EA: 0x8595094
    // IDA_NAME: _ZN26CLocalChinaErrorDispatcherD2Ev
    // DEMANGLED: CLocalChinaErrorDispatcher::~CLocalChinaErrorDispatcher()
    ~CLocalChinaErrorDispatcher();

    // IDA_EA: 0x812AF6A
    // IDA_NAME: _ZNK26CLocalChinaErrorDispatcher17doErrorDispatcherEP5CUsert
    // DEMANGLED: CLocalChinaErrorDispatcher::doErrorDispatcher(CUser *,unsigned short)const
    int doErrorDispatcher(CUser* user, unsigned short command) const;

private:
    // IDA_EA: 0x812B0C8
    // IDA_NAME: _ZNK26CLocalChinaErrorDispatcher17ProcAntiAddictionER5CUser14ENUM_CMDPACKET
    // DEMANGLED: CLocalChinaErrorDispatcher::ProcAntiAddiction(CUser &,ENUM_CMDPACKET)const
    int ProcAntiAddiction(CUser& user, ENUM_CMDPACKET command) const;

    // IDA_EA: 0x812B0D2
    // IDA_NAME: _ZNK26CLocalChinaErrorDispatcher16ProcSafeModeUserER5CUser14ENUM_CMDPACKET
    // DEMANGLED: CLocalChinaErrorDispatcher::ProcSafeModeUser(CUser &,ENUM_CMDPACKET)const
    int ProcSafeModeUser(CUser& user, ENUM_CMDPACKET command) const;

    // IDA_EA: 0x812B0DC
    // IDA_NAME: _ZNK26CLocalChinaErrorDispatcher22ProcSaefModeAfterLoginER5CUser14ENUM_CMDPACKET
    // DEMANGLED: CLocalChinaErrorDispatcher::ProcSaefModeAfterLogin(CUser &,ENUM_CMDPACKET)const
    int ProcSaefModeAfterLogin(CUser& user, ENUM_CMDPACKET command) const;

    // IDA_EA: 0x812B0E6
    // IDA_NAME: _ZNK26CLocalChinaErrorDispatcher19ProcLockInSeriaRoomER5CUser14ENUM_CMDPACKET
    // DEMANGLED: CLocalChinaErrorDispatcher::ProcLockInSeriaRoom(CUser &,ENUM_CMDPACKET)const
    int ProcLockInSeriaRoom(CUser& user, ENUM_CMDPACKET command) const;

    std::map<ENUM_CMDPACKET, _PacketErrorCheck> error_checks_;
};

#if defined(__i386__)
static_assert(sizeof(CLocalChinaErrorDispatcher::_PacketErrorCheck) == 4,
              "CLocalChinaErrorDispatcher policy value is four old i386 bytes");
#endif


#endif  // OLDSRV_PROTOCOL_LOCAL_CHINA_ERROR_DISPATCHER_H_
