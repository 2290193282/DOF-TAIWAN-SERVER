#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CASH_SERVICE_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CASH_SERVICE_RUNTIME_H_

#include <array>
#include <stddef.h>
#include <stdint.h>

#include "old_game/economy/mall/cash_billing_runtime.h"

namespace nexon {
namespace cash {

// The old cash reply writes one database byte at +0 and one transaction word
// at +4.  Natural i386 alignment supplies the intervening bytes; no opaque
// padding storage is introduced.
struct NXTransaction {
    uint8_t database_id;
    uint32_t transaction_id;
};

// Old df_game_r IDA unit: 10_mail_trade_auction_cash/nexon/nexon_cash.cpp.
// Field placement follows the constructor and all packet producers/consumers:
// cash kind +0, shop +2, service +4, request id +8, connection byte +12,
// host text +13, port +30, TCP socket +32, inbound message +64, outbound
// message +584.  Natural i386 alignment supplies the unnamed gaps.
class NCashImpl {
public:
    // IDA_EA: 0x81ACFDC.
    NCashImpl(const char* host,
              short port,
              unsigned char cash_type,
              unsigned short shop_id,
              short service_id);

    // IDA_EA: 0x81AD080.
    int reconnect();

    // These two routines return ByteBuffer by value.  Old i386 uses their
    // hidden return object as the first machine argument; declaring a normal
    // value return retains that ABI rather than treating it as a fake member.
    // IDA_EA: 0x81AD0E6.
    ByteBuffer set_data_header(unsigned char channel, unsigned char packet_id);
    // IDA_EA: 0x81AD168.
    ByteBuffer get_data_header(unsigned char& channel,
                               unsigned char& packet_id,
                               unsigned int& request_id);

    // IDA_EA: 0x81AD316.
    int checkEntrance(const char* account, char& entrance);
    // IDA_EA: 0x81AD49E.
    int getBalance(const char* account, int& balance);
    // IDA_EA: 0x81AD5C2.
    int purchase(unsigned long long account_id,
                 int product_id,
                 int product_count,
                 const char* account,
                 const char* character,
                 NXTransaction& transaction);
    // IDA_EA: 0x81AD7AE.
    int purchase(unsigned long long account_id,
                 int product_id,
                 int product_count,
                 const char* account,
                 const char* character,
                 NXTransaction& transaction,
                 short service_override);
    // IDA_EA: 0x81AD9C4.
    int gift(unsigned long long account_id,
             int product_id,
             int product_count,
             const char* account,
             const char* character,
             const char* recipient,
             const char* message,
             NXTransaction& transaction);
    // IDA_EA: 0x81ADBD4.
    int confirm(unsigned char database_id_low_byte, unsigned int transaction_id);
    // IDA_EA: 0x81ADD04.
    int cancel(unsigned char database_id_low_byte, unsigned int transaction_id);
    // IDA_EA: 0x81ADE34.
    int chargeCash(char* account,
                   unsigned char first_flag,
                   unsigned char second_flag,
                   int amount,
                   char* field_5,
                   char* field_6,
                   char* field_7,
                   char* field_8,
                   int field_9,
                   NXTransaction& transaction);

    // IDA_EA: 0x81AEA46.
    int recv_msg();
    // IDA_EA: 0x81AEB3C.
    int recv_body();
    // IDA_EA: 0x81AEC2C.
    int send_msg(unsigned int body_length);

    // Remaining members in the original NCashImpl class surface.
    // IDA_EA: 0x81AEA0A.
    int checkCoupon();
    // IDA_EA: 0x81AEA14.
    int useCoupon();
    // IDA_EA: 0x81AEA1E.
    int checkCafeCoupon();
    // IDA_EA: 0x81AEA28.
    int useCafeCoupon();
    // IDA_EA: 0x81AEA32.
    NCashImpl* _increaseRequestID();
    // IDA_EA: 0x81AD1E0.
    int handle_cash_error(int error_code);
    // IDA_EA: 0x81AD2C0.
    int handle_coupon_error(int packet_id);

private:
    uint8_t cash_type_;
    uint16_t shop_id_;
    uint16_t service_id_;
    uint32_t request_id_;
    uint8_t connected_;
    std::array<char, 17> host_name_;
    uint16_t port_;
    TCPSocket cash_socket_;
    NCashMessage incoming_message_;
    NCashMessage outgoing_message_;
};

// Old df_game_r IDA unit: 10_mail_trade_auction_cash/nexon/nexon_cash.cpp.
class NexonCash {
public:
    // IDA_EA: 0x81AE1B0.
    NexonCash(const char* host,
              short port,
              char cash_type,
              short shop_id,
              short service_id);
    // IDA_EA: 0x81AE242.
    ~NexonCash();

    // IDA_EA: 0x81AE258.
    int getBalance(const char* account, int& balance);
    // IDA_EA: 0x81AE27C.
    int purchase(unsigned long long account_id,
                 int product_id,
                 int product_count,
                 const char* account,
                 const char* character,
                 NXTransaction& transaction);
    // IDA_EA: 0x81AE2CE.
    int purchase(unsigned long long account_id,
                 int product_id,
                 int product_count,
                 const char* account,
                 const char* character,
                 NXTransaction& transaction,
                 short service_override);
    // IDA_EA: 0x81AE330.
    int gift(unsigned long long account_id,
             int product_id,
             int product_count,
             const char* account,
             const char* character,
             const char* recipient,
             const char* message,
             NXTransaction& transaction);
    // IDA_EA: 0x81AE390.
    int confirm(const NXTransaction& transaction);
    // IDA_EA: 0x81AE3BC.
    int cancel(const NXTransaction& transaction);
    // IDA_EA: 0x81AE440.
    int charge(char* account,
               unsigned char first_flag,
               unsigned char second_flag,
               int amount,
               char* field_5,
               char* field_6,
               char* field_7,
               char* field_8,
               int field_9,
               NXTransaction& transaction);
    // IDA_EA: 0x81AE4AE.
    int checkEntrance(const char* account, char& entrance);

    // IDA_EA: 0x81AE3E8.
    int checkCoupon();
    // IDA_EA: 0x81AE3FE.
    int useCoupon();
    // IDA_EA: 0x81AE414.
    int checkCafeCoupon();
    // IDA_EA: 0x81AE42A.
    int useCafeCoupon();

private:
    NCashImpl* impl_;
};

static_assert(sizeof(NXTransaction) == 0x08u,
              "NXTransaction reply layout must retain the old eight bytes");
static_assert(offsetof(NXTransaction, transaction_id) == 0x04u,
              "NXTransaction transaction id must remain at old offset +4");
#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(NCashImpl) == 0x450u,
              "NCashImpl old i386 ABI must stay 0x450");
static_assert(sizeof(NexonCash) == 0x04u,
              "NexonCash old i386 ABI must stay one implementation pointer");
#endif

}  // namespace cash
}  // namespace nexon

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CASH_SERVICE_RUNTIME_H_
