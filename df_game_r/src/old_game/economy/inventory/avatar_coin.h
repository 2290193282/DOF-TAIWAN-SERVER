#ifndef OLDSRV_06_USERINFO_STAT_INVENTORY_AVATAR_COIN_H_
#define OLDSRV_06_USERINFO_STAT_INVENTORY_AVATAR_COIN_H_

#include <stdint.h>

class CUser;

// IDA unit: 06_userinfo_stat_inventory/AvatarCoin.
// Restored scalar-only slice:
// IDA_EA 0x817FED4, 0x817FEE2, 0x817FEF0, 0x817FEFA, 0x817FF14, 0x817FF44.
// The persistence and packet paths below use the existing SIG_SAVE_INVENTORY
// serializer, DB queue, and CUser history/packet providers.
struct AvatarCoin {
    AvatarCoin();

    AvatarCoin* Set(uint32_t coin_count_value);
    int Get() const;
    int Add(uint32_t coin_count_value);
    int Use();
    bool IsUsable() const;

    // IDA_EA: 0x817FFE4
    // IDA_NAME: _ZN10AvatarCoin14SendSyncPacketEP5CUser
    // DEMANGLED: AvatarCoin::SendSyncPacket(CUser *)
    // The old call sites invoke this static packet producer with the user;
    // it reads that user's embedded AvatarCoin record before sending NOTI 587.
    static int SendSyncPacket(CUser* user);

    // IDA_EA: 0x81800D6
    // IDA_NAME: _ZN10AvatarCoin8SaveToDBEP5CUser
    // DEMANGLED: AvatarCoin::SaveToDB(CUser *)
    static int SaveToDB(CUser* user);

    // IDA_EA: 0x81801D0
    // IDA_NAME: _ZN10AvatarCoin10BuyProcessEP5CUserR10Inven_Item
    // DEMANGLED: AvatarCoin::BuyProcess(CUser *,Inven_Item &)
    static int BuyProcess(CUser* user, struct Inven_Item& item);

    // IDA_EA: 0x818028A
    // IDA_NAME: _ZN10AvatarCoin10GetPackageEP5CUserib
    // DEMANGLED: AvatarCoin::GetPackage(CUser *,int,bool)
    static int GetPackage(CUser* user, int coin_count, bool is_received);

    struct HistoryLog {
        // IDA_EA: 0x817FF54
        // IDA_NAME: _ZN10AvatarCoin10HistoryLog6SubLogEP5CUserj
        // DEMANGLED: AvatarCoin::HistoryLog::SubLog(CUser *,unsigned int)
        static void SubLog(CUser* user, unsigned int amount);

        // IDA_EA: 0x817FF9C
        // IDA_NAME: _ZN10AvatarCoin10HistoryLog6AddLogEP5CUserj
        // DEMANGLED: AvatarCoin::HistoryLog::AddLog(CUser *,unsigned int)
        static void AddLog(CUser* user, unsigned int amount);
    };

    uint32_t coin_count;
};


#endif  // OLDSRV_06_USERINFO_STAT_INVENTORY_AVATAR_COIN_H_
