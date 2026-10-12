#ifndef OLDSRV_GLOBAL_HADES_SERVER_PACKET_BRIDGE_H_
#define OLDSRV_GLOBAL_HADES_SERVER_PACKET_BRIDGE_H_

// The restored CNetwork server-frame parser passes each complete frame through
// one process-wide callback.  This is the concrete recovered subset of the
// original CDispatchServerPacket router: it preserves the Hades action/
// reconnect frames, monitor registration handshake, and the monitor login
// reply that releases character select.
bool DispatchServerPacket(const char* packet, int packet_size);

// Historical narrow entry kept for existing Hades callers.  It delegates to
// DispatchServerPacket so all CNetwork<100000,100000> proxy connections use
// the same original server-frame routing boundary.
void DispatchHadesServerPacket(const char* packet, int packet_size);

// Install the recovered generic bridge in the CNetwork server-packet callback
// slot.  The historical name is retained because GlobalData's existing
// bootstrap owns the installation point.  This does not create an I/O loop:
// the original MonitorTcpThread remains the owner of proxy readiness.
void InstallHadesServerPacketBridge();

#endif  // OLDSRV_GLOBAL_HADES_SERVER_PACKET_BRIDGE_H_
