#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_EVENT_REWARDS_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_EVENT_REWARDS_H_

#include "old_game/activity/schedule/event_messages.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// PacketDispatcher slot 213.  This is a C-side event/skill-ticket request;
// registration remains intentionally absent while the original 606-slot root
// dispatcher is not activated.
class DIspatcher_OneDayLetheTicket
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596984
    // IDA_NAME: _ZN28DIspatcher_OneDayLetheTicketC2Ev
    // DEMANGLED: DIspatcher_OneDayLetheTicket::DIspatcher_OneDayLetheTicket(void)
    DIspatcher_OneDayLetheTicket();

    // IDA_EA: 0x82416DA
    // IDA_NAME: _ZN28DIspatcher_OneDayLetheTicketD2Ev
    // IDA_EA: 0x824170A
    // IDA_NAME: _ZN28DIspatcher_OneDayLetheTicketD0Ev
    // DEMANGLED: DIspatcher_OneDayLetheTicket::~DIspatcher_OneDayLetheTicket()
    virtual ~DIspatcher_OneDayLetheTicket();

    // IDA_EA: 0x821B106
    // IDA_NAME: _ZN28DIspatcher_OneDayLetheTicket12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DIspatcher_OneDayLetheTicket::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233D76
    // IDA_NAME: _ZN28DIspatcher_OneDayLetheTicket4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DIspatcher_OneDayLetheTicket::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233D80
    // IDA_NAME: _ZN28DIspatcher_OneDayLetheTicket7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DIspatcher_OneDayLetheTicket::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// PacketDispatcher slot 29.  Registration remains intentionally absent while
// the original 606-slot dispatcher root is not activated.
class DisPatcher_Lottery
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8595628
    // IDA_NAME: _ZN18DisPatcher_LotteryC2Ev
    // DEMANGLED: DisPatcher_Lottery::DisPatcher_Lottery(void)
    DisPatcher_Lottery();

    // IDA_EA: 0x823328A
    // IDA_NAME: _ZN18DisPatcher_LotteryD2Ev
    // DEMANGLED: DisPatcher_Lottery::~DisPatcher_Lottery()
    // Non-deleting destructor identity emitted by the definition below.
    // IDA_EA: 0x82332BA
    // IDA_NAME: _ZN18DisPatcher_LotteryD0Ev
    // DEMANGLED: DisPatcher_Lottery::~DisPatcher_Lottery()
    // Deleting destructor identity emitted by the same C++ destructor body.
    virtual ~DisPatcher_Lottery();

    // IDA_EA: 0x81F8380
    // IDA_NAME: _ZN18DisPatcher_Lottery12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DisPatcher_Lottery::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233276
    // IDA_NAME: _ZN18DisPatcher_Lottery4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_Lottery::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233280
    // IDA_NAME: _ZN18DisPatcher_Lottery7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_Lottery::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// PacketDispatcher slot 211.  Registration remains intentionally absent while
// the original 606-slot dispatcher root is not activated.
class DisPatcher_MultiBoxLottery
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x859694C
    // IDA_NAME: _ZN26DisPatcher_MultiBoxLotteryC2Ev
    // DEMANGLED: DisPatcher_MultiBoxLottery::DisPatcher_MultiBoxLottery(void)
    DisPatcher_MultiBoxLottery();

    // IDA_EA: 0x8241F14
    // IDA_NAME: _ZN26DisPatcher_MultiBoxLotteryD2Ev
    // DEMANGLED: DisPatcher_MultiBoxLottery::~DisPatcher_MultiBoxLottery()
    // Non-deleting destructor identity emitted by the definition below.
    // IDA_EA: 0x8241F44
    // IDA_NAME: _ZN26DisPatcher_MultiBoxLotteryD0Ev
    // DEMANGLED: DisPatcher_MultiBoxLottery::~DisPatcher_MultiBoxLottery()
    // Deleting destructor identity emitted by the same C++ destructor body.
    virtual ~DisPatcher_MultiBoxLottery();

    // IDA_EA: 0x820B802
    // IDA_NAME: _ZN26DisPatcher_MultiBoxLottery12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: DisPatcher_MultiBoxLottery::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x8233BD2
    // IDA_NAME: _ZN26DisPatcher_MultiBoxLottery4readER9PacketBufR8MSG_BASE
    // DEMANGLED: DisPatcher_MultiBoxLottery::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x8233BDC
    // IDA_NAME: _ZN26DisPatcher_MultiBoxLottery7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: DisPatcher_MultiBoxLottery::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// PacketDispatcher slot 288.  Registration remains intentionally absent while
