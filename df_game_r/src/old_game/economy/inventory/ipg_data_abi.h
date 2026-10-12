#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_IPG_DATA_ABI_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_IPG_DATA_ABI_H_

#include <stddef.h>
#include <stdint.h>

namespace WongWork {
namespace IPG {

// IDA_EA: 0x8100B3E
// IDA_NAME: _ZN8WongWork3IPG10CIPGHelper10InitializeEjj
// The numeric values are the original IPG_DATA_TYPE dispatch values.  The
// semantic labels are intentionally separate from the still-incomplete union.
enum class SIPGDataType : uint32_t {
    kInput = 0u,
    kInputCancel = 1u,
    kPurchase = 2u,
    kCancelTransaction = 3u,
    kBalancePointQuery = 4u,
    kConfirmTransaction = 5u,
    kCeraPoint = 6u,
};

struct SIPGBranchInfo {
    SIPGDataType type;
    uint32_t wire_size;
    const char* semantic_name;
};

// These sizes are written by CIPGHelper::Initialize, not inferred from an
// overlapping payload declaration.
inline SIPGBranchInfo getSIPGBranchInfo(SIPGDataType type) {
    switch (type) {
    case SIPGDataType::kInput:
        return {type, 0x2b0u, "WebEvent coupon input / Taiwan charge request"};
    case SIPGDataType::kInputCancel:
        return {type, 0x1cbu, "legacy IPG input cancellation request"};
    case SIPGDataType::kPurchase:
        return {type, 0x1fcu, "Taiwan purchase request/result"};
    case SIPGDataType::kCancelTransaction:
        return {type, 0x1d4u, "Taiwan cancel transaction request"};
    case SIPGDataType::kBalancePointQuery:
        return {type, 0x099u, "Taiwan balance and point query/result"};
    case SIPGDataType::kConfirmTransaction:
        return {type, 0x1d4u, "Taiwan confirm transaction request"};
    case SIPGDataType::kCeraPoint:
        return {type, 0x090u, "Taiwan Cera Point request/result"};
    }

    return {type, 0u, "invalid IPG data type"};
}

namespace SIPGDataOffset {

// IDA_EA: 0x8100A2E, 0x810085E, 0x84CEAEE
constexpr size_t kWireSize = 0x000u;
constexpr size_t kEndpointIp = 0x004u;
constexpr size_t kEndpointPort = 0x018u;
constexpr size_t kDataType = 0x01cu;
constexpr size_t kUserId = 0x020u;
constexpr size_t kQueueResult = 0x024u;
constexpr size_t kPayload = 0x028u;
constexpr size_t kCallbackStream = 0x2d8u;
constexpr size_t kSize = 0x2dcu;

namespace WebEventCouponInput {
// IDA_EA: 0x80FFB0A, 0x86C121E, 0x86C1BD4
constexpr size_t kEventIPGCode = 0x0ddu;
constexpr size_t kAmountDecimal = 0x0e8u;
constexpr size_t kUniqueTransactionId = 0x0f4u;
constexpr size_t kEventName = 0x108u;
constexpr size_t kCharacterName = 0x16cu;
constexpr size_t kUserName = 0x1d0u;
constexpr size_t kSsn = 0x202u;
constexpr size_t kEmail = 0x210u;
}  // namespace WebEventCouponInput

namespace TaiwanCharge {
// IDA_EA: 0x80FFCA4, 0x81048C6
constexpr size_t kAccountName = 0x028u;
constexpr size_t kCharacterName = 0x050u;
constexpr size_t kPrice = 0x098u;
constexpr size_t kChargeKind = 0x26du;
}  // namespace TaiwanCharge

namespace TaiwanPurchase {
// IDA_EA: 0x810022E, 0x81048C6
constexpr size_t kAccountName = 0x03cu;
constexpr size_t kProductIdDecimal = 0x069u;
constexpr size_t kPriceDecimal = 0x071u;
constexpr size_t kCharacterName = 0x1e8u;
constexpr size_t kBillTransaction = 0x1f0u;
constexpr size_t kResultCodeDecimal = 0x173u;
constexpr size_t kResultMessage = 0x177u;
}  // namespace TaiwanPurchase

namespace TaiwanTransaction {
// IDA_EA: 0x81004DE, 0x8100CD2, 0x81048C6
constexpr size_t kBillTransaction = 0x1f0u;
}  // namespace TaiwanTransaction

namespace TaiwanBalancePoint {
// IDA_EA: 0x8100790, 0x81048C6
constexpr size_t kAccountName = 0x03cu;
constexpr size_t kBalanceDecimal = 0x069u;
constexpr size_t kResultCodeDecimal = 0x0b1u;
constexpr size_t kPointDecimal = 0x0b5u;
}  // namespace TaiwanBalancePoint

namespace TaiwanCeraPoint {
// IDA_EA: 0x80FFFC0, 0x81000F6, 0x81048C6
constexpr size_t kAccountName = 0x03cu;
constexpr size_t kCharacterName = 0x064u;
constexpr size_t kPointValue = 0x08cu;
constexpr size_t kPointContext = 0x094u;
constexpr size_t kPrice = 0x0acu;
constexpr size_t kCommand = 0x0b0u;
constexpr size_t kChargeKind = 0x0b1u;
constexpr size_t kPointKind = 0x0b2u;
}  // namespace TaiwanCeraPoint

}  // namespace SIPGDataOffset

namespace SIPGDataPoolNodeOffset {

// IDA_EA: 0x81019C2, 0x81021D6, 0x810547E, 0x81056D4
constexpr size_t kPayload = 0x000u;
constexpr size_t kMagic = 0x2dcu;
constexpr size_t kReferenceCount = 0x2e0u;
constexpr uint32_t kMagicValue = 0x8f21u;
constexpr size_t kSize = 0x2e4u;

}  // namespace SIPGDataPoolNodeOffset

static_assert(SIPGDataOffset::kCallbackStream + sizeof(uint32_t) ==
                  SIPGDataOffset::kSize,
              "SIPGData's i386 callback stream closes the 0x2dc-byte ABI");
static_assert(SIPGDataPoolNodeOffset::kReferenceCount + sizeof(uint8_t) ==
                  0x2e1u,
              "SIPGData pool node reference count is one byte at +0x2e0");
static_assert(SIPGDataPoolNodeOffset::kSize == 0x2e4u,
              "SIPGData pool node must retain its 0x2e4-byte stride");

}  // namespace IPG
}  // namespace WongWork

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_IPG_DATA_ABI_H_
