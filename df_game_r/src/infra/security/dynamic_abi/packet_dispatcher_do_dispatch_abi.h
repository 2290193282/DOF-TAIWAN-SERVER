#ifndef OLDSRV_SECURITY_PACKET_DISPATCHER_DO_DISPATCH_ABI_H_
#define OLDSRV_SECURITY_PACKET_DISPATCHER_DO_DISPATCH_ABI_H_

#include <stdint.h>

#include "infra/network/packet/PacketHeader.h"

class CUser;
class PacketDispatcher;

// Declaration-only bridge for the old non-static member ABI. This does not
// define PacketDispatcher, its layout, or doDispatch; the first argument is
// the original this pointer and the asm name is the raw old-server member
// symbol. Its only CMD604 use is the faithful inner DPROTO re-dispatch.
//
#endif  // OLDSRV_SECURITY_PACKET_DISPATCHER_DO_DISPATCH_ABI_H_
