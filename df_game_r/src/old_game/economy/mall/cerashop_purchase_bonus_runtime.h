#ifndef OLDSRV_OLD_GAME_NPC_CERASHOP_PURCHASE_BONUS_RUNTIME_H_
#define OLDSRV_OLD_GAME_NPC_CERASHOP_PURCHASE_BONUS_RUNTIME_H_

#include <stdint.h>

#include <map>
#include <vector>

class CUser;

namespace ARAD {

// IDA unit: 99_misc_global/ARAD/PurcahseBonus.
// The original copy path at 0x81AC04C copies exactly the first 0x0a bytes.
struct PurcahseBonus {
    // IDA_EA: 0x81AC710
    // IDA_NAME: _ZN4ARAD13PurcahseBonusC2Ev
    // DEMANGLED: ARAD::PurcahseBonus::PurcahseBonus(void)
    PurcahseBonus();

    // IDA_EA: 0x88B72B4
    // IDA_NAME: _ZN4ARAD13PurcahseBonusD2Ev
    // DEMANGLED: ARAD::PurcahseBonus::~PurcahseBonus()
    ~PurcahseBonus();

    uint8_t required_purchase_count_;  // +0x00
    uint8_t alignment_01_[3];           // +0x01
    int bonus_item_index_;              // +0x04
    uint16_t bonus_item_count_;         // +0x08

    // IDA_EA: 0x88B66CE
    // IDA_NAME: _ZNK4ARAD13PurcahseBonuseqERKS0_
    // DEMANGLED: ARAD::PurcahseBonus::operator==(ARAD::PurcahseBonus const&)const
    bool operator==(const PurcahseBonus& rhs) const;
};

// IDA unit: 07_npc_shop_dialog/ARAD/CeraShopPurcahseCountBonus.
class CeraShopPurcahseCountBonus {
public:
    // IDA_EA: 0x854C5B6
    // IDA_NAME: _ZN4ARAD26CeraShopPurcahseCountBonusC2Ev
    // DEMANGLED: ARAD::CeraShopPurcahseCountBonus::CeraShopPurcahseCountBonus(void)
    CeraShopPurcahseCountBonus();

    // IDA_EA: 0x854C5CA
    // IDA_NAME: _ZN4ARAD26CeraShopPurcahseCountBonusD2Ev
    // DEMANGLED: ARAD::CeraShopPurcahseCountBonus::~CeraShopPurcahseCountBonus()
    ~CeraShopPurcahseCountBonus();

    int LoadScript();
    int GiveBonusItem(CUser* user,
                      unsigned long purchase_index,
                      int goods_no,
                      unsigned short purchase_count);
    int CheckBonusItem(int goods_no);

private:
    int _findBonusItem(const std::vector<PurcahseBonus>& bonuses,
                       PurcahseBonus& bonus,
                       unsigned short purchase_count);
    void _processBonusItem(CUser* user,
                           int goods_no,
                           const PurcahseBonus& bonus);

    std::map<int, std::vector<PurcahseBonus> > bonus_items_by_goods_;
};

}  // namespace ARAD

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ARAD::PurcahseBonus) == 0x0cu,
              "PurcahseBonus IDA ABI size must stay 0x0c");
static_assert(sizeof(ARAD::CeraShopPurcahseCountBonus) == 0x18u,
              "CeraShopPurcahseCountBonus IDA ABI size must stay 0x18");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_CERASHOP_PURCHASE_BONUS_RUNTIME_H_
