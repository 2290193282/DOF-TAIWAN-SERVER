#ifndef OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_ADVANCE_ALTAR_BUY_ITEM_H_
#define OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_ADVANCE_ALTAR_BUY_ITEM_H_

#include <stdint.h>

#include "old_game/activity/schedule/advance_altar_dispatchers.h"

namespace CMDPacketStruct {

// IDA unit: 13_database_dispatch/advancealtar/_ReqAdvanceAltarBuyItem.
struct _ReqAdvanceAltarBuyItem : public STBaseRequest {
    _ReqAdvanceAltarBuyItem();

    uint8_t raw_00[0x10];
    // IDA process_packet@0x813FC06 passes +0x10 as the ridable id and
    // +0x14 as AdvanceAltarShopType.  Keeping the offset in each name makes
    // the wire ABI explicit without changing the 0x20-byte request layout.
    uint32_t ridable_id_10;
    uint32_t shop_type_14;
    uint32_t upgrade_id_18;
    int16_t first_level_1c;
    int16_t last_level_1e;
};

// IDA unit: 13_database_dispatch/advancealtar/_ResAdvanceAltarBuyItem.
struct _ResAdvanceAltarBuyItem : public STBaseResponse {
    _ResAdvanceAltarBuyItem();
    ~_ResAdvanceAltarBuyItem();

    int32_t result_04;
    uint32_t ridable_id_08;
    uint32_t shop_type_0c;
    uint32_t upgrade_id_10;
    int16_t first_level_14;
    int16_t last_level_16;
    int32_t remaining_star_18;
};

#if defined(__i386__)
static_assert(sizeof(_ReqAdvanceAltarBuyItem) == 0x20u,
              "CMD573 request must retain its 32-byte wire extent");
static_assert(sizeof(_ResAdvanceAltarBuyItem) == 0x1cu,
              "CMD573 response must retain its 28-byte wire extent");
#endif

}  // namespace CMDPacketStruct

namespace advancealtar {

// IDA unit: 13_database_dispatch/advancealtar/Dispatcher_BuyItem.
class Dispatcher_BuyItem
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
public:
    // IDA_EA: 0x85956EC
    // IDA_NAME: _ZN12advancealtar18Dispatcher_BuyItemC2Ev
    Dispatcher_BuyItem();
    // IDA_EA: 0x81421D8 / 0x8142208
    // IDA_NAME: _ZN12advancealtar18Dispatcher_BuyItemD2Ev / _ZN12advancealtar18Dispatcher_BuyItemD0Ev
    virtual ~Dispatcher_BuyItem();

    // IDA_EA: 0x813FAD0
    // IDA_NAME: _ZN12advancealtar18Dispatcher_BuyItem12dispatch_sigEP5CUserR9PacketBuf
    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    int read_packet(PacketBuf& packet,
                    CMDPacketStruct::STBaseRequest& request) const;
    int check_error(CUser* user,
                    CMDPacketStruct::STBaseRequest& request) const;
    int process_packet(CUser* user,
                       CMDPacketStruct::STBaseRequest& request,
                       CMDPacketStruct::STBaseResponse& response) const;
    void send_packet(CUser* user, CMDPacketStruct::STBaseResponse& response);

    // IDA_EA: 0x8142226 / 0x8142230.  The direct 573 transaction owns the
    // wire behavior, while these inherited slots remain the original zero
    // leaves for the generic dispatcher interface.
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

}  // namespace advancealtar

#endif  // OLDSRV_OLD_GAME_EVENT_PACKET_DISPATCHER_ADVANCE_ALTAR_BUY_ITEM_H_
