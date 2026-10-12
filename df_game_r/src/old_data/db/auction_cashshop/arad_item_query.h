#ifndef OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_ARAD_ITEM_QUERY_H_
#define OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_ARAD_ITEM_QUERY_H_

#include <stddef.h>
#include <stdint.h>

namespace ARAD_DATAS {
// IDA_EA: ARAD::DATABASE::ITEM_QUERY::arad_sp_buy_item_check @0x818CFAB.
// The original query reads every byte of this 0x14-byte transport value that
// crosses the buy-count DB boundary.  It is not a prefix or an opaque facade:
// result is returned at +0, the two wire policy values begin at +2/+4, and
// the three database keys occupy +8, +0xc and +0x10 respectively.
struct BuyItemInfo {
    uint8_t result;                    // +0x00
    uint8_t result_context;            // +0x01 (not consumed by this query)
    int16_t check_type;                // +0x02
    int32_t maximum_count_wire;        // +0x04; query compares its low i16
    int32_t charac_no;                 // +0x08
    int32_t member_id;                 // +0x0c
    int32_t ipg_no;                    // +0x10
};

#if defined(__i386__)
static_assert(sizeof(BuyItemInfo) == 0x14u,
              "ARAD_DATAS::BuyItemInfo must retain the IDA 0x14-byte ABI");
static_assert(offsetof(BuyItemInfo, check_type) == 0x02u &&
                  offsetof(BuyItemInfo, maximum_count_wire) == 0x04u &&
                  offsetof(BuyItemInfo, charac_no) == 0x08u &&
                  offsetof(BuyItemInfo, member_id) == 0x0cu &&
                  offsetof(BuyItemInfo, ipg_no) == 0x10u,
              "ARAD_DATAS::BuyItemInfo field offsets changed");
#endif
}

namespace ARAD {
namespace DATABASE {
namespace ITEM_QUERY {

int arad_sp_buy_item_check(ARAD_DATAS::BuyItemInfo& info);

}  // namespace ITEM_QUERY
}  // namespace DATABASE
}  // namespace ARAD


#endif  // OLDSRV_OLD_DATA_DB_AUCTION_CASHSHOP_ARAD_ITEM_QUERY_H_
