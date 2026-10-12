#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_ITEM_ACTIONS_ABI_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_ITEM_ACTIONS_ABI_H_

class CUser;
class MSG_STATIC_COMMAND;

namespace expert_job {
class CExpertJob;
}

// Narrow declaration-only ABI boundary for the named old-server enchanter
// members used by CMD171/CMD173. Their CExpertJob receiver is proven at
// CEnchanter+0 by CEnchanter::C2, without declaring either object layout.
namespace oldsrv_inventory_enchant_abi {

// Old-server callee EA 0x849E07E,
// _ZN10expert_job10CEnchanter14OnAgreeEnchantEP5CUserP18MSG_STATIC_COMMAND.
// CEnchanter::C2 at 0x849F2C2 constructs CExpertJob at receiver+0. This
// narrow member ABI therefore accepts the proven base pointer as the exact
// physical receiver address, without declaring CEnchanter's derived layout.
// Old-server callee EA 0x849DF5E,
// _ZN10expert_job10CEnchanter19OnPutItemForEnchantEP5CUserP18MSG_STATIC_COMMAND.
}  // namespace oldsrv_inventory_enchant_abi

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_ITEM_ACTIONS_ABI_H_
