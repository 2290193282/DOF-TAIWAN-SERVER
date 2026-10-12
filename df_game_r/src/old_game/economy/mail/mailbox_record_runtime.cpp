#include "old_game/economy/mail/mail_trade_runtime.h"

#include "old_game/appearance/pet/creature.h"
#include "old_game/player/core/cuser.h"

#include <algorithm>
#include <cstring>

namespace WongWork {

typedef char CMailBox_CMail_Record_Size_Must_Be_0x80[
    (sizeof(CMailBox::CMail) == 0x80) ? 1 : -1];
typedef char CMailBox_CMail_MailIdx_Offset_Must_Be_0x00[
    (offsetof(CMailBox::CMail, mail_idx) == 0x00) ? 1 : -1];
typedef char CMailBox_CMail_SenderCharacNo_Offset_Must_Be_0x04[
    (offsetof(CMailBox::CMail, sender_charac_no) == 0x04) ? 1 : -1];
typedef char CMailBox_CMail_SenderName_Offset_Must_Be_0x08[
    (offsetof(CMailBox::CMail, sender_name) == 0x08) ? 1 : -1];
typedef char CMailBox_CMail_PackageMoney_Offset_Must_Be_0x28[
    (offsetof(CMailBox::CMail, package_money) == 0x28) ? 1 : -1];
typedef char CMailBox_CMail_PackageItem_Offset_Must_Be_0x2c[
    (offsetof(CMailBox::CMail, package_item) == 0x2c) ? 1 : -1];
typedef char CMailBox_CMail_Period_Offset_Must_Be_0x6c[
    (offsetof(CMailBox::CMail, period) == 0x6c) ? 1 : -1];
typedef char CMailBox_CMail_PackageType_Offset_Must_Be_0x70[
    (offsetof(CMailBox::CMail, package_type) == 0x70) ? 1 : -1];
typedef char CMailBox_CMail_Unlimit_Offset_Must_Be_0x74[
    (offsetof(CMailBox::CMail, unlimit) == 0x74) ? 1 : -1];
typedef char CMailBox_CMail_LetterId_Offset_Must_Be_0x78[
    (offsetof(CMailBox::CMail, letter_id) == 0x78) ? 1 : -1];

// IDA_EA: 0x85519FE
// IDA_NAME: _ZN8WongWork8CMailBox5CMail12ResetPackageEv
// DEMANGLED: WongWork::CMailBox::CMail::ResetPackage(void)
CMailBox::CMail* CMailBox::CMail::ResetPackage() {
    sender_name[0] = 0;
    package_item.reset();
    package_money = 0;
    unlimit = 0;
    package_type = 0;
    return this;
}

// IDA_EA: 0x85574D4
// IDA_NAME: _ZN8WongWork8CMailBox5CMail11GetLetterIdEv
// DEMANGLED: WongWork::CMailBox::CMail::GetLetterId(void)
int CMailBox::CMail::GetLetterId() {
    return letter_id;
}

// IDA_EA: 0x85574E0
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail10IsTimeOverEl
// DEMANGLED: WongWork::CMailBox::CMail::IsTimeOver(long)const
bool CMailBox::CMail::IsTimeOver(long cur_sec) const {
    return period <= cur_sec;
}

// IDA_EA: 0x85574F2
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail9GetPeriodEv
// DEMANGLED: WongWork::CMailBox::CMail::GetPeriod(void)const
long CMailBox::CMail::GetPeriod() const {
    return period;
}

// IDA_EA: 0x85574FE
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail13GetSenderNameEv
// DEMANGLED: WongWork::CMailBox::CMail::GetSenderName(void)const
const char* CMailBox::CMail::GetSenderName() const {
    return sender_name;
}

// IDA_EA: 0x855750A
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail17GetSenderCharacNoEv
// DEMANGLED: WongWork::CMailBox::CMail::GetSenderCharacNo(void)const
int CMailBox::CMail::GetSenderCharacNo() const {
    return sender_charac_no;
}

// IDA_EA: 0x8557516
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail15GetPackageMoneyEv
// DEMANGLED: WongWork::CMailBox::CMail::GetPackageMoney(void)const
int CMailBox::CMail::GetPackageMoney() const {
    return package_money;
}

// IDA_EA: 0x8557522
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail14GetPackageItemEv
// DEMANGLED: WongWork::CMailBox::CMail::GetPackageItem(void)const
Inven_Item CMailBox::CMail::GetPackageItem() const {
    return package_item;
}

// IDA_EA: 0x8557590
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail9isUnlimitEv
// DEMANGLED: WongWork::CMailBox::CMail::isUnlimit(void)const
int CMailBox::CMail::isUnlimit() const {
    return unlimit;
}

// IDA_EA: 0x855759C
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail15isAvatarPackageEv
// DEMANGLED: WongWork::CMailBox::CMail::isAvatarPackage(void)const
bool CMailBox::CMail::isAvatarPackage() const {
    return package_type == 1;
}

// IDA_EA: 0x85575AE
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail17isCreaturePackageEv
// DEMANGLED: WongWork::CMailBox::CMail::isCreaturePackage(void)const
bool CMailBox::CMail::isCreaturePackage() const {
    return package_type == 2;
}

// IDA_EA: 0x869782E
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail10getMailIdxEv
// DEMANGLED: WongWork::CMailBox::CMail::getMailIdx(void)const
int CMailBox::CMail::getMailIdx() const {
    return mail_idx;
}

// IDA_EA: 0x8551A36
// IDA_NAME: _ZNK8WongWork8CMailBox5CMail10MakePacketEP11PacketGuardlPK5CUser
// DEMANGLED: WongWork::CMailBox::CMail::MakePacket(PacketGuard *,long,CUser const*)const
int CMailBox::CMail::MakePacket(PacketGuard* packet,
                                long current_sec,
                                const CUser* user) const {
    packet->put_int(static_cast<int>(mail_idx));
    packet->put_int(static_cast<int>(std::strlen(sender_name)));
    packet->put_str(sender_name, static_cast<int>(std::strlen(sender_name)));
    packet->put_int(package_money);
    packet->put_int(package_item.item_id_);
    packet->put_byte(package_item.seal_or_state_ != 0);

    const int current_seconds = static_cast<int>(current_sec);
    if (isAvatarPackage()) {
        const CInventory* const inventory = user->getCurCharacInvenR();
        const CAvatarItemMgr* const avatar_manager =
            inventory->GetAvatarItemMgrR();
        packet->put_int(avatar_manager->GetRemainDate(package_item.add_info_,
                                                       current_seconds));
    } else {
        packet->put_int(package_item.add_info_);
    }

    packet->put_short(package_item.avatar_select_);
    packet->put_byte(package_item.GetItemAttr());
    packet->put_int(package_item.extend_info_);
    packet->put_byte(package_item.amplify_option_.getAbilityType());
    packet->put_short(package_item.amplify_option_.getAbilityValue());

    if (package_item.item_category_ == 5u) {
        const CInventory* const inventory = user->getCurCharacInvenR();
        const user_creature::CCreatureMgr* const creature_manager =
            inventory->GetCreatureMgrR();
        packet->put_int(creature_manager->getRemainTempPeriod(
            package_item.add_info_, current_seconds));
    }

    // CLIENT_IDA_EVIDENCE: NOTI97 sub_5AEDA0 reads exactly one item-detail u32
    // at 0x5AEF6F before ParseItemRandomOptionBlock at 0x5AEF7E.
    // PacketBuf::put_packet owns that Chronicle value; do not duplicate it here.
    packet->put_packet(package_item);

    if (isAvatarPackage()) {
        const CInventory* const inventory = user->getCurCharacInvenR();
        const CAvatarItemMgr* const avatar_manager =
            inventory->GetAvatarItemMgrR();
        const int avatar_key = package_item.add_info_;

        packet->put_int(30);
        packet->put_binary(
            reinterpret_cast<const char*>(
                avatar_manager->getJewelSocketData(avatar_key)),
            30);

        const stAvatarExpansionInfo_t* const expansion =
            avatar_manager->GetExpansionInfo(avatar_key);
        packet->put_int(4);
        if (expansion) {
            packet->put_binary(reinterpret_cast<const char*>(expansion), 4);
        } else {
            stAvatarExpansionInfo_t empty_expansion;
            empty_expansion.init();
            packet->put_binary(reinterpret_cast<const char*>(&empty_expansion),
                               4);
        }
    }

    if (isUnlimit()) {
        packet->put_int(0);
    } else {
        packet->put_int(std::max(1,
                                 static_cast<int>(period - current_sec)));
    }
    return packet->put_int(static_cast<int>(letter_id));
}

}  // namespace WongWork
