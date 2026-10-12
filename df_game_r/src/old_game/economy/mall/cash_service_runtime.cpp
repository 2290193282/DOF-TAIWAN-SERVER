#include "old_game/economy/mall/cash_service_runtime.h"

#include "infra/log/format/trace.h"
#include "infra/security/policy/packet_integrity.h"

#include <sys/socket.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>

namespace {

const unsigned long long kChargeCashProtocolConstant = 0x00000000F3D8266FULL;
const unsigned long kChargeCashEncryptionKey = 0x1B1BDE64UL;

// The old routine owns four static, shared scratch regions and clears them on
// each invocation.  They are named protocol buffers rather than anonymous raw
// storage; the two wide regions are exactly 0xFA0 bytes on old glibc/i386.
struct CashChargeScratch {
    std::array<char, 0x3e8> formatted_request;
    std::array<wchar_t, 1000> wide_request;
    std::array<wchar_t, 1000> encrypted_request;
    std::array<char, 0x3e8> encoded_request;
};

static_assert(sizeof(wchar_t) == 4u,
              "old CentOS i386 cash encryption uses four-byte wchar_t");
static_assert(sizeof(CashChargeScratch::wide_request) == 0xfa0u,
              "cash charge wide scratch must retain old size");

CashChargeScratch g_cash_charge_scratch;

}  // namespace

