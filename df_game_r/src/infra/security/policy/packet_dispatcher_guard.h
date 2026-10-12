#ifndef OLDSRV_GLOBAL_PACKET_DISPATCHER_GUARD_H_
#define OLDSRV_GLOBAL_PACKET_DISPATCHER_GUARD_H_


// Packet-dispatch failure policy exported by the old global unit.
//
// LineFunc is intentionally declared here for future packet-dispatch users,
// but its sole canonical definition remains global/global_tools.cpp.  That
// provider already has the exact old IDA identity and log contract; defining
// it again here would create a duplicate global symbol.
// IDA_EA: 0x85908D7
// IDA_NAME: _Z8LineFunciPKcij
// DEMANGLED: LineFunc(int,char const*,int,unsigned int)
int LineFunc(int line, const char* function, int error_no, unsigned int mid);

// IDA_EA: 0x859092A
// IDA_NAME: _Z10DenyPacketi
// DEMANGLED: DenyPacket(int)
int DenyPacket(int packet_type);


#endif  // OLDSRV_GLOBAL_PACKET_DISPATCHER_GUARD_H_
