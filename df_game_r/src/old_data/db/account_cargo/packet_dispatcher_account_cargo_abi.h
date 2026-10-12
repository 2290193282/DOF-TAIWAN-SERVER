#ifndef OLDSRV_OLD_DATA_DB_ACCOUNT_CARGO_PACKET_DISPATCHER_ACCOUNT_CARGO_ABI_H_
#define OLDSRV_OLD_DATA_DB_ACCOUNT_CARGO_PACKET_DISPATCHER_ACCOUNT_CARGO_ABI_H_

class CUser;

// Exact static account-cargo service entry points.  The service bodies and
// their DB callback chain are intentionally left as named link debt rather
// than replaced by an invented cargo implementation.
namespace oldsrv_account_cargo_packet_abi {

// old-server callee EA 0x828AD72, _ZN13CAccountCargo18CreateAccountCargoEP5CUser
// old-server callee EA 0x828B0B2, _ZN13CAccountCargo19UpgradeAccountCargoEP5CUser
}  // namespace oldsrv_account_cargo_packet_abi

#endif  // OLDSRV_OLD_DATA_DB_ACCOUNT_CARGO_PACKET_DISPATCHER_ACCOUNT_CARGO_ABI_H_
