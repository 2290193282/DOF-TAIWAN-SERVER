#ifndef OLDSRV_OLD_DATA_DB_TAIWAN_CERA_IPG_RETURN_PAYLOADS_H_
#define OLDSRV_OLD_DATA_DB_TAIWAN_CERA_IPG_RETURN_PAYLOADS_H_

#include <stddef.h>
#include <stdint.h>

#include "common/manager/ipg_runtime.h"
#include "infra/network/packet/PacketHeader.h"

// Inter slot 71 receives the complete monitor-server packet.  The official
// 0627 consumer reads the account id immediately after the ten-byte internal
// packet header (Inter_MonitorCeraUpdate::dispatch_sig @0x84CEA92).
#pragma pack(push, 1)
struct SIG_MONITOR_CERA_UPDATE {
    PacketHeader header;       // +0x00
    uint32_t account_id;       // +0x0a
};

// CIPGHelper::IPGResult @0x810085E writes a one-byte presence flag.  A
// successful result then contains the result and original request; a network
// failure contains only the original request.  The one-byte prefix is an
// observed stream contract, so these are real unaligned nested SIPGData
// records rather than padding or an invented byte buffer.
struct SIG_IPG_RESULT_SUCCESS {
    WongWork::IPG::SIPGData result;   // enclosing SIG_IPG_RESULT +0x001
    WongWork::IPG::SIPGData request;  // enclosing SIG_IPG_RESULT +0x2dd
};

union SIG_IPG_RESULT_BODY {
    SIG_IPG_RESULT_SUCCESS success;
    WongWork::IPG::SIPGData failed_request;
};

struct SIG_IPG_RESULT {
    uint8_t has_result;          // +0x000
    SIG_IPG_RESULT_BODY body;    // +0x001
};
#pragma pack(pop)

#if defined(__i386__)
static_assert(sizeof(SIG_MONITOR_CERA_UPDATE) == 0x0eu,
              "monitor Cera update packet must remain 14 bytes");
static_assert(offsetof(SIG_MONITOR_CERA_UPDATE, account_id) == 0x0au,
              "monitor Cera account id must remain at +0x0a");
static_assert(sizeof(SIG_IPG_RESULT_SUCCESS) == 0x5b8u,
              "IPG success body must contain two 0x2dc-byte records");
static_assert(offsetof(WongWork::IPG::SIPGData, data_type) == 0x01cu &&
                  offsetof(WongWork::IPG::SIPGData, user_uid) == 0x020u &&
                  offsetof(WongWork::IPG::SIPGData, callback_stream) ==
                      0x2d8u,
              "IPG callback header fields moved");
static_assert(
    offsetof(WongWork::IPG::SIPGData,
             wire.cera_point.account_name) == 0x03cu &&
        offsetof(WongWork::IPG::SIPGData,
                 wire.cera_point.character_name) == 0x064u &&
        offsetof(WongWork::IPG::SIPGData,
                 wire.cera_point.charge_kind) == 0x0b1u &&
        offsetof(WongWork::IPG::SIPGData,
                 wire.cera_point.result_code_decimal) == 0x0b3u,
    "IPG point-result trace fields moved");
static_assert(
    offsetof(WongWork::IPG::SIPGData,
             wire.input.account_header.charge.account_name) == 0x028u &&
        offsetof(WongWork::IPG::SIPGData,
                 wire.input.account_header.charge.character_name) ==
            0x050u &&
        offsetof(WongWork::IPG::SIPGData,
                 wire.input.charge_kind) == 0x26du &&
        offsetof(WongWork::IPG::SIPGData,
                 wire.input.charge_result_decimal) == 0x26eu,
    "IPG Cera-input trace fields moved");
static_assert(offsetof(SIG_IPG_RESULT_SUCCESS, result) == 0x000u &&
                  offsetof(SIG_IPG_RESULT_SUCCESS, request) == 0x2dcu,
              "IPG success result/request order changed");
static_assert(offsetof(SIG_IPG_RESULT, has_result) == 0x000u &&
                  offsetof(SIG_IPG_RESULT, body) == 0x001u,
              "IPG result presence byte must remain a one-byte prefix");
static_assert(offsetof(SIG_IPG_RESULT, body.success.result) == 0x001u &&
                  offsetof(SIG_IPG_RESULT, body.success.request) == 0x2ddu &&
                  offsetof(SIG_IPG_RESULT, body.failed_request) == 0x001u,
              "IPG nested callback records moved");
static_assert(
    offsetof(SIG_IPG_RESULT, body.success.result.user_uid) == 0x021u &&
        offsetof(SIG_IPG_RESULT,
                 body.success.result.callback_stream) == 0x2d9u &&
        offsetof(SIG_IPG_RESULT,
                 body.failed_request.data_type) == 0x01du &&
        offsetof(SIG_IPG_RESULT,
                 body.failed_request.user_uid) == 0x021u,
    "IPG callback flow fields moved in the one-byte envelope");
static_assert(sizeof(SIG_IPG_RESULT) == 0x5b9u,
              "maximum IPG callback payload must remain 0x5b9 bytes");
#endif

#endif  // OLDSRV_OLD_DATA_DB_TAIWAN_CERA_IPG_RETURN_PAYLOADS_H_
