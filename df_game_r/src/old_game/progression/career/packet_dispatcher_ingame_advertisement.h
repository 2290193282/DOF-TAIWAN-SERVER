#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_INGAME_ADVERTISEMENT_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_INGAME_ADVERTISEMENT_H_

#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA unit: 08_quest_skill_growth/Dispatcher_RequestIngameAdvertisement.
// 0828 registers this legacy owner on CMD486.  The target client batches
// request IDs and expects an entry-count response after the framework result.
class Dispatcher_RequestIngameAdvertisement
    : public IPacketDispatcher<MSG_REQUEST_INGAME_ADVERTISEMENT,
                               stRequestIngameAdvertisement,
                               static_cast<ch_state>(2)> {
public:
    // IDA_EA: 0x85958C8
    // IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisementC2Ev
    // DEMANGLED: Dispatcher_RequestIngameAdvertisement::Dispatcher_RequestIngameAdvertisement(void)
    Dispatcher_RequestIngameAdvertisement();

    // IDA_EA: 0x8243E76 / 0x8243EA6
    // IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisementD2Ev / D0Ev
    // DEMANGLED: Dispatcher_RequestIngameAdvertisement::~Dispatcher_RequestIngameAdvertisement()
    virtual ~Dispatcher_RequestIngameAdvertisement();

    // IDA_EA: 0x81E434C
    // IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisement4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81E43A2
    // IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisement7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81E4662
    // IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisement11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81E46C8
    // IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisement4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81E4B6C
    // IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisement24sendRewardItemSystemMailEP5CUsermi
    void sendRewardItemSystemMail(CUser* user,
                                  unsigned long item_id,
                                  int item_count);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestIngameAdvertisement) == 0x04u,
              "CMD486 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_INGAME_ADVERTISEMENT_H_
