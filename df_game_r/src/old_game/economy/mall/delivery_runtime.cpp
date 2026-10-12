#include "old_game/economy/mall/delivery_runtime.h"

#include "old_game/activity/reward/event_base.h"


#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(DeliveryMsg) == 0x10u,
              "DeliveryMsg IDA ABI size must stay 0x10");
static_assert(sizeof(BuyItemDeliveryMsg) == 0x18u,
              "BuyItemDeliveryMsg IDA ABI size must stay 0x18");
static_assert(sizeof(CDelivery) == 0x18u,
              "CDelivery IDA ABI size must stay 0x18");
#endif


// IDA_EA: 0x86978C0
// IDA_NAME: _ZN11DeliveryMsgC2Ev
// DEMANGLED: DeliveryMsg::DeliveryMsg(void)
DeliveryMsg::DeliveryMsg()
    : delivery_type(0),
      account_id(0),
      charac_no(0),
      delivery_state(-1) {}

// IDA_EA: 0x86978EC
// IDA_NAME: _ZN11DeliveryMsg11setBaseDataEijii
// DEMANGLED: DeliveryMsg::setBaseData(int,unsigned int,int,int)
DeliveryMsg* DeliveryMsg::setBaseData(int delivery_type_value,
                                      unsigned int account_id_value,
                                      int charac_no_value,
                                      int delivery_state_value) {
    delivery_type = delivery_type_value;
    account_id = account_id_value;
    charac_no = charac_no_value;
    delivery_state = delivery_state_value;
    return this;
}

// IDA_EA: 0x869795A
// IDA_NAME: _ZN18BuyItemDeliveryMsgC2Ev
// DEMANGLED: BuyItemDeliveryMsg::BuyItemDeliveryMsg(void)
BuyItemDeliveryMsg::BuyItemDeliveryMsg()
    : DeliveryMsg(),
      item_id(0),
      item_count(0) {}

// IDA_EA: 0x8116386
// IDA_NAME: _ZN9CDeliveryC2Ev
// DEMANGLED: CDelivery::CDelivery(void)
CDelivery::CDelivery() {}

// IDA_EA: 0x811639A
// IDA_NAME: _ZN9CDeliveryD2Ev
// DEMANGLED: CDelivery::~CDelivery()
CDelivery::~CDelivery() {
    events_by_address.clear();
}

// IDA_EA: 0x81163DC
// IDA_NAME: _ZN9CDelivery12checkAddressEi
// DEMANGLED: CDelivery::checkAddress(int)
bool CDelivery::checkAddress(int address) {
    return address > 0 && address <= 2;
}

// IDA_EA: 0x81163FA
// IDA_NAME: _ZN9CDelivery6attachEiP10CEventBase
// DEMANGLED: CDelivery::attach(int,CEventBase *)
void CDelivery::attach(int address, CEventBase* event) {
    if (!checkAddress(address)) {
        return;
    }

    std::map<int, std::list<CEventBase*> >::iterator found =
        events_by_address.find(address);
    if (found != events_by_address.end()) {
        for (std::list<CEventBase*>::iterator it = found->second.begin();
             it != found->second.end(); ++it) {
            if (*it == event) {
                return;
            }
        }
    }

    events_by_address[address].push_back(event);
}

// IDA_EA: 0x81165FA
// IDA_NAME: _ZN9CDelivery6detachEiP10CEventBase
// DEMANGLED: CDelivery::detach(int,CEventBase *)
void CDelivery::detach(int address, CEventBase* event) {
    if (!checkAddress(address)) {
        return;
    }

    std::map<int, std::list<CEventBase*> >::iterator found =
        events_by_address.find(address);
    if (found != events_by_address.end()) {
        found->second.remove(event);
    }
}

// IDA_EA: 0x8116682
// IDA_NAME: _ZN9CDelivery6notifyEiR11DeliveryMsg
// DEMANGLED: CDelivery::notify(int,DeliveryMsg &)
int CDelivery::notify(int address, DeliveryMsg& msg) {
    if (!checkAddress(address)) {
        return 1;
    }

    std::map<int, std::list<CEventBase*> >::iterator found =
        events_by_address.find(address);
    if (found == events_by_address.end()) {
        return 0;
    }

    for (std::list<CEventBase*>::iterator it = found->second.begin();
         it != found->second.end(); ++it) {
        CEventBase* event = *it;
        if (event) {
            // Active old df_game_r at 0x8116682 loads vtable +0x30, which is
            // the named CEventBase::delivery(DeliveryMsg&) slot at 0x810AEAE.
            event->delivery(msg);
        }
    }

    return 0;
}
