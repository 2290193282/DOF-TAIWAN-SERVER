#ifndef OLDSRV_CHARACTER_SELECT_CHARACTER_DISPATCHERS_H_
#define OLDSRV_CHARACTER_SELECT_CHARACTER_DISPATCHERS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x81C01EC in the companion source.


#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_game/player/session/packet_dispatcher_return_select_character.h"
#include "old_game/player/core/character_messages.h"
#include "old_game/player/session/character_select_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/message_base.h"


class CUser;
class PacketBuf;
struct SIG_CREATE_CHARAC;

// The reconstructed signal-8 owner has a native SQL006 continuation after
// the legacy DB queue.  Signal 32 must therefore be requested only after the
// authoritative inventory reply has reached CUser; otherwise the old FIFO
// assumption exposes the character-list appearance projection as live gear.
namespace character_select_load_order {
int RequestEtcAfterInventory(CUser* user, unsigned int tag_charac_no);
}

// IDA unit: 04_character_select_create/Inter_CreateCharac.
class Inter_CreateCharac : public ISigDispatcher {
public:
    Inter_CreateCharac();
    virtual ~Inter_CreateCharac();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
    int process_event(CUser* user, SIG_CREATE_CHARAC* payload);
    void process_createGift(CUser* user, SIG_CREATE_CHARAC* payload);
};

// IDA unit: 04_character_select_create/Inter_DeleteCharac.
class Inter_DeleteCharac : public ISigDispatcher {
public:
    Inter_DeleteCharac();
    virtual ~Inter_DeleteCharac();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 04_character_select_create/Inter_LoadCharac.
class Inter_LoadCharac : public ISigDispatcher {
public:
    Inter_LoadCharac();
    virtual ~Inter_LoadCharac();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 04_character_select_create/Inter_SelectCharac.
class Inter_SelectCharac : public ISigDispatcher {
public:
    virtual ~Inter_SelectCharac();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

// IDA unit: 04_character_select_create/DisPatcher_CreateCharac.
// C2 @ 0x85950FC: IPacketDispatcher<MSG_CREATE_CHARACTER,stCreateCharac,2>.
class DisPatcher_CreateCharac
    : public IPacketDispatcher<MSG_CREATE_CHARACTER,
                               stCreateCharac,
                               static_cast<ch_state>(2)> {
public:
    DisPatcher_CreateCharac();
    virtual ~DisPatcher_CreateCharac();
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    int makeGift(CUser* user, SIG_CREATE_CHARAC* create_charac_payload);
    virtual int check_state(CUser* user, PacketBuf& packet);
};

// IDA unit: 04_character_select_create/DisPatcher_DeleteCharac.
// C2 @ 0x8595134: IPacketDispatcher<MSG_DELETE_CHARACTER,stDeleteCharac,2>.
class DisPatcher_DeleteCharac
    : public IPacketDispatcher<MSG_DELETE_CHARACTER,
                               stDeleteCharac,
                               static_cast<ch_state>(2)> {
public:
    DisPatcher_DeleteCharac();
    virtual ~DisPatcher_DeleteCharac();
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual int check_state(CUser* user, PacketBuf& packet);
};

// IDA unit: 04_character_select_create/DisPatcher_SelectCharac.
// C2 @ 0x859524C: IPacketDispatcher<MSG_CHARAC_SELECT,stSelectCharac,2>.
class DisPatcher_SelectCharac
    : public IPacketDispatcher<MSG_CHARAC_SELECT,
                               stSelectCharac,
                               static_cast<ch_state>(2)> {
public:
    DisPatcher_SelectCharac();
    virtual ~DisPatcher_SelectCharac();
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    int checkPvpEnterCharacter(CUser* user, MSG_CHARAC_SELECT& message);
    int RequestDBCharac(CUser* user, const MSG_CHARAC_SELECT& message);
    int RequestDBInventory(CUser* user, MSG_CHARAC_SELECT& message);
    int RequestDBSkill(CUser* user, MSG_CHARAC_SELECT& message);
    int RequestDBQuest(CUser* user, MSG_CHARAC_SELECT& message);
    int RequestDBTicket(CUser* user, MSG_CHARAC_SELECT& message);
    int RequestDBDungeonClear(CUser* user, MSG_CHARAC_SELECT& message);
    int RequestDBETC(CUser* user, MSG_CHARAC_SELECT& message);
    int RequestDBKillMonsterInfo(CUser* user, MSG_CHARAC_SELECT& message);
    int RequestBlackCount(CUser* user, MSG_CHARAC_SELECT& message);
    int RegistTimerSchedules(CUser* user);
    int RequestDBInventory_forchina(CUser* user, MSG_CHARAC_SELECT& message);
};

// CMD 7 is the vtable-backed IPacketDispatcher declared by
// packet_dispatcher_return_select_character.h.  Keep this compatibility header on
// that canonical type so no non-polymorphic duplicate can shadow it.


#endif  // OLDSRV_CHARACTER_SELECT_CHARACTER_DISPATCHERS_H_
