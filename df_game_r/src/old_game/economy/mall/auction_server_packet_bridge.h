#ifndef OLDSRV_OLD_GAME_ECONOMY_MALL_AUCTION_SERVER_PACKET_BRIDGE_H_
#define OLDSRV_OLD_GAME_ECONOMY_MALL_AUCTION_SERVER_PACKET_BRIDGE_H_

// Converts one complete Auction/Point-Auction service frame into the
// normalized InterDispatcher stream consumed by the game-server handlers.
// The CNetwork auction parser has already split the TCP stream at this point.
bool DispatchAuctionServerPacket(const char* packet, int packet_size);

// Installs DispatchAuctionServerPacket in CNetwork's auction callback slot.
// This must run before either auction proxy starts receiving TCP data.
void InstallAuctionServerPacketBridge();

#endif  // OLDSRV_OLD_GAME_ECONOMY_MALL_AUCTION_SERVER_PACKET_BRIDGE_H_
