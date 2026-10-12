#include "old_game/economy/shop/buy_item_param.h"

// IDA_EA: 0x8230906
// IDA_NAME: _ZN12BuyItemParamC2Ev
// DEMANGLED: BuyItemParam::BuyItemParam(void)
BuyItemParam::BuyItemParam()
    : ParamBase(),
      item_(),
      material_(),
      price_(0),
      cera_price_(0),
      win_point_price_(0),
      finish_point_price_(0),
      guild_power_cost_(0),
      user_power_cost_(0),
      material_requirement_(),
      inserted_slot_(-1),
      material_slot_(-1),
      prototype_(0),
      material_prototype_(0),
      sell_price_(0) {}

// IDA_EA: 0x82484A6
// IDA_NAME: _ZN12BuyItemParamD2Ev
// DEMANGLED: BuyItemParam::~BuyItemParam()
// IDA_EA: 0x82484D6
// IDA_NAME: _ZN12BuyItemParamD0Ev
// DEMANGLED: BuyItemParam::~BuyItemParam()
BuyItemParam::~BuyItemParam() {}
