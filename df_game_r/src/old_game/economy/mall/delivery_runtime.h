#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_DELIVERY_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_DELIVERY_RUNTIME_H_

#include <stdint.h>

#include <list>
#include <map>


class CEventBase;

// IDA unit: 10_mail_trade_auction_cash/DeliveryMsg.
struct DeliveryMsg {
    DeliveryMsg();

    DeliveryMsg* setBaseData(int delivery_type,
                             unsigned int account_id,
                             int charac_no,
                             int delivery_state);

    int delivery_type;
    unsigned int account_id;
    int charac_no;
    int delivery_state;
};

// IDA unit: 10_mail_trade_auction_cash/BuyItemDeliveryMsg.
struct BuyItemDeliveryMsg : public DeliveryMsg {
    BuyItemDeliveryMsg();

    int item_id;
    int item_count;
};

// IDA unit: 10_mail_trade_auction_cash/CDelivery.
class CDelivery {
public:
    CDelivery();
    ~CDelivery();

    bool checkAddress(int address);
    void attach(int address, CEventBase* event);
    void detach(int address, CEventBase* event);
    int notify(int address, DeliveryMsg& msg);

private:
    std::map<int, std::list<CEventBase*> > events_by_address;
};


#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_DELIVERY_RUNTIME_H_
