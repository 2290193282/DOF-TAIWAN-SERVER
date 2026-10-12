#include "common/manager/mannerless_user_accusation_packet.h"

#include <cstring>

// IDA_EA: 0x822C1CA
// IDA_NAME: _ZN33Packet_Mannerless_User_Accusation24isNeedTypeForChattingMsgEi
// DEMANGLED: Packet_Mannerless_User_Accusation::isNeedTypeForChattingMsg(int)
bool Packet_Mannerless_User_Accusation::isNeedTypeForChattingMsg(
    int accusation_type) {
    return static_cast<unsigned int>(accusation_type) <= 2u;
}

// IDA_EA: 0x822C1E6
// IDA_NAME: _ZN33Packet_Mannerless_User_AccusationC2Ev
// DEMANGLED: Packet_Mannerless_User_Accusation::Packet_Mannerless_User_Accusation(void)
Packet_Mannerless_User_Accusation::Packet_Mannerless_User_Accusation()
    : header(0x1B66u, 0x015Fu),
      uid(-1),
      charac_no(0),
      accusation_type(4),
      reporter_acc_id(0),
      result_or_status(0) {
    // The two server-group dwords are deliberately not initialized: the old
    // C2 leaves them untouched and CMD140 writes them after it parses the
    // client-selected group byte.
    std::memset(reporter_name, 0, sizeof(reporter_name));
    std::memset(accused_name, 0, sizeof(accused_name));
    std::memset(chat_message, 0, sizeof(chat_message));
}
