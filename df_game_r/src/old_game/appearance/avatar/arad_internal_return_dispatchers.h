#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_INTERNAL_RETURN_DISPATCHERS_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_INTERNAL_RETURN_DISPATCHERS_H_

// C-owned business leaves for ARAD's in-world internal return dispatcher.
// The map/lifetime owner is B: old_data/db/legacy_dispatch/
// arad_internal_dispatcher.{h,cpp}.  Do not register these through the root
// PacketDispatcher here.
#include "old_data/db/legacy_dispatch/arad_internal_dispatcher.h"

namespace ARAD {
namespace DISPATCHER {

// IDA type layout for every concrete leaf: Arad_IInterDispatcher only
// (vptr + code, 0x08 on i386).  The exact constructor code is retained in
// each declaration rather than being collapsed into a generic leaf template:
// the old binary has one independently addressable C2/D2/D0 trio per class.

class Arad_INTER_kLoadRewardEventItem : public Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A46E | IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_INTER_kLoadRewardEventItemC2Ev
    Arad_INTER_kLoadRewardEventItem();
    // IDA_EA: 0x819A492 | IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_INTER_kLoadRewardEventItemD2Ev
    // IDA_EA: 0x819A4C2 | IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_INTER_kLoadRewardEventItemD0Ev
    virtual ~Arad_INTER_kLoadRewardEventItem();
    // IDA_EA: 0x8198FD0 | IDA_NAME: _ZN4ARAD10DISPATCHER31Arad_INTER_kLoadRewardEventItem12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int length);
};

class Arad_INTER_kSaveServerState : public Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A4E0 | IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kSaveServerStateC2Ev
    Arad_INTER_kSaveServerState();
    // IDA_EA: 0x819A504 | IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kSaveServerStateD2Ev
    // IDA_EA: 0x819A534 | IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kSaveServerStateD0Ev
    virtual ~Arad_INTER_kSaveServerState();
    // IDA_EA: 0x8199278 | IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kSaveServerState12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int length);
};

class Arad_INTER_kLoadServerState : public Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A552 | IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kLoadServerStateC2Ev
    Arad_INTER_kLoadServerState();
    // IDA_EA: 0x819A576 | IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kLoadServerStateD2Ev
    // IDA_EA: 0x819A5A6 | IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kLoadServerStateD0Ev
    virtual ~Arad_INTER_kLoadServerState();
    // IDA_EA: 0x81992FE | IDA_NAME: _ZN4ARAD10DISPATCHER27Arad_INTER_kLoadServerState12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int length);
};

class Arad_INTER_kDeleteServerState : public Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A5C4 | IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kDeleteServerStateC2Ev
    Arad_INTER_kDeleteServerState();
    // IDA_EA: 0x819A5E8 | IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kDeleteServerStateD2Ev
    // IDA_EA: 0x819A618 | IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kDeleteServerStateD0Ev
    virtual ~Arad_INTER_kDeleteServerState();
    // IDA_EA: 0x819935E | IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kDeleteServerState12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int length);
};

class Arad_INTER_kAvatarRoulette : public Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A3FC | IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kAvatarRouletteC2Ev
    Arad_INTER_kAvatarRoulette();
    // IDA_EA: 0x819A420 | IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kAvatarRouletteD2Ev
    // IDA_EA: 0x819A450 | IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kAvatarRouletteD0Ev
    virtual ~Arad_INTER_kAvatarRoulette();
    // IDA_EA: 0x8198A9A | IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kAvatarRoulette12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int length);
};

class Arad_INTER_kAradJumpingCharacter : public Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A636 | IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacterC2Ev
    Arad_INTER_kAradJumpingCharacter();
    // IDA_EA: 0x819A65A | IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacterD2Ev
    // IDA_EA: 0x819A68A | IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacterD0Ev
    virtual ~Arad_INTER_kAradJumpingCharacter();
    // IDA_EA: 0x81999F2 | IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacter12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int length);
    // IDA_EA: 0x81993C4 | IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacter14clearEpicQuestEP5CUser
    int clearEpicQuest(CUser* user);
    // IDA_EA: 0x819973E | IDA_NAME: _ZN4ARAD10DISPATCHER32Arad_INTER_kAradJumpingCharacter13sendHelperBoxEP5CUserhh
    int sendHelperBox(CUser* user, unsigned char ticket_type,
                      unsigned char grow_type);
};

