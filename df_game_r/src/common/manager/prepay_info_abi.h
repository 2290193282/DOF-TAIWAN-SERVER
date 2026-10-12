#ifndef OLDSRV_GLOBAL_PREPAY_INFO_ABI_H_
#define OLDSRV_GLOBAL_PREPAY_INFO_ABI_H_

#include <stddef.h>
#include <stdint.h>

#include <map>

#include "old_game/economy/inventory/taiwan_billing_runtime.h"


// IDA evidence:
//   CPrePayInfoMgr::InsertPrePayInfo @ 0x84ED0BC constructs
//   pair<int const, PrePayInfo> with a 0x2c-byte local, proving the value is
//   0x28 bytes.  Inter_AuctionResultBidding @ 0x84D6CDA reads the two billing
//   item IDs at +0x08/+0x0c and the two BillTransaction instances at
//   +0x10/+0x1c.  The first eight bytes are the auction operation identity
//   copied from Inter_OwnerisVIP @ 0x84E7C90.
struct PrePayInfo {
    uint32_t auction_transaction_low;   // +0x00
    uint32_t auction_transaction_high;  // +0x04
    int billing_item_id;                // +0x08
    int secondary_billing_item_id;      // +0x0c
    Taiwan::BillTransaction primary_transaction;    // +0x10
    Taiwan::BillTransaction secondary_transaction;  // +0x1c
};

static_assert(sizeof(PrePayInfo) == 0x28u,
              "PrePayInfo must retain the IDA 0x28-byte value ABI");
static_assert(offsetof(PrePayInfo, billing_item_id) == 0x08u,
              "PrePayInfo primary billing item offset must stay +0x08");
static_assert(offsetof(PrePayInfo, primary_transaction) == 0x10u,
              "PrePayInfo primary transaction offset must stay +0x10");
static_assert(offsetof(PrePayInfo, secondary_transaction) == 0x1cu,
              "PrePayInfo secondary transaction offset must stay +0x1c");

class CPrePayInfoMgr {
public:
    // IDA_EA: 0x8231EDE
    // IDA_NAME: _ZN14CPrePayInfoMgrC2Ev
    CPrePayInfoMgr();

    // IDA_EA: 0x82151B8
    // IDA_NAME: _ZN14CPrePayInfoMgr14FindPrePayInfoEi
    // DEMANGLED: CPrePayInfoMgr::FindPrePayInfo(int)
    PrePayInfo* FindPrePayInfo(int charac_no);

    // IDA_EA: 0x84ED0BC
    // IDA_NAME: _ZN14CPrePayInfoMgr16InsertPrePayInfoEi10PrePayInfo
    // The old ABI passes the 0x28-byte value by value.
    void InsertPrePayInfo(int charac_no, PrePayInfo info);

    // IDA_EA: 0x84ED10E
    // IDA_NAME: _ZN14CPrePayInfoMgr16DeletePrePayInfoEi
    void DeletePrePayInfo(int charac_no);

private:
    typedef std::map<int, PrePayInfo> PrePayInfoMap;
    PrePayInfoMap entries_;  // +0x00
};

#if defined(__i386__)
static_assert(sizeof(CPrePayInfoMgr) == 0x18u,
              "CPrePayInfoMgr must retain the IDA std::map-only layout");
#endif

// IDA_EA: 0x8231F20
// IDA_NAME: _Z16G_CPrePayInfoMgrv
// DEMANGLED: G_CPrePayInfoMgr(void)
CPrePayInfoMgr* G_CPrePayInfoMgr();

#endif  // OLDSRV_GLOBAL_PREPAY_INFO_ABI_H_
