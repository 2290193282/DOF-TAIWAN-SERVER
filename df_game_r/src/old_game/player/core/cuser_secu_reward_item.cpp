#include "old_game/player/core/cuser.h"

#include <cstring>

namespace {

// CP949 bytes from the old server's src__19/src__20 data objects.
const char kSecurityRewardMailTitle[] =
    "\xb0\xed\xba\xed\xb8\xb0\xc0\xc7 \xbc\xb1\xb9\xb0";
const char kSecurityRewardMailText[] =
    "\xb0\xed\xba\xed\xb8\xb0\xc0\xc7 \xbc\xb1\xb9\xb0\xc0\xcc \xbf\xec\xc6\xed\xc0\xb8\xb7\xce "
    "\xb5\xb5\xc2\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.";

}  // namespace

// IDA_EA: 0x868833C
// IDA_NAME: _ZN5CUser18sendSecuRewardItemEP10Inven_Item25ENUM_SECURITY_REWARD_TYPEPi
// DEMANGLED: CUser::sendSecuRewardItem(Inven_Item *,ENUM_SECURITY_REWARD_TYPE,int *)
int CUser::sendSecuRewardItem(Inven_Item* item,
                              ENUM_SECURITY_REWARD_TYPE reward_type,
                              int* result_slots) {
    // The original body copies all 0x3d bytes, including the GUID, before its
    // null check. Do not replace this physical transfer with setCopy().
    const Inven_Item copied_item = *item;
    if (!item) {
        return 0;
    }

    int inserted_slot = -1;
    bool send_by_mail = static_cast<unsigned char>(CheckInTrade()) != 0;
    if (!send_by_mail) {
        inserted_slot = getCurCharacInvenW()->insertItemIntoInventory(
            copied_item, static_cast<eItemAddReason>(13), true, true);
        send_by_mail = inserted_slot == -1;
    }

    const int result_index = static_cast<int>(reward_type);
    if (send_by_mail) {
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            kSecurityRewardMailTitle,
            copied_item,
            0u,
            static_cast<unsigned int>(getCurCharacNo()),
            kSecurityRewardMailText,
            static_cast<int>(std::strlen(kSecurityRewardMailText)),
            0u,
            static_cast<ENUM_SERVER_GROUP>(GetServerGroup()),
            false,
            false);
        result_slots[result_index] = -1;
        return 0;
    }

    result_slots[result_index] = inserted_slot;
    return 1;
}
