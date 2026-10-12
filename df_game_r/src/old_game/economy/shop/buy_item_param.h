#ifndef OLDSRV_OLD_GAME_NPC_BUY_ITEM_PARAM_H_
#define OLDSRV_OLD_GAME_NPC_BUY_ITEM_PARAM_H_

#include <stdint.h>
#include <utility>

#include "old_game/economy/inventory/inventory.h"
#include "infra/network/packet/message_base.h"

class CItem;

// IDA unit: 99_misc_global/BuyItemParam.  The state-3 CMD21 template places
// this exact 0xb7-byte result object on its stack.
// GCC ignores __attribute__((packed)) for the non-POD Inven_Item members;
// the old 0xb7-byte local object requires the surrounding pack pragma.
#pragma pack(push, 1)
struct BuyItemParam : public ParamBase {
    // IDA_EA: 0x8230906
    // IDA_NAME: _ZN12BuyItemParamC2Ev
    // DEMANGLED: BuyItemParam::BuyItemParam(void)
    BuyItemParam();

    // IDA_EA: 0x82484A6
    // IDA_NAME: _ZN12BuyItemParamD2Ev
    // DEMANGLED: BuyItemParam::~BuyItemParam()
    // IDA_EA: 0x82484D6
    // IDA_NAME: _ZN12BuyItemParamD0Ev
    // DEMANGLED: BuyItemParam::~BuyItemParam()
    virtual ~BuyItemParam();

    Inven_Item item_;                       // IDA +0x04
    Inven_Item material_;                   // IDA +0x41
    uint8_t padding_7e_[2];                 // IDA +0x7e
    int price_;                             // IDA +0x80
    int cera_price_;                        // IDA +0x84
    int win_point_price_;                   // IDA +0x88
    int finish_point_price_;                // IDA +0x8c
    int guild_power_cost_;                  // IDA +0x90
    int user_power_cost_;                   // IDA +0x94
    std::pair<int, int> material_requirement_;  // IDA +0x98
    int inserted_slot_;                     // IDA +0xa0
    int material_slot_;                     // IDA +0xa4
    CItem* prototype_;                      // IDA +0xa8
    CItem* material_prototype_;             // IDA +0xac
    int sell_price_;                        // IDA +0xb0
    uint8_t padding_b4_[3];                 // IDA stack extent +0xb4
};
#pragma pack(pop)

#if defined(__i386__)
static_assert(sizeof(BuyItemParam) == 0x0b7u,
              "CMD21 parameter must retain the old 183-byte ABI");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_BUY_ITEM_PARAM_H_
