#include "old_game/economy/mall/auction_server_proxy_abi.h"

#include "common/manager/global_data.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"

CAuctionServerProxy* GlobalData::s_auction_proxy = 0;
CCeraAuctionServerProxy* GlobalData::s_cera_auction_proxy = 0;

namespace nsl {

// IDA_EA: 0x822E4D2
// IDA_NAME: _ZN3nsl13PACKET_HEADERC2Ev
// DEMANGLED: nsl::PACKET_HEADER::PACKET_HEADER(void)
PACKET_HEADER::PACKET_HEADER() {}

// IDA_EA: 0x822E490
// IDA_NAME: _ZN3nsl13PACKET_HEADER11setPacketIDEi
// DEMANGLED: nsl::PACKET_HEADER::setPacketID(int)
PACKET_HEADER* PACKET_HEADER::setPacketID(int packet_id_arg) {
    packet_id = static_cast<uint8_t>(packet_id_arg);
    return this;
}

// IDA_EA: 0x822E4A0
// IDA_NAME: _ZN3nsl13PACKET_HEADER7setSizeEi
// DEMANGLED: nsl::PACKET_HEADER::setSize(int)
PACKET_HEADER* PACKET_HEADER::setSize(int size_arg) {
    size = static_cast<uint32_t>(size_arg);
    return this;
}

// IDA_EA: 0x822E4AE
// IDA_NAME: _ZN3nsl13PACKET_HEADER11setCategoryEi
// DEMANGLED: nsl::PACKET_HEADER::setCategory(int)
PACKET_HEADER* PACKET_HEADER::setCategory(int category_arg) {
    category = static_cast<uint8_t>(category_arg);
    return this;
}

// IDA_EA: 0x82F7D0C
// IDA_NAME: _ZN3nsl13PACKET_HEADER11getPacketIDEv
// DEMANGLED: nsl::PACKET_HEADER::getPacketID(void)
int PACKET_HEADER::getPacketID() {
    return static_cast<int>(static_cast<int8_t>(packet_id));
}

// IDA_EA: 0x82F7D1C
// IDA_NAME: _ZN3nsl13PACKET_HEADER7getSizeEv
// DEMANGLED: nsl::PACKET_HEADER::getSize(void)
int PACKET_HEADER::getSize() {
    return static_cast<int>(size);
}

// IDA_EA: 0x82F7D28
// IDA_NAME: _ZN3nsl13PACKET_HEADER11getCategoryEv
// DEMANGLED: nsl::PACKET_HEADER::getCategory(void)
int PACKET_HEADER::getCategory() {
    return static_cast<int>(static_cast<int8_t>(category));
}

}  // namespace nsl

namespace {

struct AuctionReadyPacket : public nsl::PACKET_HEADER {
    uint8_t reserved[12];
};

struct AuctionRegistPacket : public nsl::PACKET_HEADER {
    uint8_t reserved[16];
};

static_assert(sizeof(AuctionReadyPacket) == 18u,
              "auction ready packet IDA ABI size must stay 18");
static_assert(sizeof(AuctionRegistPacket) == 22u,
              "auction regist packet IDA ABI size must stay 22");

}  // namespace

// IDA_EA: 0x8234F44
// IDA_NAME: _ZN19CAuctionServerProxy9IsRunningEv
// DEMANGLED: CAuctionServerProxy::IsRunning(void)
int CAuctionServerProxy::IsRunning() {
    return running_;
}

// IDA_EA: 0x82AA9AC
// IDA_NAME: _ZN19CAuctionServerProxyD2Ev
// DEMANGLED: CAuctionServerProxy::~CAuctionServerProxy()
CAuctionServerProxy::~CAuctionServerProxy() {}

// IDA_EA: 0x82F7A90
// IDA_NAME: _ZN19CAuctionServerProxyC2EPciS0_
// DEMANGLED: CAuctionServerProxy::CAuctionServerProxy(char *,int,char *)
CAuctionServerProxy::CAuctionServerProxy(char* auction_server_ip,
                                         int auction_server_port,
                                         char* server_name)
    : BaseServerProxy(auction_server_ip, auction_server_port, server_name),
      running_(0),
      reserved_running_pad_() {
    Init();
}

// IDA_EA: 0x82F7AD6
// IDA_NAME: _ZN19CAuctionServerProxy4InitEv
// DEMANGLED: CAuctionServerProxy::Init(void)
CAuctionServerProxy* CAuctionServerProxy::Init() {
    running_ = 0;
    return this;
}

// IDA_EA: 0x82F7AE6
// IDA_NAME: _ZN19CAuctionServerProxy12OnDisconnectEv
// DEMANGLED: CAuctionServerProxy::OnDisconnect(void)
CAuctionServerProxy* CAuctionServerProxy::OnDisconnect() {
    return Init();
}

// IDA_EA: 0x82F7AFA
// IDA_NAME: _ZN19CAuctionServerProxy7ParsingEi
// DEMANGLED: CAuctionServerProxy::Parsing(int)
int CAuctionServerProxy::Parsing(int received_bytes) {
    return GetTcpHandler()->Parsing_AuctionServerPacket(received_bytes);
}

// IDA_EA: 0x82F7B1C
// IDA_NAME: _ZN19CAuctionServerProxy10SendPacketEPN3nsl13PACKET_HEADERE
// DEMANGLED: CAuctionServerProxy::SendPacket(nsl::PACKET_HEADER *)
int CAuctionServerProxy::SendPacket(nsl::PACKET_HEADER* packet) {
    return SendTcpPacket(reinterpret_cast<char*>(packet), packet->getSize());
}

