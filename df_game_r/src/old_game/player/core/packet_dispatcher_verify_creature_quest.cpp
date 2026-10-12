#include "old_game/player/core/packet_dispatcher_verify_creature_quest.h"

#include "old_game/appearance/pet/creature.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x81E3242
// IDA_NAME: _ZN30Dispatcher_VerifyCreatureQuest4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_VerifyCreatureQuest::read(PacketBuf &,MSG_BASE &)
int Dispatcher_VerifyCreatureQuest::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81E324C
// IDA_NAME: _ZN30Dispatcher_VerifyCreatureQuest7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_VerifyCreatureQuest::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_VerifyCreatureQuest::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)message;
    (void)param;
    if (!user) {
        return 0;
    }
    if (user->get_state() > 2) {
        user->getCurCharacInvenR()->GetCreatureMgrR()
            ->VerifyEquipedCreatureQuest();
    }
    return 0;
}

// IDA_EA: 0x8243FFC
// IDA_NAME: _ZN30Dispatcher_VerifyCreatureQuestD2Ev
// DEMANGLED: Dispatcher_VerifyCreatureQuest::~Dispatcher_VerifyCreatureQuest()
// IDA_EA: 0x824402C
// IDA_NAME: _ZN30Dispatcher_VerifyCreatureQuestD0Ev
// DEMANGLED: Dispatcher_VerifyCreatureQuest::~Dispatcher_VerifyCreatureQuest()
Dispatcher_VerifyCreatureQuest::~Dispatcher_VerifyCreatureQuest() {}

// IDA_EA: 0x8597D1E
// IDA_NAME: _ZN30Dispatcher_VerifyCreatureQuestC2Ev
// DEMANGLED: Dispatcher_VerifyCreatureQuest::Dispatcher_VerifyCreatureQuest(void)
Dispatcher_VerifyCreatureQuest::Dispatcher_VerifyCreatureQuest()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>() {}
