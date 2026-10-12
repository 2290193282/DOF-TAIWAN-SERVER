#ifndef OLDSRV_OLD_GAME_DUNGEON_PVP_LUCK_POINT_RUNTIME_H_
#define OLDSRV_OLD_GAME_DUNGEON_PVP_LUCK_POINT_RUNTIME_H_

#include <map>
#include <vector>

class CUser;

// IDA unit: 99_misc_global/CLuckPoint.  The old member symbol has no object
// receiver: its first argument is the user whose luck-point balance changes.
class CLuckPoint {
public:
    // IDA_EA: 0x8550884 / 0x85508EC.  stGenerateResult_t embeds this
    // complete value after its item vector; it is not an optional side
    // channel owned by the generator.
    CLuckPoint();
    ~CLuckPoint();

    // IDA_EA: 0x8550932 / 0x8550954 / 0x85509DA.
    CUser* AddUser(CUser* user);
    int _GetMaxLuckPointUserIndex();
    unsigned int GetLuckPoint();

    // IDA_EA: 0x8550A84
    // IDA_NAME: _ZN10CLuckPoint12UseLuckPointEP5CUseriii
    // DEMANGLED: CLuckPoint::UseLuckPoint(CUser *,int,int,int)
    static int UseLuckPoint(CUser* user,
                            int use_luck_point,
                            int rarity,
                            int item_grade);

    // IDA_EA: 0x8550B14 / 0x8550BE4 / 0x8550E50 / 0x8550EF0.
    int UseLuckPoint(int use_luck_point, int rarity, int item_grade,
                     int item_index);
    int GetItemRarity(int* rarity_weight,
                      int random_number,
                      int drop_type);
    CUser* GetUserGainItem(int item_index);
    bool sendItemDropRate(const char* message);

private:
    std::vector<CUser*> users_;              // +0x00
    std::map<int, int> user_gain_item_index_; // +0x0c
};

#if __SIZEOF_POINTER__ == 4
static_assert(sizeof(CLuckPoint) == 0x24u, "CLuckPoint ABI");
#endif

#endif  // OLDSRV_OLD_GAME_DUNGEON_PVP_LUCK_POINT_RUNTIME_H_
