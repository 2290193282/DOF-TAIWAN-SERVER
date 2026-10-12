#include "old_game/economy/mail/mail_trade_value_types.h"

namespace ARAD_DATAS {

// IDA_EA: 0x8197E12
// IDA_NAME: _ZNK10ARAD_DATAS16AradMailItemInfo11GetItemAttrEv
// DEMANGLED: ARAD_DATAS::AradMailItemInfo::GetItemAttr(void)const
int AradMailItemInfo::GetItemAttr() const {
    return item_attr;
}

// IDA_EA: 0x81A3FC2
// IDA_NAME: _ZN10ARAD_DATAS16AradMailItemInfoC2Ev
// DEMANGLED: ARAD_DATAS::AradMailItemInfo::AradMailItemInfo(void)
AradMailItemInfo::AradMailItemInfo()
    : seal_flag(0),
      endurance(0),
      item_index(0),
      extend_info(0),
      item_attr(0) {
}

// IDA_EA: 0x81A3FF2
// IDA_NAME: _ZN10ARAD_DATAS12AradMailInfoC2Ev
// DEMANGLED: ARAD_DATAS::AradMailInfo::AradMailInfo(void)
AradMailInfo::AradMailInfo()
    : server_group(0),
      gold(0),
      occ_time(0),
      receiver_charac_no(0),
      sender_charac_no(0),
      letter_text_length(0),
      content_address(0),
      title_address(0),
      item_info() {
}

}  // namespace ARAD_DATAS
