#ifndef OLDSRV_OLD_GAME_PVP_BATTLEFIELD_PACKET_DISPATCHER_PVP_PRACTICE_COMBO_SCORE_H_
#define OLDSRV_OLD_GAME_PVP_BATTLEFIELD_PACKET_DISPATCHER_PVP_PRACTICE_COMBO_SCORE_H_

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
class PacketBuf;

class Dispatcher_RequestComboScore
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_RequestComboScore();
    virtual ~Dispatcher_RequestComboScore();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_ComboScoreInfo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    Dispatcher_ComboScoreInfo();
    virtual ~Dispatcher_ComboScoreInfo();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Inter_PvpPracticeComboScore : public ISigDispatcher {
public:
    Inter_PvpPracticeComboScore();
    virtual ~Inter_PvpPracticeComboScore();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestComboScore) == 0x04u,
              "CMD577 dispatcher must remain vptr-only on target i386");
static_assert(sizeof(Dispatcher_ComboScoreInfo) == 0x04u,
              "CMD578 dispatcher must remain vptr-only on target i386");
static_assert(sizeof(Inter_PvpPracticeComboScore) == 0x04u,
              "combo-score inter dispatcher must remain vptr-only on i386");
#endif

#endif  // OLDSRV_OLD_GAME_PVP_BATTLEFIELD_PACKET_DISPATCHER_PVP_PRACTICE_COMBO_SCORE_H_