// the original 606-slot dispatcher root is not activated.
class Dispatcher_RequestOnTimeEventReward
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8596FA4
    // IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventRewardC2Ev
    // DEMANGLED: Dispatcher_RequestOnTimeEventReward::Dispatcher_RequestOnTimeEventReward(void)
    Dispatcher_RequestOnTimeEventReward();

    // IDA_EA: 0x8240A0E
    // IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventRewardD2Ev
    // DEMANGLED: Dispatcher_RequestOnTimeEventReward::~Dispatcher_RequestOnTimeEventReward()
    // Non-deleting destructor identity emitted by the definition below.
    // IDA_EA: 0x8240A3E
    // IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventRewardD0Ev
    // DEMANGLED: Dispatcher_RequestOnTimeEventReward::~Dispatcher_RequestOnTimeEventReward()
    // Deleting destructor identity emitted by the same C++ destructor body.
    virtual ~Dispatcher_RequestOnTimeEventReward();

    // IDA_EA: 0x822458C
    // IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventReward12dispatch_sigEP5CUserR9PacketBuf
    // DEMANGLED: Dispatcher_RequestOnTimeEventReward::dispatch_sig(CUser *,PacketBuf &)
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);

    // IDA_EA: 0x82340BE
    // IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventReward4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_RequestOnTimeEventReward::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x82340C8
    // IDA_NAME: _ZN35Dispatcher_RequestOnTimeEventReward7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_RequestOnTimeEventReward::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

// PacketDispatcher slot 449.  Registration is intentionally absent: this is
// a recovered leaf provider only until the original 606-slot dispatcher root
// has its own verified activation path.
class Dispatcher_Exchange_Random_Item_Reward
    : public IPacketDispatcher<MSG_EXCHANGE_RANDOM_ITEM_REWARD,
                               stExchangeRandomItemReward,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597A46
    // IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_RewardC2Ev
    // DEMANGLED: Dispatcher_Exchange_Random_Item_Reward::Dispatcher_Exchange_Random_Item_Reward(void)
    Dispatcher_Exchange_Random_Item_Reward();

    // IDA_EA: 0x8244578
    // IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_RewardD2Ev
    // IDA_EA: 0x82445A8
    // IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_RewardD0Ev
    // DEMANGLED: Dispatcher_Exchange_Random_Item_Reward::~Dispatcher_Exchange_Random_Item_Reward()
    virtual ~Dispatcher_Exchange_Random_Item_Reward();

    // IDA_EA: 0x81DE406
    // IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_Reward4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Exchange_Random_Item_Reward::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DE52C
    // IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_Reward7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Exchange_Random_Item_Reward::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DE72C
    // IDA_NAME: _ZN38Dispatcher_Exchange_Random_Item_Reward4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Exchange_Random_Item_Reward::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

// PacketDispatcher slot 450.  As with CMD449, this intentionally has no
// constructor-table or 606-slot-root registration side effect.
class Dispatcher_Avatar_Disjoint_Random_Reward
    : public IPacketDispatcher<MSG_AVATAR_DISJOINT_RANDOM_REWARD,
                               stAvatarDisjointRandomReward,
                               static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x8597A62
    // IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_RewardC2Ev
    // DEMANGLED: Dispatcher_Avatar_Disjoint_Random_Reward::Dispatcher_Avatar_Disjoint_Random_Reward(void)
    Dispatcher_Avatar_Disjoint_Random_Reward();

    // IDA_EA: 0x824452A
    // IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_RewardD2Ev
    // IDA_EA: 0x824455A
    // IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_RewardD0Ev
    // DEMANGLED: Dispatcher_Avatar_Disjoint_Random_Reward::~Dispatcher_Avatar_Disjoint_Random_Reward()
    virtual ~Dispatcher_Avatar_Disjoint_Random_Reward();

    // IDA_EA: 0x81DE9DA
    // IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_Reward4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_Avatar_Disjoint_Random_Reward::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81DEA06
    // IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_Reward7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_Avatar_Disjoint_Random_Reward::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81DEC56
    // IDA_NAME: _ZN40Dispatcher_Avatar_Disjoint_Random_Reward4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_Avatar_Disjoint_Random_Reward::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DIspatcher_OneDayLetheTicket) == 0x04u,
              "CMD213 dispatcher must retain its old vptr-only ABI");
static_assert(sizeof(DisPatcher_Lottery) == 0x04u,
              "CMD029 dispatcher must retain its old vptr-only ABI");
static_assert(sizeof(DisPatcher_MultiBoxLottery) == 0x04u,
              "CMD211 dispatcher must retain its old vptr-only ABI");
static_assert(sizeof(Dispatcher_RequestOnTimeEventReward) == 0x04u,
              "CMD288 dispatcher must retain its old vptr-only ABI");
static_assert(sizeof(Dispatcher_Exchange_Random_Item_Reward) == 0x04u,
              "CMD449 dispatcher must retain its old vptr-only ABI");
static_assert(sizeof(Dispatcher_Avatar_Disjoint_Random_Reward) == 0x04u,
              "CMD450 dispatcher must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_EVENT_REWARDS_H_
