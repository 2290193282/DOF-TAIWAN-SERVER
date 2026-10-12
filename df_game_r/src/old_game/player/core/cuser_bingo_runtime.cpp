#include "old_game/player/core/cuser.h"

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_game/activity/schedule/bingo_runtime.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x8690D9A
// IDA_NAME: _ZN5CUser23sendDecorationObjectCntEs
// DEMANGLED: CUser::sendDecorationObjectCnt(short)
void CUser::sendDecorationObjectCnt(short count) {
    PacketGuard packet;
    packet.put_header(0, 428);
    packet.put_short(count);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x86912BE
// IDA_NAME: _ZN5CUser13initBingoDataEv
// DEMANGLED: CUser::initBingoData(void)
int CUser::initBingoData() {
    CEventManager* const manager = GlobalData::s_event_manager;
    CEventBase* const base = manager ? manager->GetRepeatEvent(106) : 0;
    if (!base || !base->IsEventing(this)) {
        return 0;
    }

    BingoEvent* const event = static_cast<BingoEvent*>(base);
    boost::shared_ptr<CBingo>& bingo = event->getBingoSystem();
    if (!bingo.get()) {
        return 0;
    }
    bingo->initUserData(*getBingoData());
    sendBingoData();
    return 1;
}

// IDA_EA: 0x869134E
// IDA_NAME: _ZN5CUser13sendBingoDataEv
// DEMANGLED: CUser::sendBingoData(void)
void CUser::sendBingoData() {
    int today = 0;
    CEventManager* const manager = GlobalData::s_event_manager;
    CEventBase* const base = manager ? manager->GetRepeatEvent(106) : 0;
    if (base) {
        BingoEvent* const event = static_cast<BingoEvent*>(base);
        boost::shared_ptr<CBingo>& bingo = event->getBingoSystem();
        if (bingo.get()) {
            today = bingo->getToday();
        }
    }

    BingoData* const data = getBingoData();
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 450);
    packet.put_byte(today);
    packet.put_int(static_cast<int>(data->bingo_count));
    packet.put_byte(data->selected_count);
    packet.finalize(true);
    Send(packet);
}

// IDA_EA: 0x8691470
// IDA_NAME: _ZN5CUser16sendBingoAddDataEv
// DEMANGLED: CUser::sendBingoAddData(void)
void CUser::sendBingoAddData() {
    CEventManager* const manager = GlobalData::s_event_manager;
    CEventBase* const event = manager ? manager->GetRepeatEvent(106) : 0;
    if (event && event->IsEventing(this) &&
        getBingoData()->selected_count != 0) {
        sendDecorationObjectCnt(1);
    }
}