class Arad_INTER_kAvatarHiddenOptionChange : public Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A6A8 | IDA_NAME: _ZN4ARAD10DISPATCHER36Arad_INTER_kAvatarHiddenOptionChangeC2Ev
    Arad_INTER_kAvatarHiddenOptionChange();
    // IDA_EA: 0x819A6CC | IDA_NAME: _ZN4ARAD10DISPATCHER36Arad_INTER_kAvatarHiddenOptionChangeD2Ev
    // IDA_EA: 0x819A6FC | IDA_NAME: _ZN4ARAD10DISPATCHER36Arad_INTER_kAvatarHiddenOptionChangeD0Ev
    virtual ~Arad_INTER_kAvatarHiddenOptionChange();
    // IDA_EA: 0x8199D6E | IDA_NAME: _ZN4ARAD10DISPATCHER36Arad_INTER_kAvatarHiddenOptionChange12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int length);
};

class Arad_INTER_kUseAvatarRecharge : public Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A71A | IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kUseAvatarRechargeC2Ev
    Arad_INTER_kUseAvatarRecharge();
    // IDA_EA: 0x819A73E | IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kUseAvatarRechargeD2Ev
    // IDA_EA: 0x819A76E | IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kUseAvatarRechargeD0Ev
    virtual ~Arad_INTER_kUseAvatarRecharge();
    // IDA_EA: 0x8199F0E | IDA_NAME: _ZN4ARAD10DISPATCHER29Arad_INTER_kUseAvatarRecharge12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int length);
};

class Arad_INTER_kEmblemCompound : public Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A78C | IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kEmblemCompoundC2Ev
    Arad_INTER_kEmblemCompound();
    // IDA_EA: 0x819A7B0 | IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kEmblemCompoundD2Ev
    // IDA_EA: 0x819A7E0 | IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kEmblemCompoundD0Ev
    virtual ~Arad_INTER_kEmblemCompound();
    // IDA_EA: 0x819A0C0 | IDA_NAME: _ZN4ARAD10DISPATCHER26Arad_INTER_kEmblemCompound12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int length);
};

class Arad_INTER_kAvatarConvert : public Arad_IInterDispatcher {
public:
    // IDA_EA: 0x819A7FE | IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_INTER_kAvatarConvertC2Ev
    Arad_INTER_kAvatarConvert();
    // IDA_EA: 0x819A822 | IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_INTER_kAvatarConvertD2Ev
    // IDA_EA: 0x819A852 | IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_INTER_kAvatarConvertD0Ev
    virtual ~Arad_INTER_kAvatarConvert();
    // IDA_EA: 0x819A20E | IDA_NAME: _ZN4ARAD10DISPATCHER25Arad_INTER_kAvatarConvert12dispatch_sigEP5CUserPci
    virtual int dispatch_sig(CUser* user, char* payload, int length);
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Arad_INTER_kLoadRewardEventItem) == 0x08u,
              "ARAD internal leaf ABI must stay 0x08");
static_assert(sizeof(Arad_INTER_kSaveServerState) == 0x08u,
              "ARAD internal leaf ABI must stay 0x08");
static_assert(sizeof(Arad_INTER_kLoadServerState) == 0x08u,
              "ARAD internal leaf ABI must stay 0x08");
static_assert(sizeof(Arad_INTER_kDeleteServerState) == 0x08u,
              "ARAD internal leaf ABI must stay 0x08");
static_assert(sizeof(Arad_INTER_kAvatarConvert) == 0x08u,
              "ARAD internal leaf ABI must stay 0x08");
#endif

}  // namespace DISPATCHER
}  // namespace ARAD

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_ARAD_INTERNAL_RETURN_DISPATCHERS_H_
