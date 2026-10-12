#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8686DD0
// IDA_NAME: _ZN15cUserHistoryLog16AntibotPacketLogEN20AntibotPacketLogType1TE
// DEMANGLED: cUserHistoryLog::AntibotPacketLog(AntibotPacketLogType::T)
int cUserHistoryLog::AntibotPacketLog(
    AntibotPacketLogType::T event_type) {
    return owner_->LogHistory("AntibotPacketLog,%d",
                              static_cast<int>(event_type));
}

// IDA_EA: 0x8686E3A
// IDA_NAME: _ZN15cUserHistoryLog21SendBuffertoClientLogEishb
// DEMANGLED: cUserHistoryLog::SendBuffertoClientLog(int,short,unsigned char,bool)
int cUserHistoryLog::SendBuffertoClientLog(int destination_kind,
                                           int16_t buffer_size,
                                           uint8_t attempt_count,
                                           bool send_result) {
    return owner_->LogHistory("SendBuffertoClient [%s],%d,%d,%d",
                              destination_kind != 0 ? "DPROTO" : "ANTIBOT",
                              static_cast<int>(buffer_size),
                              static_cast<int>(attempt_count),
                              send_result ? 1 : 0);
}

// IDA_EA: 0x8686DF4
// IDA_NAME: _ZN15cUserHistoryLog12AbnormalUserE27ENUM_ABNORMAL_USER_LOG_TYPEtii
// DEMANGLED: cUserHistoryLog::AbnormalUser(ENUM_ABNORMAL_USER_LOG_TYPE,unsigned short,int,int)
int cUserHistoryLog::AbnormalUser(ENUM_ABNORMAL_USER_LOG_TYPE log_type,
                                  uint16_t command,
                                  int value,
                                  int detail) {
    return owner_->LogHistory("AbnormalUser,%u,%hu,%d,%d",
                              static_cast<unsigned int>(log_type),
                              static_cast<unsigned int>(command),
                              value,
                              detail);
}
