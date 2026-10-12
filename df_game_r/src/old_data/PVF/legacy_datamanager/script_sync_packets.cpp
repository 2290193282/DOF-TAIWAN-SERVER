#include "old_data/PVF/data/datamanager/script_sync_packets.h"

// IDA_EA: 0x8106C48
// IDA_NAME: _ZN16tagCS_GET_SCRIPTC2Ev
// DEMANGLED: tagCS_GET_SCRIPT::tagCS_GET_SCRIPT(void)
tagCS_GET_SCRIPT::tagCS_GET_SCRIPT() : tagPacketHeader() {
    setCategory(2);
    setPacketID(9);
    setSize(11);
}

// IDA_EA: 0x8106C94
// IDA_NAME: _ZN26tagCS_CHECK_SCRIPT_VERSIONC2Ev
// DEMANGLED: tagCS_CHECK_SCRIPT_VERSION::tagCS_CHECK_SCRIPT_VERSION(void)
tagCS_CHECK_SCRIPT_VERSION::tagCS_CHECK_SCRIPT_VERSION() : tagPacketHeader() {
    setCategory(1);
    setPacketID(5);
    setSize(27);
}

// IDA_EA: 0x8106BFC
// IDA_NAME: _ZN25tagCS_UPDATE_CHANNEL_INFOC2Ev
// DEMANGLED: tagCS_UPDATE_CHANNEL_INFO::tagCS_UPDATE_CHANNEL_INFO(void)
tagCS_UPDATE_CHANNEL_INFO::tagCS_UPDATE_CHANNEL_INFO() : tagPacketHeader() {
    setCategory(1);
    setPacketID(2);
    setSize(51);
}