// IDA_EA: 0x82F7B46
// IDA_NAME: _ZN19CAuctionServerProxy16SendRegistPacketEv
// DEMANGLED: CAuctionServerProxy::SendRegistPacket(void)
int CAuctionServerProxy::SendRegistPacket() {
    AuctionRegistPacket packet;
    packet.setCategory(0);
    packet.setPacketID(0);
    packet.setSize(sizeof(packet));
    return SendPacket(&packet);
}

// IDA_EA: 0x82F7B7A
// IDA_NAME: _ZN19CAuctionServerProxy27SendCheckAuctionReadyPacketEv
// DEMANGLED: CAuctionServerProxy::SendCheckAuctionReadyPacket(void)
int CAuctionServerProxy::SendCheckAuctionReadyPacket() {
    AuctionReadyPacket packet;
    packet.setCategory(0);
    packet.setPacketID(13);
    packet.setSize(sizeof(packet));
    return SendPacket(&packet);
}

// IDA_EA: 0x84ED344
// IDA_NAME: _ZN19CAuctionServerProxy10SetRunningEb
// DEMANGLED: CAuctionServerProxy::SetRunning(bool)
CAuctionServerProxy* CAuctionServerProxy::SetRunning(bool running) {
    running_ = static_cast<uint8_t>(running);
    return this;
}

// IDA_EA: 0x8234F54
// IDA_NAME: _ZN23CCeraAuctionServerProxy9IsRunningEv
// DEMANGLED: CCeraAuctionServerProxy::IsRunning(void)
int CCeraAuctionServerProxy::IsRunning() {
    return running_;
}

// IDA_EA: 0x82AA9C8
// IDA_NAME: _ZN23CCeraAuctionServerProxyD2Ev
// DEMANGLED: CCeraAuctionServerProxy::~CCeraAuctionServerProxy()
CCeraAuctionServerProxy::~CCeraAuctionServerProxy() {}

// IDA_EA: 0x82F7BA0
// IDA_NAME: _ZN23CCeraAuctionServerProxyC2EPciS0_
// DEMANGLED: CCeraAuctionServerProxy::CCeraAuctionServerProxy(char *,int,char *)
CCeraAuctionServerProxy::CCeraAuctionServerProxy(
    char* cera_auction_server_ip,
    int cera_auction_server_port,
    char* server_name)
    : BaseServerProxy(cera_auction_server_ip,
                      cera_auction_server_port,
                      server_name),
      running_(0),
      reserved_running_pad_() {
    Init();
}

// IDA_EA: 0x82F7BE6
// IDA_NAME: _ZN23CCeraAuctionServerProxy4InitEv
// DEMANGLED: CCeraAuctionServerProxy::Init(void)
CCeraAuctionServerProxy* CCeraAuctionServerProxy::Init() {
    running_ = 0;
    return this;
}

// IDA_EA: 0x82F7BF6
// IDA_NAME: _ZN23CCeraAuctionServerProxy12OnDisconnectEv
// DEMANGLED: CCeraAuctionServerProxy::OnDisconnect(void)
CCeraAuctionServerProxy* CCeraAuctionServerProxy::OnDisconnect() {
    return Init();
}

// IDA_EA: 0x82F7C0A
// IDA_NAME: _ZN23CCeraAuctionServerProxy7ParsingEi
// DEMANGLED: CCeraAuctionServerProxy::Parsing(int)
int CCeraAuctionServerProxy::Parsing(int received_bytes) {
    return GetTcpHandler()->Parsing_AuctionServerPacket(received_bytes);
}

// IDA_EA: 0x82F7C2C
// IDA_NAME: _ZN23CCeraAuctionServerProxy10SendPacketEPN3nsl13PACKET_HEADERE
// DEMANGLED: CCeraAuctionServerProxy::SendPacket(nsl::PACKET_HEADER *)
int CCeraAuctionServerProxy::SendPacket(nsl::PACKET_HEADER* packet) {
    return SendTcpPacket(reinterpret_cast<char*>(packet), packet->getSize());
}

// IDA_EA: 0x82F7C56
// IDA_NAME: _ZN23CCeraAuctionServerProxy16SendRegistPacketEv
// DEMANGLED: CCeraAuctionServerProxy::SendRegistPacket(void)
int CCeraAuctionServerProxy::SendRegistPacket() {
    AuctionRegistPacket packet;
    packet.setCategory(18);
    packet.setPacketID(0);
    packet.setSize(sizeof(packet));
    return SendPacket(&packet);
}

// IDA_EA: 0x82F7C8A
// IDA_NAME: _ZN23CCeraAuctionServerProxy31SendCheckCeraAuctionReadyPacketEv
// DEMANGLED: CCeraAuctionServerProxy::SendCheckCeraAuctionReadyPacket(void)
int CCeraAuctionServerProxy::SendCheckCeraAuctionReadyPacket() {
    AuctionReadyPacket packet;
    packet.setCategory(18);
    packet.setPacketID(13);
    packet.setSize(sizeof(packet));
    return SendPacket(&packet);
}

// IDA_EA: 0x84ED360
// IDA_NAME: _ZN23CCeraAuctionServerProxy10SetRunningEb
// DEMANGLED: CCeraAuctionServerProxy::SetRunning(bool)
CCeraAuctionServerProxy* CCeraAuctionServerProxy::SetRunning(bool running) {
    running_ = static_cast<uint8_t>(running);
    return this;
}
