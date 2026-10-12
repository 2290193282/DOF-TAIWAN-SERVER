#ifndef OLDSRV_GLOBAL_AUCTION_SERVER_PROXY_ABI_H_
#define OLDSRV_GLOBAL_AUCTION_SERVER_PROXY_ABI_H_

#include <stdint.h>

#include "common/manager/base_server_proxy_abi.h"

namespace nsl {

#pragma pack(push, 1)
// IDA unit: 99_misc_global/nsl/PACKET_HEADER.
// IDA_EA: set/get/ctor @0x822E490/@0x822E4A0/@0x822E4AE/
// @0x822E4D2/@0x82F7D0C/@0x82F7D1C/@0x82F7D28.
struct PACKET_HEADER {
    PACKET_HEADER();

    PACKET_HEADER* setPacketID(int packet_id);
    PACKET_HEADER* setSize(int size);
    PACKET_HEADER* setCategory(int category);
    int getPacketID();
    int getSize();
    int getCategory();

    uint8_t category;
    uint8_t packet_id;
    uint32_t size;
};
#pragma pack(pop)

}  // namespace nsl

static_assert(sizeof(nsl::PACKET_HEADER) == 0x06u,
              "nsl::PACKET_HEADER IDA ABI size must stay 0x06");

// IDA unit: 10_mail_trade_auction_cash/CAuctionServerProxy.
// IDA_EA: methods @0x8234F44/@0x82AA9AC/@0x82F7A90/@0x82F7AD6/
// @0x82F7AE6/@0x82F7AFA/@0x82F7B1C/@0x82F7B46/@0x82F7B7A/@0x84ED344.
class CAuctionServerProxy : public BaseServerProxy {
public:
    CAuctionServerProxy(char* auction_server_ip,
                        int auction_server_port,
                        char* server_name);
    virtual ~CAuctionServerProxy();

    int IsRunning();
    CAuctionServerProxy* Init();
    virtual CAuctionServerProxy* OnDisconnect();
    virtual int Parsing(int received_bytes);
    int SendPacket(nsl::PACKET_HEADER* packet);
    int SendRegistPacket();
    int SendCheckAuctionReadyPacket();
    CAuctionServerProxy* SetRunning(bool running);

private:
    uint8_t running_;
    uint8_t reserved_running_pad_[3];
};

// IDA unit: 10_mail_trade_auction_cash/CCeraAuctionServerProxy.
// IDA_EA: methods @0x8234F54/@0x82AA9C8/@0x82F7BA0/@0x82F7BE6/
// @0x82F7BF6/@0x82F7C0A/@0x82F7C2C/@0x82F7C56/@0x82F7C8A/@0x84ED360.
class CCeraAuctionServerProxy : public BaseServerProxy {
public:
    CCeraAuctionServerProxy(char* cera_auction_server_ip,
                            int cera_auction_server_port,
                            char* server_name);
    virtual ~CCeraAuctionServerProxy();

    int IsRunning();
    CCeraAuctionServerProxy* Init();
    virtual CCeraAuctionServerProxy* OnDisconnect();
    virtual int Parsing(int received_bytes);
    int SendPacket(nsl::PACKET_HEADER* packet);
    int SendRegistPacket();
    int SendCheckCeraAuctionReadyPacket();
    CCeraAuctionServerProxy* SetRunning(bool running);

private:
    uint8_t running_;
    uint8_t reserved_running_pad_[3];
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CAuctionServerProxy) == 0x30E10u,
              "CAuctionServerProxy IDA ABI size must stay 0x30E10");
static_assert(sizeof(CCeraAuctionServerProxy) == 0x30E10u,
              "CCeraAuctionServerProxy IDA ABI size must stay 0x30E10");
#endif

#endif  // OLDSRV_GLOBAL_AUCTION_SERVER_PROXY_ABI_H_
