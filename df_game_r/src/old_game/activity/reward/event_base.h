#ifndef OLDSRV_GLOBAL_EVENT_BASE_H_
#define OLDSRV_GLOBAL_EVENT_BASE_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mall/delivery_runtime.h"

class CUser;
class PacketGuard;

// The original ABI passes this four-byte event payload as a named value type.
// Keeping it as an integer typedef changes every StartEvent/TriggerEventStart
// mangled name from `10Word_Param` to `j`.
struct Word_Param {
    Word_Param(uint32_t raw_value) : value(raw_value) {}

    operator uint32_t() const { return value; }

    uint32_t value;
};

static_assert(sizeof(Word_Param) == 0x04u,
              "Word_Param IDA ABI size must stay 0x04");

// IDA unit: 99_misc_global/stADDINFO.
// IDA_EA: 0x810AF10
struct stADDINFO {
    stADDINFO();

    int16_t word_00;
    int16_t word_02;
};

// IDA unit: 11_event_runtime/CEventBase public base ABI.
// IDA_EA: ctor 0x810AE2C, dtor 0x810AE52/0x810AE78,
// virtuals 0x810AF26, 0x80C84C4..0x80C84FA, 0x810AE96..0x810AEAE.
// This is the shared event base layout used by CEventManager/CDelivery. The
// concrete event subclasses and gameplay behavior remain 11_event_runtime.
class CEventBase {
public:
    CEventBase();
    virtual ~CEventBase();

    virtual void StartEvent() = 0;
    virtual void EndEvent() = 0;
    virtual void StartEvent(Word_Param param);
    virtual void StartAction();
    virtual void AppendInfo(PacketGuard& guard);
    virtual stADDINFO GetAddInfo();
    virtual void changeCharacName(CUser* user,
                                  int charac_no,
                                  char* name,
                                  int length);
    virtual void dailyresetData();
    virtual void deliveryStart(CDelivery* delivery);
    virtual void deliveryEnd(CDelivery* delivery);
    virtual void delivery(DeliveryMsg& msg);
    virtual int IsEventing(CUser* user) const;

    CEventBase* SetEventFlag(bool enabled);
    // IsEventing(NULL) is false for some active user-scoped events.  Runtime
    // configuration therefore observes the raw global flag through this
    // non-virtual helper, preserving the recovered vtable and object layout.
    bool IsEventFlagSet() const;

protected:
    stADDINFO add_info_;
    uint8_t event_flag_;
    uint8_t reserved_09_[3];
};

static_assert(sizeof(stADDINFO) == 0x04u,
              "stADDINFO IDA ABI size must stay 0x04");

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CEventBase) == 0x0Cu,
              "CEventBase IDA ABI size must stay 0x0C");
#endif


#endif  // OLDSRV_GLOBAL_EVENT_BASE_H_