namespace nexon {
namespace cash {

// IDA_EA: 0x81ACFDC
// IDA_NAME: _ZN5nexon4cash9NCashImplC2EPKcshts
// DEMANGLED: nexon::cash::NCashImpl::NCashImpl(char const*,short,unsigned char,unsigned short,short)
NCashImpl::NCashImpl(const char* host,
                     short port,
                     unsigned char cash_type,
                     unsigned short shop_id,
                     short service_id) {
    request_id_ = 0;
    cash_type_ = cash_type;
    shop_id_ = shop_id;
    service_id_ = static_cast<uint16_t>(service_id);
    connected_ = 0;
    std::strcpy(host_name_.data(), host);
    port_ = static_cast<uint16_t>(port);
}

// IDA_EA: 0x81AD080
// IDA_NAME: _ZN5nexon4cash9NCashImpl9reconnectEv
// DEMANGLED: nexon::cash::NCashImpl::reconnect(void)
int NCashImpl::reconnect() {
    if (connected_ != 1u) {
        cash_socket_.close();
        cash_socket_.open();
        connected_ = static_cast<uint8_t>(
            cash_socket_.connect(host_name_.data(), port_));
    }
    return connected_;
}

// IDA_EA: 0x81AD0E6
// IDA_NAME: _ZN5nexon4cash9NCashImpl15set_data_headerEhh
// DEMANGLED: nexon::cash::NCashImpl::set_data_header(unsigned char,unsigned char)
ByteBuffer NCashImpl::set_data_header(unsigned char channel,
                                      unsigned char packet_id) {
    ByteBuffer request(outgoing_message_.body(), 0x200u);
    request << channel;
    request << packet_id;
    _increaseRequestID();
    request << request_id_;
    return request;
}

// IDA_EA: 0x81AD168
// IDA_NAME: _ZN5nexon4cash9NCashImpl15get_data_headerERhS2_Rj
// DEMANGLED: nexon::cash::NCashImpl::get_data_header(unsigned char &,unsigned char &,unsigned int &)
ByteBuffer NCashImpl::get_data_header(unsigned char& channel,
                                      unsigned char& packet_id,
                                      unsigned int& request_id) {
    ByteBuffer response(incoming_message_.body(),
                        static_cast<unsigned int>(
                            incoming_message_.body_length()));
    response >> channel;
    response >> packet_id;
    response >> request_id;
    return response;
}

// IDA_EA: 0x81AD1E0
// IDA_NAME: _ZN5nexon4cash9NCashImpl17handle_cash_errorEi
// DEMANGLED: nexon::cash::NCashImpl::handle_cash_error(int)
int NCashImpl::handle_cash_error(int error_code) {
    int result = 16;
    switch (error_code) {
        case 5:
            result = 3;
            std::printf("[%s] NO REMIN CASH\n", "handle_cash_error");
            break;
        case 6:
            result = 4;
            std::printf("[%s] DB error occur\n", "handle_cash_error");
            break;
        case 8:
            result = 5;
            std::printf("[%s] E_GIFT_LIMIT_OVER Remain cash\n",
                        "handle_cash_error");
            break;
        case 9:
            result = 6;
            std::printf("[%s] E_GIFT_NOT_READY error\n",
                        "handle_cash_error");
            break;
        case 10:
            result = 7;
            std::printf("[%s] E_GIFT_UNDER_AGE error\n",
                        "handle_cash_error");
            break;
        default:
            std::printf("[%s] Undefined cash error: %u\n",
                        "handle_cash_error",
                        error_code);
            break;
    }
    return result;
}

// IDA_EA: 0x81AD2C0
// IDA_NAME: _ZN5nexon4cash9NCashImpl19handle_coupon_errorEi
// DEMANGLED: nexon::cash::NCashImpl::handle_coupon_error(int)
int NCashImpl::handle_coupon_error(int packet_id) {
    if (packet_id != 1) {
        if (packet_id > 1) {
            if (packet_id != 2 && packet_id != 3) {
                std::printf("[%s] Invalid coupon packet_id: %u\n",
                            "handle_coupon_error",
                            packet_id);
            }
        } else if (packet_id) {
            std::printf("[%s] Invalid coupon packet_id: %u\n",
                        "handle_coupon_error",
                        packet_id);
        }
    }
    return 16;
}

// IDA_EA: 0x81AEA0A
// IDA_NAME: _ZN5nexon4cash9NCashImpl11checkCouponEv
// DEMANGLED: nexon::cash::NCashImpl::checkCoupon(void)
int NCashImpl::checkCoupon() {
    return 0;
}

// IDA_EA: 0x81AEA14
// IDA_NAME: _ZN5nexon4cash9NCashImpl9useCouponEv
// DEMANGLED: nexon::cash::NCashImpl::useCoupon(void)
int NCashImpl::useCoupon() {
    return 0;
}

// IDA_EA: 0x81AEA1E
// IDA_NAME: _ZN5nexon4cash9NCashImpl15checkCafeCouponEv
// DEMANGLED: nexon::cash::NCashImpl::checkCafeCoupon(void)
int NCashImpl::checkCafeCoupon() {
    return 0;
}

// IDA_EA: 0x81AEA28
// IDA_NAME: _ZN5nexon4cash9NCashImpl13useCafeCouponEv
// DEMANGLED: nexon::cash::NCashImpl::useCafeCoupon(void)
int NCashImpl::useCafeCoupon() {
    return 0;
}

// IDA_EA: 0x81AEA32
// IDA_NAME: _ZN5nexon4cash9NCashImpl18_increaseRequestIDEv
// DEMANGLED: nexon::cash::NCashImpl::_increaseRequestID(void)
NCashImpl* NCashImpl::_increaseRequestID() {
    ++request_id_;
    return this;
}

// IDA_EA: 0x81AD316
// IDA_NAME: _ZN5nexon4cash9NCashImpl13checkEntranceEPKcRc
// DEMANGLED: nexon::cash::NCashImpl::checkEntrance(char const*,char &)
int NCashImpl::checkEntrance(const char* account, char& entrance) {
    if (reconnect() != 1) {
        return 2;
    }

    ByteBuffer request(outgoing_message_.body(), 0x200u);
    request << static_cast<unsigned char>(3);
    request << static_cast<unsigned char>(1);
    request << static_cast<unsigned char>(service_id_);
    request << account;
    _increaseRequestID();

    int result = send_msg(static_cast<unsigned int>(request.length()));
    if (result) {
        return result;
    }
    result = recv_msg();
    if (result) {
        return result;
    }

    ByteBuffer response(incoming_message_.body(),
                        static_cast<unsigned int>(
                            incoming_message_.body_length()));
    unsigned char ignored_response_code = 0;
    unsigned char received_entrance = 0;
    response >> ignored_response_code;
    response >> received_entrance;
    entrance = static_cast<char>(received_entrance);
    std::printf("[%s] CheckEntrace: %d\n",
                "checkEntrance",
                static_cast<int>(entrance));
    return 0;
}

// IDA_EA: 0x81AD49E
// IDA_NAME: _ZN5nexon4cash9NCashImpl10getBalanceEPKcRi
// DEMANGLED: nexon::cash::NCashImpl::getBalance(char const*,int &)
int NCashImpl::getBalance(const char* account, int& balance) {
    if (reconnect() != 1) {
        return 2;
    }

    ByteBuffer request = set_data_header(0, 0);
    request << account;

    int result = send_msg(static_cast<unsigned int>(request.length()));
    if (result) {
        return result;
    }
    result = recv_msg();
    if (result) {
        return result;
    }

    unsigned char response_channel = 0;
    unsigned char packet_id = 0;
    unsigned int response_request_id = 0;
    ByteBuffer response = get_data_header(response_channel,
                                          packet_id,
                                          response_request_id);
    if (packet_id) {
        return handle_cash_error(packet_id);
    }
    response >> balance;
    std::printf("[%s] Remain cash: %d\n", "getBalance", balance);
    return 0;
}

// IDA_EA: 0x81AD5C2
// IDA_NAME: _ZN5nexon4cash9NCashImpl8purchaseEyiiPKcS3_RNS0_13NXTransactionE
// DEMANGLED: nexon::cash::NCashImpl::purchase(unsigned long long,int,int,char const*,char const*,nexon::cash::NXTransaction &)
int NCashImpl::purchase(unsigned long long account_id,
                        int product_id,
                        int product_count,
                        const char* account,
                        const char* character,
                        NXTransaction& transaction) {
    if (reconnect() != 1) {
        return 2;
    }

    ByteBuffer request = set_data_header(0, 1);
    request << static_cast<short>(service_id_);
    request << account_id;
    request << product_id;
    request << product_count;
    request << account;
    request << character;
    request << cash_type_;
    request << static_cast<short>(shop_id_);

    int result = send_msg(static_cast<unsigned int>(request.length()));
    if (result) {
        return result;
    }
    result = recv_msg();
    if (result) {
        return result;
    }

    unsigned char response_channel = 0;
    unsigned char packet_id = 0;
    unsigned int response_request_id = 0;
    ByteBuffer response = get_data_header(response_channel,
                                          packet_id,
                                          response_request_id);
    if (packet_id != 2u) {
        return handle_cash_error(packet_id);
    }
    response >> transaction.database_id;
    response >> transaction.transaction_id;
    std::printf("[%s] PURCHASE OK, db_id: %u, transaction_id: %u\n",
                "purchase",
                static_cast<unsigned int>(transaction.database_id),
                transaction.transaction_id);
    return 0;
}

// IDA_EA: 0x81AD7AE
// IDA_NAME: _ZN5nexon4cash9NCashImpl8purchaseEyiiPKcS3_RNS0_13NXTransactionEs
// DEMANGLED: nexon::cash::NCashImpl::purchase(unsigned long long,int,int,char const*,char const*,nexon::cash::NXTransaction &,short)
int NCashImpl::purchase(unsigned long long account_id,
                        int product_id,
                        int product_count,
                        const char* account,
                        const char* character,
                        NXTransaction& transaction,
                        short service_override) {
    if (reconnect() != 1) {
        return 2;
    }

    ByteBuffer request = set_data_header(0, 1);
    if (service_override == 60 || service_override == 62) {
        request << service_override;
    } else {
        request << static_cast<short>(service_id_);
    }
    request << account_id;
    request << product_id;
    request << product_count;
    request << account;
    request << character;
    request << cash_type_;
    request << static_cast<short>(shop_id_);

    int result = send_msg(static_cast<unsigned int>(request.length()));
    if (result) {
        return result;
    }
    result = recv_msg();
    if (result) {
        return result;
    }

    unsigned char response_channel = 0;
    unsigned char packet_id = 0;
    unsigned int response_request_id = 0;
    ByteBuffer response = get_data_header(response_channel,
                                          packet_id,
                                          response_request_id);
    if (packet_id != 2u) {
        return handle_cash_error(packet_id);
    }
    response >> transaction.database_id;
    response >> transaction.transaction_id;
    std::printf("[%s] PURCHASE OK, db_id: %u, transaction_id: %u\n",
                "purchase",
                static_cast<unsigned int>(transaction.database_id),
                transaction.transaction_id);
    return 0;
}

// IDA_EA: 0x81AD9C4
// IDA_NAME: _ZN5nexon4cash9NCashImpl4giftEyiiPKcS3_S3_S3_RNS0_13NXTransactionE
// DEMANGLED: nexon::cash::NCashImpl::gift(unsigned long long,int,int,char const*,char const*,char const*,char const*,nexon::cash::NXTransaction &)
int NCashImpl::gift(unsigned long long account_id,
                    int product_id,
                    int product_count,
                    const char* account,
                    const char* character,
                    const char* recipient,
                    const char* message,
                    NXTransaction& transaction) {
    if (reconnect() != 1) {
        return 2;
    }

    ByteBuffer request = set_data_header(0, 7);
    request << static_cast<short>(service_id_);
    request << account_id;
    request << product_id;
    request << product_count;
    request << account;
    request << character;
    request << cash_type_;
    request << static_cast<short>(shop_id_);
    request << recipient;
    request << message;

    int result = send_msg(static_cast<unsigned int>(request.length()));
    if (result) {
        return result;
    }
    result = recv_msg();
    if (result) {
        return result;
    }

    unsigned char response_channel = 0;
    unsigned char packet_id = 0;
    unsigned int response_request_id = 0;
    ByteBuffer response = get_data_header(response_channel,
                                          packet_id,
                                          response_request_id);
    if (packet_id != 2u) {
        return handle_cash_error(packet_id);
    }
    response >> transaction.database_id;
    response >> transaction.transaction_id;
    std::printf("[%s] PURCHASE OK, db_id: %u, transaction_id: %u\n",
                "gift",
                static_cast<unsigned int>(transaction.database_id),
                transaction.transaction_id);
    return 0;
}

// IDA_EA: 0x81ADBD4
// IDA_NAME: _ZN5nexon4cash9NCashImpl7confirmEhj
// DEMANGLED: nexon::cash::NCashImpl::confirm(unsigned char,unsigned int)
int NCashImpl::confirm(unsigned char database_id_low_byte,
                       unsigned int transaction_id) {
    if (!transaction_id) {
        return 0;
    }
    if (reconnect() != 1) {
        return 2;
    }

    ByteBuffer request = set_data_header(0, 3);
    request << database_id_low_byte;
    request << transaction_id;

    int result = send_msg(static_cast<unsigned int>(request.length()));
    if (result) {
        return result;
    }
    result = recv_msg();
    if (result) {
        return result;
    }

    unsigned char response_channel = 0;
    unsigned char packet_id = 0;
    unsigned int response_request_id = 0;
    ByteBuffer response = get_data_header(response_channel,
                                          packet_id,
                                          response_request_id);
    (void)response;
    if (packet_id != 3u) {
        return handle_cash_error(packet_id);
    }
    std::printf("[%s] E_PURCHASE_CONFIRM\n", "confirm");
    return 0;
}

// IDA_EA: 0x81ADD04
// IDA_NAME: _ZN5nexon4cash9NCashImpl6cancelEhj
// DEMANGLED: nexon::cash::NCashImpl::cancel(unsigned char,unsigned int)
int NCashImpl::cancel(unsigned char database_id_low_byte,
                      unsigned int transaction_id) {
    if (!transaction_id) {
        return 0;
    }
    if (reconnect() != 1) {
        return 2;
    }

    ByteBuffer request = set_data_header(0, 4);
    request << database_id_low_byte;
    request << transaction_id;

    int result = send_msg(static_cast<unsigned int>(request.length()));
    if (result) {
        return result;
    }
    result = recv_msg();
    if (result) {
        return result;
    }

    unsigned char response_channel = 0;
    unsigned char packet_id = 0;
    unsigned int response_request_id = 0;
    ByteBuffer response = get_data_header(response_channel,
                                          packet_id,
                                          response_request_id);
    (void)response;
    if (packet_id != 3u) {
        return handle_cash_error(packet_id);
    }
    std::printf("[%s] E_PURCHASE_CONFIRM\n", "cancel");
    return 0;
}

// IDA_EA: 0x81ADE34
// IDA_NAME: _ZN5nexon4cash9NCashImpl10chargeCashEPchhiS2_S2_S2_S2_iRNS0_13NXTransactionE
// DEMANGLED: nexon::cash::NCashImpl::chargeCash(char *,unsigned char,unsigned char,int,char *,char *,char *,char *,int,nexon::cash::NXTransaction &)
int NCashImpl::chargeCash(char* account,
                          unsigned char first_flag,
                          unsigned char second_flag,
                          int amount,
                          char* field_5,
                          char* field_6,
                          char* field_7,
                          char* field_8,
                          int field_9,
                          NXTransaction& transaction) {
    ByteBuffer request = set_data_header(2, 1);
    std::memset(g_cash_charge_scratch.formatted_request.data(),
                0,
                g_cash_charge_scratch.formatted_request.size());
    std::memset(g_cash_charge_scratch.wide_request.data(),
                0,
                sizeof(g_cash_charge_scratch.wide_request));
    std::memset(g_cash_charge_scratch.encrypted_request.data(),
                0,
                sizeof(g_cash_charge_scratch.encrypted_request));
    std::memset(g_cash_charge_scratch.encoded_request.data(),
                0,
                g_cash_charge_scratch.encoded_request.size());

    std::snprintf(g_cash_charge_scratch.formatted_request.data(),
                  g_cash_charge_scratch.formatted_request.size(),
                  "%s|%i|%i|%i|%s|%s|%s|%s|%i",
                  account,
                  static_cast<unsigned int>(first_flag),
                  static_cast<unsigned int>(second_flag),
                  amount,
                  field_5,
                  field_6,
                  field_7,
                  field_8,
                  field_9);

    const size_t source_length =
        std::strlen(g_cash_charge_scratch.formatted_request.data()) + 1u;
    std::mbstowcs(g_cash_charge_scratch.wide_request.data(),
                  g_cash_charge_scratch.formatted_request.data(),
                  source_length);

    const unsigned int adler_input_length = static_cast<unsigned int>(
        2u * std::wcslen(g_cash_charge_scratch.wide_request.data()));
    const unsigned int adler = Encryption::Adler32(
        reinterpret_cast<char*>(g_cash_charge_scratch.wide_request.data()),
        adler_input_length);
    Encryption::EncodeDressStringWide(
        g_cash_charge_scratch.wide_request.data(),
        g_cash_charge_scratch.encrypted_request.data(),
        kChargeCashEncryptionKey);

    const size_t encoded_length_bytes = 2u *
        (std::wcslen(g_cash_charge_scratch.encrypted_request.data()) + 1u);
    (void)encoded_length_bytes;
    const size_t encoded_character_count =
        std::wcslen(g_cash_charge_scratch.encrypted_request.data());
    std::wcstombs(g_cash_charge_scratch.encoded_request.data(),
                  g_cash_charge_scratch.encrypted_request.data(),
                  encoded_character_count);

    request << g_cash_charge_scratch.encoded_request.data();
    request << kChargeCashProtocolConstant;
    request << static_cast<unsigned long long>(adler);

    int result = send_msg(static_cast<unsigned int>(request.length()));
    if (result) {
        return result;
    }
    result = recv_msg();
    if (result) {
        return result;
    }

    unsigned char response_channel = 0;
    unsigned char packet_id = 0;
    unsigned int response_request_id = 0;
    ByteBuffer response = get_data_header(response_channel,
                                          packet_id,
                                          response_request_id);
    if (packet_id != 2u) {
        cMyTrace trace("chargeCash", 771, 0);
        trace("NCashImpl::chargeCash Recv Error!! packet_id:%d, data:%s",
              packet_id,
              g_cash_charge_scratch.formatted_request.data());
        return packet_id;
    }

    response >> transaction.database_id;
    response >> transaction.transaction_id;
    std::printf("[%s] CHARGE_CASH OK, db_id: %u, transaction_id: %u\n",
                "chargeCash",
                static_cast<unsigned int>(transaction.database_id),
                transaction.transaction_id);
    return 0;
}

// IDA_EA: 0x81AE1B0
// IDA_NAME: _ZN5nexon4cash9NexonCashC2EPKcscss
// DEMANGLED: nexon::cash::NexonCash::NexonCash(char const*,short,char,short,short)
NexonCash::NexonCash(const char* host,
                     short port,
                     char cash_type,
                     short shop_id,
                     short service_id)
    : impl_(new NCashImpl(host,
                          port,
                          static_cast<unsigned char>(cash_type),
                          static_cast<unsigned short>(shop_id),
                          service_id)) {
}

// IDA_EA: 0x81AE242
// IDA_NAME: _ZN5nexon4cash9NexonCashD2Ev
// DEMANGLED: nexon::cash::NexonCash::~NexonCash()
NexonCash::~NexonCash() {
    std::free(impl_);
}

// IDA_EA: 0x81AE258
// IDA_NAME: _ZN5nexon4cash9NexonCash10getBalanceEPKcRi
// DEMANGLED: nexon::cash::NexonCash::getBalance(char const*,int &)
int NexonCash::getBalance(const char* account, int& balance) {
    return impl_->getBalance(account, balance);
}

// IDA_EA: 0x81AE27C
// IDA_NAME: _ZN5nexon4cash9NexonCash8purchaseEyiiPKcS3_RNS0_13NXTransactionE
// DEMANGLED: nexon::cash::NexonCash::purchase(unsigned long long,int,int,char const*,char const*,nexon::cash::NXTransaction &)
int NexonCash::purchase(unsigned long long account_id,
                        int product_id,
                        int product_count,
                        const char* account,
                        const char* character,
                        NXTransaction& transaction) {
    return impl_->purchase(account_id,
                           product_id,
                           product_count,
                           account,
                           character,
                           transaction);
}

// IDA_EA: 0x81AE2CE
// IDA_NAME: _ZN5nexon4cash9NexonCash8purchaseEyiiPKcS3_RNS0_13NXTransactionEs
// DEMANGLED: nexon::cash::NexonCash::purchase(unsigned long long,int,int,char const*,char const*,nexon::cash::NXTransaction &,short)
int NexonCash::purchase(unsigned long long account_id,
                        int product_id,
                        int product_count,
                        const char* account,
                        const char* character,
                        NXTransaction& transaction,
                        short service_override) {
    return impl_->purchase(account_id,
                           product_id,
                           product_count,
                           account,
                           character,
                           transaction,
                           service_override);
}

// IDA_EA: 0x81AE330
// IDA_NAME: _ZN5nexon4cash9NexonCash4giftEyiiPKcS3_S3_S3_RNS0_13NXTransactionE
// DEMANGLED: nexon::cash::NexonCash::gift(unsigned long long,int,int,char const*,char const*,char const*,char const*,nexon::cash::NXTransaction &)
int NexonCash::gift(unsigned long long account_id,
                    int product_id,
                    int product_count,
                    const char* account,
                    const char* character,
                    const char* recipient,
                    const char* message,
                    NXTransaction& transaction) {
    return impl_->gift(account_id,
                       product_id,
                       product_count,
                       account,
                       character,
                       recipient,
                       message,
                       transaction);
}

// IDA_EA: 0x81AE390
// IDA_NAME: _ZN5nexon4cash9NexonCash7confirmERKNS0_13NXTransactionE
// DEMANGLED: nexon::cash::NexonCash::confirm(nexon::cash::NXTransaction const&)
int NexonCash::confirm(const NXTransaction& transaction) {
    return impl_->confirm(
        transaction.database_id,
        transaction.transaction_id);
}

// IDA_EA: 0x81AE3BC
// IDA_NAME: _ZN5nexon4cash9NexonCash6cancelERKNS0_13NXTransactionE
// DEMANGLED: nexon::cash::NexonCash::cancel(nexon::cash::NXTransaction const&)
int NexonCash::cancel(const NXTransaction& transaction) {
    return impl_->cancel(
        transaction.database_id,
        transaction.transaction_id);
}

// IDA_EA: 0x81AE440
// IDA_NAME: _ZN5nexon4cash9NexonCash6chargeEPchhiS2_S2_S2_S2_iRNS0_13NXTransactionE
// DEMANGLED: nexon::cash::NexonCash::charge(char *,unsigned char,unsigned char,int,char *,char *,char *,char *,int,nexon::cash::NXTransaction &)
int NexonCash::charge(char* account,
                      unsigned char first_flag,
                      unsigned char second_flag,
                      int amount,
                      char* field_5,
                      char* field_6,
                      char* field_7,
                      char* field_8,
                      int field_9,
                      NXTransaction& transaction) {
    return impl_->chargeCash(account,
                             first_flag,
                             second_flag,
                             amount,
                             field_5,
                             field_6,
                             field_7,
                             field_8,
                             field_9,
                             transaction);
}

// IDA_EA: 0x81AE4AE
// IDA_NAME: _ZN5nexon4cash9NexonCash13checkEntranceEPKcRc
// DEMANGLED: nexon::cash::NexonCash::checkEntrance(char const*,char &)
int NexonCash::checkEntrance(const char* account, char& entrance) {
    return impl_->checkEntrance(account, entrance);
}

// IDA_EA: 0x81AE3E8
// IDA_NAME: _ZN5nexon4cash9NexonCash11checkCouponEv
// DEMANGLED: nexon::cash::NexonCash::checkCoupon(void)
int NexonCash::checkCoupon() {
    return impl_->checkCoupon();
}

// IDA_EA: 0x81AE3FE
// IDA_NAME: _ZN5nexon4cash9NexonCash9useCouponEv
// DEMANGLED: nexon::cash::NexonCash::useCoupon(void)
int NexonCash::useCoupon() {
    return impl_->useCoupon();
}

// IDA_EA: 0x81AE414
// IDA_NAME: _ZN5nexon4cash9NexonCash15checkCafeCouponEv
// DEMANGLED: nexon::cash::NexonCash::checkCafeCoupon(void)
int NexonCash::checkCafeCoupon() {
    return impl_->checkCafeCoupon();
}

// IDA_EA: 0x81AE42A
// IDA_NAME: _ZN5nexon4cash9NexonCash13useCafeCouponEv
// DEMANGLED: nexon::cash::NexonCash::useCafeCoupon(void)
int NexonCash::useCafeCoupon() {
    return impl_->useCafeCoupon();
}

// IDA_EA: 0x81AEA46
// IDA_NAME: _ZN5nexon4cash9NCashImpl8recv_msgEv
// DEMANGLED: nexon::cash::NCashImpl::recv_msg(void)
int NCashImpl::recv_msg() {
    char* const buffer = incoming_message_.data();
    const ssize_t received = ::recv(cash_socket_.getHandle(),
                                    buffer,
                                    3u,
                                    MSG_WAITALL);
    if (received) {
        if (received >= 0) {
            incoming_message_.decode_header();
            return recv_body();
        }
        std::printf("[%s] recv error: %s\n", "recv_msg", std::strerror(errno));
    } else {
        std::printf("[%s] connection lost: %s\n",
                    "recv_msg",
                    std::strerror(errno));
    }

    connected_ = 0;
    cash_socket_.close();
    return 2;
}

// IDA_EA: 0x81AEB3C
// IDA_NAME: _ZN5nexon4cash9NCashImpl9recv_bodyEv
// DEMANGLED: nexon::cash::NCashImpl::recv_body(void)
int NCashImpl::recv_body() {
    const size_t body_length = static_cast<size_t>(incoming_message_.body_length());
    char* const buffer = incoming_message_.body();
    const ssize_t received = ::recv(cash_socket_.getHandle(),
                                    buffer,
                                    body_length,
                                    MSG_WAITALL);
    if (received) {
        if (received >= 0) {
            return 0;
        }
        std::printf("[%s] recv error: %s\n", "recv_body", std::strerror(errno));
    } else {
        std::printf("[%s] connection lost: %s\n",
                    "recv_body",
                    std::strerror(errno));
    }

    connected_ = 0;
    cash_socket_.close();
    return 2;
}

// IDA_EA: 0x81AEC2C
// IDA_NAME: _ZN5nexon4cash9NCashImpl8send_msgEj
// DEMANGLED: nexon::cash::NCashImpl::send_msg(unsigned int)
int NCashImpl::send_msg(unsigned int body_length) {
    outgoing_message_.encode_header(body_length);
    const size_t message_length =
        static_cast<size_t>(outgoing_message_.length());
    char* const buffer = outgoing_message_.data();
    const ssize_t sent = ::send(cash_socket_.getHandle(),
                                buffer,
                                message_length,
                                MSG_WAITALL);
    if (outgoing_message_.length() == sent) {
        return 0;
    }

    std::printf("send error: send bytes: %d, message length: %d, error: %s\n",
                static_cast<int>(sent),
                outgoing_message_.length(),
                std::strerror(errno));
    connected_ = 0;
    cash_socket_.close();
    return 2;
}

}  // namespace cash
}  // namespace nexon
