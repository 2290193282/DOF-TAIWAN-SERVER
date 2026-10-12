#ifndef OLDSRV_OLD_GAME_CREATURE_AVATAR_CREATURE_H_
#define OLDSRV_OLD_GAME_CREATURE_AVATAR_CREATURE_H_

#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/appearance/pet/creature_cmd372_abi.h"
#include "infra/network/packet/packet_guard.h"

#include <stddef.h>
#include <stdint.h>
#include <list>
#include <map>
#include <queue>

class CUser;
class Stream;
struct Inven_Item;
enum ENUM_INTERNALPACKET : int;
struct SIG_CREATURE_ITEM;
struct SIG_CREATURE_ITEM_DETAIL_INFO;
struct SIG_LOAD_INVENTORY;
struct SIG_POSTAL_CREATURE_TEMP_PERIOD;
struct AvatarDetailInfo;

template <typename T>
class DynamicPool {
public:
    // IDA layout, shared by the Avatar_Item, CCreature and CEgg pools:
    // callback +0x00, error state +0x04, free-node queue +0x08,
    // allocated-block list +0x30, total nodes +0x38, bookkeeping +0x3c,
    // initial/growth counts +0x40/+0x42, allocation map +0x44.
    class CNode : public T {
    public:
        CNode();
        ~CNode();

        uint32_t dummy;
        uint8_t reference_count;
    };

    typedef void (*PoolExhaustedCallback)();

    DynamicPool(int initial_count, int growth_count);
    explicit DynamicPool(int initial_count);
    ~DynamicPool();

    T* Acquire();
    int Free(T* object);
    int MoreAlloc(int count);
    int Remain();
    int Size();

private:
    PoolExhaustedCallback exhausted_callback_;
    int error_state_;
    std::queue<CNode*> free_nodes_;
    std::list<CNode*> allocated_blocks_;
    int total_node_count_;
    // IDA C2 clears this 32-bit bookkeeping member at +0x3c.  No runtime
    // body in the recovered family assigns it afterwards.
    uint32_t cleared_bookkeeping_;
    uint16_t initial_block_count_;
    uint16_t growth_block_count_;
    std::map<void*, int> allocated_node_map_;
};

// The old user_creature classes are global-namespace ABI owners.  Keep this
// global namespace so their one-to-one IDA symbols remain exact.
namespace user_creature {

class CCreatureMgr;

class CCreatureItemConverter {
public:
    // IDA_EA: 0x8336EAE
    // IDA_NAME: _ZN13user_creature22CCreatureItemConverter10SetEggItemEP10Inven_Item
    // DEMANGLED: user_creature::CCreatureItemConverter::SetEggItem(Inven_Item *)
    static bool SetEggItem(Inven_Item* item);
};

// IDA unit: user_creature::COverSkillTime.  The object is the single saved
// server-time word embedded at CCreature +0x50.
class COverSkillTime {
public:
    // IDA_EA: 0x833D636
    // IDA_NAME: _ZN13user_creature14COverSkillTimeC2Ev
    // DEMANGLED: user_creature::COverSkillTime::COverSkillTime(void)
    COverSkillTime();

    // IDA_EA: 0x833D644
    // IDA_NAME: _ZN13user_creature14COverSkillTime7SetTimeEv
    // DEMANGLED: user_creature::COverSkillTime::SetTime(void)
    int SetTime();

    // IDA_EA: 0x833D65E
    // IDA_NAME: _ZN13user_creature14COverSkillTime9checkTimeEv
    // DEMANGLED: user_creature::COverSkillTime::checkTime(void)
    bool checkTime();

    // IDA_EA: 0x833EF04
    // IDA_NAME: _ZN13user_creature14COverSkillTime7GetTimeEv
    // DEMANGLED: user_creature::COverSkillTime::GetTime(void)
    COverSkillTime* GetTime();

    // Typed read of the single IDA +0x00 time word.  This is intentionally
    // inline-only: the old exported GetTime() ABI continues to return `this`.
    int current_time() const { return time_; }

private:
    int time_;
};

static_assert(sizeof(COverSkillTime) == 0x04u,
              "COverSkillTime IDA ABI size must stay 0x04");

// IDA type: user_creature::CREATURE_ITME_TYPE.  The spelling is part of the
// original ABI and therefore intentionally retained.
enum CREATURE_ITME_TYPE {
    CREATURE_ITME_TYPE_EGG = 0,
    CREATURE_ITME_TYPE_CREATURE = 1,
};

// IDA unit: user_creature::CCreatureItem.  This is the polymorphic 0x1c-byte
// base used by both concrete pool item types.  The original vtable has four
// pure slots: Reset, IsSave, LoadCreatureItem, and SaveCreatureItem.
class CCreatureItem {
public:
    // IDA_EA: 0x8340C82
    // IDA_NAME: _ZN13user_creature13CCreatureItemC2Ev
    // DEMANGLED: user_creature::CCreatureItem::CCreatureItem(void)
    CCreatureItem();

    virtual void Reset() = 0;
    virtual int IsSave() = 0;
    virtual int LoadCreatureItem(CCreatureMgr* manager,
                                 Inven_Item* item,
                                 SIG_CREATURE_ITEM_DETAIL_INFO* detail,
                                 CUser* user) = 0;
    virtual int SaveCreatureItem(Inven_Item* item,
                                 SIG_CREATURE_ITEM_DETAIL_INFO* detail,
                                 CUser* user) = 0;

    // IDA_EA: 0x833EFB0
    // IDA_NAME: _ZN13user_creature13CCreatureItem9SetItemIdEi
    // DEMANGLED: user_creature::CCreatureItem::SetItemId(int)
    CCreatureItem* SetItemId(int item_id);

    // IDA_EA: 0x833EF20
    // IDA_NAME: _ZN13user_creature13CCreatureItem3SetEPS0_
    // DEMANGLED: user_creature::CCreatureItem::Set(user_creature::CCreatureItem *)
    CCreatureItem* Set(CCreatureItem* source);

    // IDA_EA: 0x833EFE2
    // IDA_NAME: _ZN13user_creature13CCreatureItem19SetCreatureItemTypeENS_18CREATURE_ITME_TYPEE
    // DEMANGLED: user_creature::CCreatureItem::SetCreatureItemType(user_creature::CREATURE_ITME_TYPE)
    CCreatureItem* SetCreatureItemType(CREATURE_ITME_TYPE type);

    // IDA_EA: 0x8337050
    // IDA_NAME: _ZN13user_creature13CCreatureItem9SetSlotNoEi
    // DEMANGLED: user_creature::CCreatureItem::SetSlotNo(int)
    CCreatureItem* SetSlotNo(int slot);

    // IDA_EA: 0x833EFA4
    // IDA_NAME: _ZN13user_creature13CCreatureItem9GetItemIdEv
    // DEMANGLED: user_creature::CCreatureItem::GetItemId(void)
    int GetItemId();

    // IDA_EA: 0x833EFBE
    // IDA_NAME: _ZN13user_creature13CCreatureItem6GetUidEv
    // DEMANGLED: user_creature::CCreatureItem::GetUid(void)
    int GetUid();

    // IDA_EA: 0x833EFCA
    // IDA_NAME: _ZN13user_creature13CCreatureItem9GetSlotNoEv
    // DEMANGLED: user_creature::CCreatureItem::GetSlotNo(void)
    int GetSlotNo();

    // IDA_EA: 0x833F00A
    // IDA_NAME: _ZNK13user_creature13CCreatureItem13getExpireTimeEv
    // DEMANGLED: user_creature::CCreatureItem::getExpireTime(void)const
    int getExpireTime() const;

    // IDA_EA: 0x833F016
    // IDA_NAME: _ZNK13user_creature13CCreatureItem13GetRemainDateEi
    // DEMANGLED: user_creature::CCreatureItem::GetRemainDate(int)const
    int GetRemainDate(int tick) const;

    // IDA_EA: 0x833EF98
    // IDA_NAME: _ZN13user_creature13CCreatureItem19GetCreatureItemTypeEv
    // DEMANGLED: user_creature::CCreatureItem::GetCreatureItemType(void)
    int GetCreatureItemType();

    // IDA_EA: 0x833EFD6
    // IDA_NAME: _ZN13user_creature13CCreatureItem11GetNoChargeEv
    // DEMANGLED: user_creature::CCreatureItem::GetNoCharge(void)
    int GetNoCharge();

    // IDA_EA: 0x833EFF0
    // IDA_NAME: _ZN13user_creature13CCreatureItem7GetStatEv
    // DEMANGLED: user_creature::CCreatureItem::GetStat(void)
    int GetStat();

    // IDA_EA: 0x833EFFC
    // IDA_NAME: _ZN13user_creature13CCreatureItem13SetExpireTimeEi
    // DEMANGLED: user_creature::CCreatureItem::SetExpireTime(int)
    CCreatureItem* SetExpireTime(int expire_time);

protected:
    friend struct CCreatureItemLayoutVerifier;

    int item_id_;             // +0x04
    int creature_item_type_;  // +0x08
    int uid_;                 // +0x0c
    int slot_no_;             // +0x10
    uint8_t no_charge_;       // +0x14
    uint8_t stat_;            // +0x15
    uint16_t padding_;        // +0x16
    int expire_time_;         // +0x18
};

#if defined(__i386__)
struct CCreatureItemLayoutVerifier {
    static_assert(offsetof(CCreatureItem, item_id_) == 0x04u,
                  "CCreatureItem item id must stay at IDA +0x04");
    static_assert(offsetof(CCreatureItem, creature_item_type_) == 0x08u,
                  "CCreatureItem type must stay at IDA +0x08");
    static_assert(offsetof(CCreatureItem, uid_) == 0x0cu,
                  "CCreatureItem uid must stay at IDA +0x0c");
    static_assert(offsetof(CCreatureItem, slot_no_) == 0x10u,
                  "CCreatureItem slot must stay at IDA +0x10");
    static_assert(offsetof(CCreatureItem, no_charge_) == 0x14u,
                  "CCreatureItem no-charge flag must stay at IDA +0x14");
    static_assert(offsetof(CCreatureItem, stat_) == 0x15u,
                  "CCreatureItem stat must stay at IDA +0x15");
    static_assert(offsetof(CCreatureItem, expire_time_) == 0x18u,
                  "CCreatureItem expiration must stay at IDA +0x18");
};
static_assert(sizeof(CCreatureItem) == 0x1cu,
              "CCreatureItem IDA ABI size must stay 0x1c");
#endif

// IDA layout: CCreatureMgr begins with this map at +0x00 and keeps the
// temporary period map at +0x30.  Name both containers once so every manager
// method works through the canonical typed members.
typedef std::map<int, CCreatureItem*> CCreatureItemMap;
typedef std::map<int, int> CCreatureTempPeriodMap;

class CArtifact {
public:
    // IDA_EA: 0x833EF0C
    // IDA_NAME: _ZN13user_creature9CArtifactC2Ev
    // DEMANGLED: user_creature::CArtifact::CArtifact(void)
    CArtifact();
    // IDA_EA: 0x8336F60
    // IDA_NAME: _ZN13user_creature9CArtifact5ResetEv
    // DEMANGLED: user_creature::CArtifact::Reset(void)
    CArtifact* Reset();

    // IDA_EA: 0x8336F6E
    // IDA_NAME: _ZN13user_creature9CArtifact13EquipArtifactEPK10Inven_Item
    // DEMANGLED: user_creature::CArtifact::EquipArtifact(Inven_Item const*)
    int EquipArtifact(const Inven_Item* item);

    // IDA_EA: 0x8336FC8
    // IDA_NAME: _ZN13user_creature9CArtifact19GetExperienceAmountEv
    // DEMANGLED: user_creature::CArtifact::GetExperienceAmount(void)
    long double GetExperienceAmount();

    // IDA_EA: 0x8337012
    // IDA_NAME: _ZN13user_creature9CArtifact23GetCreatureMinimumLevelEv
    // DEMANGLED: user_creature::CArtifact::GetCreatureMinimumLevel(void)
    int GetCreatureMinimumLevel();

private:
    uint32_t item_pointer_;
};

static_assert(sizeof(CArtifact) == 0x04u,
              "CArtifact IDA ABI size must stay 0x04");

class CCreature;

class CCreatureMgr {
public:
    // IDA_EA: 0x822D3A8
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr18IsEquippedCreatureEv
    // DEMANGLED: user_creature::CCreatureMgr::IsEquippedCreature(void)const
    bool IsEquippedCreature() const;

    // IDA_EA: 0x8338F36
    // IDA_NAME: _ZN13user_creature12CCreatureMgr16GetCreatureIndexEv
    // DEMANGLED: user_creature::CCreatureMgr::GetCreatureIndex(void)
    int GetCreatureIndex();

    // IDA_EA: 0x83392F6
    // IDA_NAME: _ZN13user_creature12CCreatureMgr5EquipEPNS_9CCreatureE
    // DEMANGLED: user_creature::CCreatureMgr::Equip(user_creature::CCreature *)
    int Equip(class CCreature* creature);

    // IDA_EA: 0x833934C
    // IDA_NAME: _ZN13user_creature12CCreatureMgr9DismantleEPNS_9CCreatureE
    // DEMANGLED: user_creature::CCreatureMgr::Dismantle(user_creature::CCreature *)
    int Dismantle(class CCreature* creature);

    // IDA_EA: 0x83393A2
    // IDA_NAME: _ZN13user_creature12CCreatureMgr17EquipCreatureItemEPNS_9CCreatureES2_
    // DEMANGLED: user_creature::CCreatureMgr::EquipCreatureItem(user_creature::CCreature *,user_creature::CCreature *)
    bool EquipCreatureItem(class CCreature* old_creature,
                           class CCreature* new_creature);

    // IDA_EA: 0x83393FC
    // IDA_NAME: _ZN13user_creature12CCreatureMgr19IsArtifactEquipSlotEi
    // DEMANGLED: user_creature::CCreatureMgr::IsArtifactEquipSlot(int)
    bool IsArtifactEquipSlot(int slot);

    // IDA_EA: 0x8339420
    // IDA_NAME: _ZN13user_creature12CCreatureMgr12SwapCreatureEP10Inven_ItemiiS2_ii
    // DEMANGLED: user_creature::CCreatureMgr::SwapCreature(Inven_Item *,int,int,Inven_Item *,int,int)
    int SwapCreature(Inven_Item* left,
                     int left_slot,
                     int left_space,
                     Inven_Item* right,
                     int right_slot,
                     int right_space);

    // IDA_EA: 0x83396C8
    // IDA_NAME: _ZN13user_creature12CCreatureMgr13EquipArtifactEiP10Inven_Item
    // DEMANGLED: user_creature::CCreatureMgr::EquipArtifact(int,Inven_Item *)
    bool EquipArtifact(int slot, Inven_Item* item);

    // IDA_EA: 0x8339718
    // IDA_NAME: _ZN13user_creature12CCreatureMgr17DismantleArtifactEiP10Inven_Item
    // DEMANGLED: user_creature::CCreatureMgr::DismantleArtifact(int,Inven_Item *)
    int DismantleArtifact(int slot, Inven_Item* item);

    // IDA_EA: 0x8339752
    // IDA_NAME: _ZN13user_creature12CCreatureMgr12SwapArtifactEP10Inven_ItemiiS2_ii
    // DEMANGLED: user_creature::CCreatureMgr::SwapArtifact(Inven_Item *,int,int,Inven_Item *,int,int)
    int SwapArtifact(Inven_Item* left,
                     int left_slot,
                     int left_space,
                     Inven_Item* right,
                     int right_slot,
                     int right_space);

    // IDA_EA: 0x833989C
    // IDA_NAME: _ZN13user_creature12CCreatureMgr16DenySwapArtifactEP10Inven_ItemiiS2_ii
    // DEMANGLED: user_creature::CCreatureMgr::DenySwapArtifact(Inven_Item *,int,int,Inven_Item *,int,int)
    int DenySwapArtifact(Inven_Item* left,
                         int left_slot,
                         int left_space,
                         Inven_Item* right,
                         int right_slot,
                         int right_space);

    // IDA_EA: 0x83399FA
    // IDA_NAME: _ZN13user_creature12CCreatureMgr16SwapCreatureItemEP10Inven_ItemiiS2_ii
    // DEMANGLED: user_creature::CCreatureMgr::SwapCreatureItem(Inven_Item *,int,int,Inven_Item *,int,int)
    int SwapCreatureItem(Inven_Item* left,
                         int left_slot,
                         int left_space,
                         Inven_Item* right,
                         int right_slot,
                         int right_space);

    // IDA_EA: 0x8339ACC
    // IDA_NAME: _ZN13user_creature12CCreatureMgr17CheckDiedCreatureEv
    // DEMANGLED: user_creature::CCreatureMgr::CheckDiedCreature(void)
    bool CheckDiedCreature();

    // IDA_EA: 0x8339EE0
    // IDA_NAME: _ZN13user_creature12CCreatureMgr10SetStomachEi
    // DEMANGLED: user_creature::CCreatureMgr::SetStomach(int)
    int SetStomach(int value);

    // IDA_EA: 0x8339196
    // IDA_NAME: _ZN13user_creature12CCreatureMgrC2Ev
    // DEMANGLED: user_creature::CCreatureMgr::CCreatureMgr(void)
    CCreatureMgr();
    // IDA_EA: 0x86992CC
    // IDA_NAME: _ZN13user_creature12CCreatureMgrC2ERKS0_
    // DEMANGLED: user_creature::CCreatureMgr::CCreatureMgr(user_creature::CCreatureMgr const&)
    CCreatureMgr(const CCreatureMgr& other);
    // IDA_EA: 0x8339226
    // IDA_NAME: _ZN13user_creature12CCreatureMgrD2Ev
    // DEMANGLED: user_creature::CCreatureMgr::~CCreatureMgr(void)
    ~CCreatureMgr();
    // IDA_EA: 0x869D614
    // IDA_NAME: _ZN13user_creature12CCreatureMgraSERKS0_
    // DEMANGLED: user_creature::CCreatureMgr::operator=(user_creature::CCreatureMgr const&)
    CCreatureMgr& operator=(const CCreatureMgr& other);
    // IDA_EA: 0x822D39A
    // IDA_NAME: _ZN13user_creature12CCreatureMgr7SetUserEP5CUser
    // DEMANGLED: user_creature::CCreatureMgr::SetUser(CUser *)
    CCreatureMgr* SetUser(CUser* user);

    // IDA_EA: 0x833BA30
    // IDA_NAME: _ZN13user_creature12CCreatureMgr18IsSaveCreatureItemEv
    // DEMANGLED: user_creature::CCreatureMgr::IsSaveCreatureItem(void)
    int IsSaveCreatureItem();

    // IDA_EA: 0x833BBBE
    // IDA_NAME: _ZN13user_creature12CCreatureMgr16SaveCreatureItemEv
    // DEMANGLED: user_creature::CCreatureMgr::SaveCreatureItem(void)
    int SaveCreatureItem();
    // IDA_EA: 0x833A45A
    // IDA_NAME: _ZN13user_creature12CCreatureMgr5ResetEv
    // DEMANGLED: user_creature::CCreatureMgr::Reset(void)
    CCreatureMgr* Reset();

    // IDA_EA: 0x833B80C
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr15GetCreatureItemEP18SIG_LOAD_INVENTORY
    // DEMANGLED: user_creature::CCreatureMgr::GetCreatureItem(SIG_LOAD_INVENTORY *)const
    int GetCreatureItem(SIG_LOAD_INVENTORY* inventory) const;

    // IDA_EA: 0x833926C
    // IDA_NAME: _ZN13user_creature12CCreatureMgr24IsCreatureEquipmentScopeEi
    // DEMANGLED: user_creature::CCreatureMgr::IsCreatureEquipmentScope(int)
    int IsCreatureEquipmentScope(int slot);

    // IDA_EA: 0x8339292
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr16FindCreatureItemEi
    // DEMANGLED: user_creature::CCreatureMgr::FindCreatureItem(int)const
    const CCreatureItem* FindCreatureItem(int key) const;

    // IDA_EA: 0x833D758
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr13getExpireDateEi
    // DEMANGLED: user_creature::CCreatureMgr::getExpireDate(int)const
    int getExpireDate(int key) const;

    // IDA_EA: 0x833D78E
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr13GetRemainDateEii
    // DEMANGLED: user_creature::CCreatureMgr::GetRemainDate(int,int)const
    int GetRemainDate(int key, int tick) const;

    // IDA_EA: 0x8531FBE
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr19GetCreatureItemSizeEv
    // DEMANGLED: user_creature::CCreatureMgr::GetCreatureItemSize(void)const
    int GetCreatureItemSize() const;

    // IDA_EA: 0x833AD34
    // IDA_NAME: _ZN13user_creature12CCreatureMgr11ChangeOwnerEP10Inven_Itemiiii
    // DEMANGLED: user_creature::CCreatureMgr::ChangeOwner(Inven_Item *,int,int,int,int)
    int ChangeOwner(Inven_Item* item,
                    int slot,
                    int reason,
                    int old_charac_no,
                    int old_creature_flag);

    // IDA_EA: 0x833A854
    // IDA_NAME: _ZN13user_creature12CCreatureMgr22UnregisterCreatureItemEi
    // DEMANGLED: user_creature::CCreatureMgr::UnregisterCreatureItem(int)
    int UnregisterCreatureItem(int key);

    // CMD226 reconstructed transaction helper.  Both identities are located
    // before either object is erased, so validation failure cannot consume a
    // single half of the pair.  No member storage is added to the ABI owner.
    int ConsumeCompoundCreatureItems(int first_uid, int second_uid);

    // IDA_EA: 0x833A77E
    // IDA_NAME: _ZN13user_creature12CCreatureMgr20RegisterCreatureItemEPNS_13CCreatureItemEi
    // DEMANGLED: user_creature::CCreatureMgr::RegisterCreatureItem(user_creature::CCreatureItem *,int)
    int RegisterCreatureItem(CCreatureItem* item, int key);

    // IDA_EA: 0x833A942
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr21IsExistedCreatureItemEi
    // DEMANGLED: user_creature::CCreatureMgr::IsExistedCreatureItem(int)const
    int IsExistedCreatureItem(int slot) const;

    // IDA_EA: 0x833B606
    // IDA_NAME: _ZN13user_creature12CCreatureMgr18DeleteCreatureItemEii
    // DEMANGLED: user_creature::CCreatureMgr::DeleteCreatureItem(int,int)
    int DeleteCreatureItem(int key, int reserved);

    // IDA_EA: 0x833AA32
    // IDA_NAME: _ZN13user_creature12CCreatureMgr8SendMailEP10Inven_Itemii
    // DEMANGLED: user_creature::CCreatureMgr::SendMail(Inven_Item *,int,int)
    int SendMail(Inven_Item* item, int slot, int creature_flag);

    // IDA_EA: 0x833AB9A
    // IDA_NAME: _ZN13user_creature12CCreatureMgr8RecvMailEP10Inven_Itemii
    // DEMANGLED: user_creature::CCreatureMgr::RecvMail(Inven_Item *,int,int)
    int RecvMail(Inven_Item* item, int slot, int creature_flag);

    // IDA_EA: 0x833B09C
    // IDA_NAME: _ZN13user_creature12CCreatureMgr13AuctionRegistEP10Inven_Itemii
    // DEMANGLED: user_creature::CCreatureMgr::AuctionRegist(Inven_Item *,int,int)
    int AuctionRegist(Inven_Item* item, int slot, int creature_flag);

    // IDA_EA: 0x833F10C
    // IDA_NAME: _ZN13user_creature12CCreatureMgr17setTempExpireTimeEi
    // DEMANGLED: user_creature::CCreatureMgr::setTempExpireTime(int)
    CCreatureMgr* setTempExpireTime(int expire_time);

    // IDA_EA: 0x833B204
    // IDA_NAME: _ZN13user_creature12CCreatureMgr18InsertCreatureItemEP10Inven_Itemiiii
    // DEMANGLED: user_creature::CCreatureMgr::InsertCreatureItem(Inven_Item *,int,int,int,int)
    int InsertCreatureItem(Inven_Item* item,
                           int slot,
                           int creature_flag,
                           int duration_days,
                           int delete_tick);

    // IDA_EA: 0x833B620
    // IDA_NAME: _ZN13user_creature12CCreatureMgr22SetCreatureItemAddInfoEii
    // DEMANGLED: user_creature::CCreatureMgr::SetCreatureItemAddInfo(int,int)
    int SetCreatureItemAddInfo(int slot, int add_info);

    // IDA_EA: 0x833A5BC
    // IDA_NAME: _ZN13user_creature12CCreatureMgr7UseItemEP10Inven_Itemi
    // DEMANGLED: user_creature::CCreatureMgr::UseItem(Inven_Item *,int)
    int UseItem(Inven_Item* item, int slot);

    // IDA_EA: 0x8339B06
    // IDA_NAME: _ZN13user_creature12CCreatureMgr7GainExpEi
    // DEMANGLED: user_creature::CCreatureMgr::GainExp(int)
    int GainExp(int item_id);

    // IDA_EA: 0x8339DBA
    // IDA_NAME: _ZN13user_creature12CCreatureMgr7UseFeedEv
    // DEMANGLED: user_creature::CCreatureMgr::UseFeed(void)
    int UseFeed();

    // IDA_EA: 0x833BE56
    // IDA_NAME: _ZN13user_creature12CCreatureMgr11GetArtifactEi
    // DEMANGLED: user_creature::CCreatureMgr::GetArtifact(int)
    CArtifact* GetArtifact(int slot);

    // IDA_EA: 0x833AEDC
    // IDA_NAME: _ZN13user_creature12CCreatureMgr12ChangeSlotNoEP10Inven_Itemii
    // DEMANGLED: user_creature::CCreatureMgr::ChangeSlotNo(Inven_Item *,int,int)
    int ChangeSlotNo(Inven_Item* item, int slot, int reason);

    // IDA_EA: 0x833C28A
    // IDA_NAME: _ZN13user_creature12CCreatureMgr11TurnStomachEb
    // DEMANGLED: user_creature::CCreatureMgr::TurnStomach(bool)
    char TurnStomach(bool fighting);

    // IDA_EA: 0x833A638
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr17GetCreatureItemIdEv
    // DEMANGLED: user_creature::CCreatureMgr::GetCreatureItemId(void)const
    int GetCreatureItemId() const;

    // IDA_EA: 0x833A660
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr13IsDieCreatureEv
    // DEMANGLED: user_creature::CCreatureMgr::IsDieCreature(void)const
    int IsDieCreature() const;

    // 0828 CMD549 reconstruction: expose the authoritative equipped-creature
    // identity without adding storage to the restored 0x4c-byte manager.
    bool GetActiveCreatureSkillContext(int& creature_uid,
                                       int& item_id,
                                       int& level,
                                       int& state) const;

    // Applies a server-resolved [fill point], persists creature_items.stomach
    // through internal packet 138, and publishes the absolute value in
    // NOTI103.  expected_uid closes an equip/swap race between validation and
    // mutation.
    int ApplyFillPoint(int expected_uid,
                       int fill_point,
                       int& absolute_stomach);

    // IDA_EA: 0x833A688
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr15GetCreatureNameEv
    // DEMANGLED: user_creature::CCreatureMgr::GetCreatureName(void)const
    const char* GetCreatureName() const;

    // IDA_EA: 0x833A6B0
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr23GetEquipedCreatureLevelEv
    // DEMANGLED: user_creature::CCreatureMgr::GetEquipedCreatureLevel(void)const
    int GetEquipedCreatureLevel() const;

    // IDA_EA: 0x8339B6E
    // IDA_NAME: _ZN13user_creature12CCreatureMgr14RenameCreatureEPKcii
    // DEMANGLED: user_creature::CCreatureMgr::RenameCreature(char const*,int,int)
    int RenameCreature(const char* name, int slot, int rename_mode);

    // IDA_EA: 0x8339FA0
    // IDA_NAME: _ZN13user_creature12CCreatureMgr5HatchEi
    // DEMANGLED: user_creature::CCreatureMgr::Hatch(int)
    int Hatch(int add_info);

    // IDA_EA: 0x833A6D8
    // IDA_NAME: _ZN13user_creature12CCreatureMgr16ResponseCreatureEv
    // DEMANGLED: user_creature::CCreatureMgr::ResponseCreature(void)
    int ResponseCreature();

    // IDA_EA: 0x833B592
    // IDA_NAME: _ZN13user_creature12CCreatureMgr24UpdateCreatureItemResultEP17SIG_CREATURE_ITEM
    // DEMANGLED: user_creature::CCreatureMgr::UpdateCreatureItemResult(SIG_CREATURE_ITEM *)
    int UpdateCreatureItemResult(SIG_CREATURE_ITEM* signal);

    // IDA_EA: 0x833B38C
    // IDA_NAME: _ZN13user_creature12CCreatureMgr24InsertCreatureItemResultEP17SIG_CREATURE_ITEM
    // DEMANGLED: user_creature::CCreatureMgr::InsertCreatureItemResult(SIG_CREATURE_ITEM *)
    int InsertCreatureItemResult(SIG_CREATURE_ITEM* signal);

    // Original callees retained as canonical member dependencies.
    int LoadCreatureItem(Inven_Item* item,
                         SIG_CREATURE_ITEM_DETAIL_INFO* detail);
    int SendCreatureItemInfo(SIG_CREATURE_ITEM* signal);

    // IDA_EA: 0x833BDD2
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr20SendCreatureItemListEv
    // DEMANGLED: user_creature::CCreatureMgr::SendCreatureItemList(void)const
    int SendCreatureItemList() const;

    // IDA_EA: 0x833BAEE
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr21checkMaxCreatureLevelEib
    // DEMANGLED: user_creature::CCreatureMgr::checkMaxCreatureLevel(int,bool)const
    bool checkMaxCreatureLevel(int item_id, bool grow_creature) const;

    // IDA_EA: 0x833BBB2
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr22isGrowCreatureMaxLevelEv
    // DEMANGLED: user_creature::CCreatureMgr::isGrowCreatureMaxLevel(void)const
    int isGrowCreatureMaxLevel() const;

    // IDA_EA: 0x833BE7C
    // IDA_NAME: _ZN13user_creature12CCreatureMgr12LoadArtifactEiP10Inven_Item
    // DEMANGLED: user_creature::CCreatureMgr::LoadArtifact(int,Inven_Item *)
    bool LoadArtifact(int slot, Inven_Item* item);

    // IDA_EA: 0x833BEB4
    // IDA_NAME: _ZN13user_creature12CCreatureMgr7EvoluteEiib
    // DEMANGLED: user_creature::CCreatureMgr::Evolute(int,int,bool)
    int Evolute(int creature_id, int required_level, bool force);

    // IDA_EA: 0x833C31A
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr12GetExtraGoldEi
    // DEMANGLED: user_creature::CCreatureMgr::GetExtraGold(int)const
    int GetExtraGold(int gold) const;

    // IDA_EA: 0x833C00C
    // IDA_NAME: _ZN13user_creature12CCreatureMgr16SetKeepRightItemEP5CUserP10Inven_Item
    // DEMANGLED: user_creature::CCreatureMgr::SetKeepRightItem(CUser *,Inven_Item *)
    bool SetKeepRightItem(CUser* user, Inven_Item* item);

    // IDA_EA: 0x833C240
    // IDA_NAME: _ZN13user_creature12CCreatureMgr9CheckItemEP5CUserP10Inven_ItemRb
    // DEMANGLED: user_creature::CCreatureMgr::CheckItem(CUser *,Inven_Item *,bool &)
    bool CheckItem(CUser* user, Inven_Item* item, bool& valid);

    // IDA_EA: 0x833C41E
    // IDA_NAME: _ZN13user_creature12CCreatureMgr16SetOverSkillTimeERKi
    // DEMANGLED: user_creature::CCreatureMgr::SetOverSkillTime(int const&)
    int SetOverSkillTime(const int& item_id);

    // IDA_EA: 0x833C6F6
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr21GetOverSkillExtraGoldEi
    // DEMANGLED: user_creature::CCreatureMgr::GetOverSkillExtraGold(int)const
    int GetOverSkillExtraGold(int gold) const;

    // IDA_EA: 0x833D6C2
    // IDA_NAME: _ZN13user_creature12CCreatureMgr11ItemEvoluteEi
    // DEMANGLED: user_creature::CCreatureMgr::ItemEvolute(int)
    int ItemEvolute(int member_count);

    // IDA_EA: 0x833DA1E
    // IDA_NAME: _ZN13user_creature12CCreatureMgr20EventEvoluteCreatureEiii
    // DEMANGLED: user_creature::CCreatureMgr::EventEvoluteCreature(int,int,int)
    int EventEvoluteCreature(int item_id,
                             int required_level,
                             int client_creature_index);

    // IDA_EA: 0x833D6FA
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr26VerifyEquipedCreatureQuestEv
    // DEMANGLED: user_creature::CCreatureMgr::VerifyEquipedCreatureQuest(void)const
    int VerifyEquipedCreatureQuest() const;

    // IDA_EA: 0x833D7DE
    // IDA_NAME: _ZN13user_creature12CCreatureMgr16RegistTempPeriodEii
    // DEMANGLED: user_creature::CCreatureMgr::RegistTempPeriod(int,int)
    int RegistTempPeriod(int key, int period);

    // IDA_EA: 0x833D88C
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr19getRemainTempPeriodEii
    // DEMANGLED: user_creature::CCreatureMgr::getRemainTempPeriod(int,int)const
    int getRemainTempPeriod(int key, int current_tick) const;

    // IDA_EA: 0x833D962
    // IDA_NAME: _ZN13user_creature12CCreatureMgr18UnRegistTempPeriodEi
    // DEMANGLED: user_creature::CCreatureMgr::UnRegistTempPeriod(int)
    int UnRegistTempPeriod(int key);

    // IDA_EA: 0x833D9D8
    // IDA_NAME: _ZN13user_creature12CCreatureMgr22LoadCreatureTempPeriodEP31SIG_POSTAL_CREATURE_TEMP_PERIOD
    // DEMANGLED: user_creature::CCreatureMgr::LoadCreatureTempPeriod(SIG_POSTAL_CREATURE_TEMP_PERIOD *)
    int LoadCreatureTempPeriod(SIG_POSTAL_CREATURE_TEMP_PERIOD* period);

    // IDA_EA: 0x833DEBC
    // IDA_NAME: _ZNK13user_creature12CCreatureMgr32IsGrowCreature_Equipped_CreatureEv
    // DEMANGLED: user_creature::CCreatureMgr::IsGrowCreature_Equipped_Creature(void)const
    bool IsGrowCreature_Equipped_Creature() const;

private:
    friend struct CCreatureMgrLayoutVerifier;

    CCreatureItemMap creature_items_;             // +0x00
    CCreature* active_creature_;                   // +0x18
    CUser* owner_;                                 // +0x1c
    CArtifact artifacts_[3];                       // +0x20
    int temporary_expire_time_;                    // +0x2c
    CCreatureTempPeriodMap temporary_periods_;     // +0x30
    uint8_t save_state_;                           // +0x48
};
#if defined(__i386__)
struct CCreatureMgrLayoutVerifier {
    static_assert(offsetof(CCreatureMgr, creature_items_) == 0x00u,
                  "CCreatureMgr items must stay at IDA +0x00");
    static_assert(offsetof(CCreatureMgr, active_creature_) == 0x18u,
                  "CCreatureMgr active creature must stay at IDA +0x18");
    static_assert(offsetof(CCreatureMgr, owner_) == 0x1cu,
                  "CCreatureMgr owner must stay at IDA +0x1c");
    static_assert(offsetof(CCreatureMgr, artifacts_) == 0x20u,
                  "CCreatureMgr artifacts must stay at IDA +0x20");
    static_assert(offsetof(CCreatureMgr, temporary_expire_time_) == 0x2cu,
                  "CCreatureMgr temporary expiration must stay at IDA +0x2c");
    static_assert(offsetof(CCreatureMgr, temporary_periods_) == 0x30u,
                  "CCreatureMgr temporary periods must stay at IDA +0x30");
    static_assert(offsetof(CCreatureMgr, save_state_) == 0x48u,
                  "CCreatureMgr save state must stay at IDA +0x48");
    static_assert(sizeof(CCreatureMgr) == 0x4cu,
                  "CCreatureMgr IDA ABI size must stay 0x4c");
};
#endif
class CEgg : public CCreatureItem {
public:
    // IDA_EA: 0x8338F4C
    // IDA_NAME: _ZN13user_creature4CEggC2Ev
    // DEMANGLED: user_creature::CEgg::CEgg(void)
    CEgg();
    // IDA_EA: 0x8338F74
    // IDA_NAME: _ZN13user_creature4CEggD2Ev
    // DEMANGLED: user_creature::CEgg::~CEgg(void)
    ~CEgg();

    // IDA_EA: 0x8338F82
    // IDA_NAME: _ZN13user_creature4CEgg5ResetEv
    // DEMANGLED: user_creature::CEgg::Reset(void)
    virtual void Reset();

    // IDA_EA: 0x83390C8
    // IDA_NAME: _ZN13user_creature4CEgg16SaveCreatureItemEP10Inven_ItemP29SIG_CREATURE_ITEM_DETAIL_INFOP5CUser
    // DEMANGLED: user_creature::CEgg::SaveCreatureItem(Inven_Item *,SIG_CREATURE_ITEM_DETAIL_INFO *,CUser *)
    virtual int SaveCreatureItem(Inven_Item* item,
                                 SIG_CREATURE_ITEM_DETAIL_INFO* detail,
                                 CUser* user);

    // IDA_EA: 0x8338FAC
    // IDA_NAME: _ZN13user_creature4CEgg16LoadCreatureItemEPNS_12CCreatureMgrEP10Inven_ItemP29SIG_CREATURE_ITEM_DETAIL_INFOP5CUser
    // DEMANGLED: user_creature::CEgg::LoadCreatureItem(user_creature::CCreatureMgr *,Inven_Item *,SIG_CREATURE_ITEM_DETAIL_INFO *,CUser *)
    virtual int LoadCreatureItem(CCreatureMgr* manager,
                                 Inven_Item* item,
                                 SIG_CREATURE_ITEM_DETAIL_INFO* detail,
                                 CUser* user);

    // IDA_EA: 0x833F102
    // IDA_NAME: _ZN13user_creature4CEgg6IsSaveEv
    // DEMANGLED: user_creature::CEgg::IsSave(void)
    virtual int IsSave();

    // IDA_EA: 0x833912A
    // IDA_NAME: _ZN13user_creature4CEgg5HatchEPi
    // DEMANGLED: user_creature::CEgg::Hatch(int *)
    int Hatch(int* hatch_item_id);
};
#if defined(__i386__)
static_assert(sizeof(CEgg) == 0x1cu,
              "CEgg IDA ABI size must stay 0x1c");
#endif

class CCreatureItemFactory;

CCreatureItemFactory* GetInstanceCreatureItemFactory();

// IDA_EA: 0x8336C63 / 0x8336C80 / 0x8336C9D / 0x8336CBA.
// ProcessServerStatusLog queries these named factory counters; it must not
// infer the two DynamicPool members from their object offsets.
int GetCreatureEggPoolSize();
int GetCreaturePoolSize();
int GetCreatureEggPoolRemainSize();
int GetCreaturePoolRemainSize();

// IDA enum: user_creature::CREATURE_STATE.  The known values below are used
// by the original SetState transition; callers retain their raw enum value.
enum CREATURE_STATE {
    CREATURE_STATE_ABI_0 = 0,
    CREATURE_STATE_ABI_1 = 1,
    CREATURE_STATE_ABI_2 = 2,
    CREATURE_STATE_ABI_3 = 3,
};

// IDA unit: user_creature::CStomach.  The timer value and the current stomach
// value occupy the verified eight-byte record used by CCreature at +52.
class CStomach {
public:
    // IDA_EA: 0x8336CD8
    // IDA_NAME: _ZN13user_creature8CStomachC2Ev
    // DEMANGLED: user_creature::CStomach::CStomach(void)
    CStomach();

    // IDA_EA: 0x8336E66
    // IDA_NAME: _ZN13user_creature8CStomach14IncrementValueEf
    // DEMANGLED: user_creature::CStomach::IncrementValue(float)
    long double IncrementValue(float value);

    // IDA_EA: 0x8336CFC
    // IDA_NAME: _ZN13user_creature8CStomach15GetStomachValueENS_14CREATURE_STATEEb
    // DEMANGLED: user_creature::CStomach::GetStomachValue(user_creature::CREATURE_STATE,bool)
    long double GetStomachValue(CREATURE_STATE state, bool grow_type);

    // IDA_EA: 0x8336DEE
    // IDA_NAME: _ZN13user_creature8CStomach13IsSaveStomachENS_14CREATURE_STATEE
    // DEMANGLED: user_creature::CStomach::IsSaveStomach(user_creature::CREATURE_STATE)
    int IsSaveStomach(CREATURE_STATE state);

    // IDA_EA: 0x8336E4C
    // IDA_NAME: _ZN13user_creature8CStomach6ResumeEv
    // DEMANGLED: user_creature::CStomach::Resume(void)
    int Resume();

    // IDA_EA: 0x833EEF6
    // IDA_NAME: _ZN13user_creature8CStomach15SetStomachValueEf
    // DEMANGLED: user_creature::CStomach::SetStomachValue(float)
    CStomach* SetStomachValue(float value);

private:
    int last_tick_;
    float value_;
};
static_assert(sizeof(CStomach) == 0x08u,
              "CStomach IDA ABI size must stay 0x08");

// IDA unit: user_creature::CPacketRespondent.  C2 invokes PacketGuard::C2 at
// +0x00, then writes the four dwords at +0x0c..+0x18 and the dirty byte at
// +0x1c.  Model that inheritance and every named trailing field directly;
// the compiler supplies only the ABI-required trailing alignment bytes.
class CPacketRespondent : public PacketGuard {
public:
    // IDA_EA: 0x834032A
    // IDA_NAME: _ZN13user_creature17CPacketRespondentC2EP5CUserii
    // DEMANGLED: user_creature::CPacketRespondent::CPacketRespondent(CUser *,int,int)
    CPacketRespondent(CUser* user, int packet_class, int packet_id);
    // IDA_EA: 0x834036A
    // IDA_NAME: _ZN13user_creature17CPacketRespondentD2Ev
    // DEMANGLED: user_creature::CPacketRespondent::~CPacketRespondent(void)
    ~CPacketRespondent();

    // IDA_EA: 0x83403B8
    // IDA_NAME: _ZN13user_creature17CPacketRespondent8finalizeEb
    // DEMANGLED: user_creature::CPacketRespondent::finalize(bool)
    int finalize(bool encrypt);

    // IDA_EA: 0x83403E0
    // IDA_NAME: _ZN13user_creature17CPacketRespondent4SendEv
    // DEMANGLED: user_creature::CPacketRespondent::Send(void)
    int Send();

    // IDA_EA: 0x834048A
    // IDA_NAME: _ZN13user_creature17CPacketRespondent7SendAllEv
    // DEMANGLED: user_creature::CPacketRespondent::SendAll(void)
    int SendAll();

    // IDA_EA: 0x8340256
    // IDA_NAME: _ZN13user_creature17CPacketRespondent10IsGateAreaEP5CUseri
    // DEMANGLED: user_creature::CPacketRespondent::IsGateArea(CUser *,int)
    bool IsGateArea(CUser* user, int area_index);

    // IDA_EA: 0x8340404
    // IDA_NAME: _ZN13user_creature17CPacketRespondent9BroadcastEii
    // DEMANGLED: user_creature::CPacketRespondent::Broadcast(int,int)
    int Broadcast(int village_index, int area_index);

    // IDA_EA: 0x8340760
    // IDA_NAME: _ZN13user_creature17CPacketRespondent29MakeNotipacketGainExpCreatureEiib
    // DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketGainExpCreature(int,int,bool)
    int MakeNotipacketGainExpCreature(int exp, int level, bool grow_type);
    // IDA_EA: 0x83404B8
    // IDA_NAME: _ZN13user_creature17CPacketRespondent20SendAccordingToPlaceEv
    // DEMANGLED: user_creature::CPacketRespondent::SendAccordingToPlace(void)
    int SendAccordingToPlace();

    // IDA_EA: 0x83406FE
    // IDA_NAME: _ZN13user_creature17CPacketRespondent26MakeNotipacketDiedCreatureEv
    // DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketDiedCreature(void)
    int MakeNotipacketDiedCreature();

    // IDA_EA: 0x8340842
    // IDA_NAME: _ZN13user_creature17CPacketRespondent29MakeNotipacketEvoluteCreatureEi
    // DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketEvoluteCreature(int)
    int MakeNotipacketEvoluteCreature(int creature_id);

    // IDA_EA: 0x83408B6
    // IDA_NAME: _ZN13user_creature17CPacketRespondent29MakeNotipacketRevivalCreatureEv
    // DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketRevivalCreature(void)
    int MakeNotipacketRevivalCreature();

    // IDA_EA: 0x8340918
    // IDA_NAME: _ZN13user_creature17CPacketRespondent27MakeNotipacketCreatureStateEii
    // DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketCreatureState(int,int)
    int MakeNotipacketCreatureState(int creature_uid, int state);

    // IDA_EA: 0x8340538
    // IDA_NAME: _ZN13user_creature17CPacketRespondent8SetErrorEi
    // DEMANGLED: user_creature::CPacketRespondent::SetError(int)
    int SetError(int error);

    // IDA_EA: 0x83405A2
    // IDA_NAME: _ZN13user_creature17CPacketRespondent5SetOkEv
    // DEMANGLED: user_creature::CPacketRespondent::SetOk(void)
    int SetOk();

    // IDA_EA: 0x83405FA
    // IDA_NAME: _ZN13user_creature17CPacketRespondent27MakeCmdpacketRenameCreatureEii
    // DEMANGLED: user_creature::CPacketRespondent::MakeCmdpacketRenameCreature(int,int)
    int MakeCmdpacketRenameCreature(int slot, int rename_mode);

    // IDA_EA: 0x8340672
    // IDA_NAME: _ZN13user_creature17CPacketRespondent28MakeNotipacketRenameCreatureEPKci
    // DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketRenameCreature(char const*,int)
    int MakeNotipacketRenameCreature(const char* name, int name_length);

    // IDA_EA: 0x834097E
    // IDA_NAME: _ZN13user_creature17CPacketRespondent30MakeNotipacketResponseCreatureEv
    // DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketResponseCreature(void)
    int MakeNotipacketResponseCreature();

    // IDA_EA: 0x83409E0
    // IDA_NAME: _ZN13user_creature17CPacketRespondent30MakeNotipacketCreatureItemListEPKSt3mapIiPNS_13CCreatureItemESt4lessIiESaISt4pairIKiS3_EEEi
    // DEMANGLED: user_creature::CPacketRespondent::MakeNotipacketCreatureItemList(std::map<int,user_creature::CCreatureItem *,std::less<int>,std::allocator<std::pair<int const,user_creature::CCreatureItem *>>> const*,int)
    int MakeNotipacketCreatureItemList(
        const std::map<int, CCreatureItem*>* creature_items,
        int slot);

private:
    CUser* user_;             // IDA +0x0c
    int packet_class_;        // IDA +0x10
    int packet_id_;           // IDA +0x14
    int reserved_zero_;       // IDA +0x18, set to zero by C2
    bool send_on_destroy_;    // IDA +0x1c
};
static_assert(sizeof(CPacketRespondent) == 0x20u,
              "CPacketRespondent IDA ABI size must stay 0x20");

// IDA unit: user_creature::CCreature.  The full creature item base and tail
// are restored in their own IDA bodies; these verified state-machine offsets
// are exposed here so C has one shared class identity.
class CCreature : public CCreatureItem {
public:
    // IDA_EA: 0x833705E
    // IDA_NAME: _ZN13user_creature9CCreatureC2Ev
    // DEMANGLED: user_creature::CCreature::CCreature(void)
    CCreature();
    // IDA_EA: 0x83370A2
    // IDA_NAME: _ZN13user_creature9CCreatureD2Ev
    // DEMANGLED: user_creature::CCreature::~CCreature(void)
    ~CCreature();

    // IDA_EA: 0x83370B0
    // IDA_NAME: _ZN13user_creature9CCreature5ResetEv
    // DEMANGLED: user_creature::CCreature::Reset(void)
    virtual void Reset();

    // IDA_EA: 0x8337330
    // IDA_NAME: _ZN13user_creature9CCreature5EquipEP5CUser
    // DEMANGLED: user_creature::CCreature::Equip(CUser *)
    int Equip(CUser* user);

    // IDA_EA: 0x8337382
    // IDA_NAME: _ZN13user_creature9CCreature9DismantleEP5CUser
    // DEMANGLED: user_creature::CCreature::Dismantle(CUser *)
    int Dismantle(CUser* user);

    // IDA_EA: 0x83373AA
    // IDA_NAME: _ZN13user_creature9CCreature14RenameCreatureEPKc
    // DEMANGLED: user_creature::CCreature::RenameCreature(char const*)
    int RenameCreature(const char* name);

    // IDA_EA: 0x83384B8
    // IDA_NAME: _ZN13user_creature9CCreature17CheckDiedCreatureEP5CUser
    // DEMANGLED: user_creature::CCreature::CheckDiedCreature(CUser *)
    int CheckDiedCreature(CUser* user);

    // IDA_EA: 0x83388EC
    // IDA_NAME: _ZN13user_creature9CCreature19IsEvolutionCreatureEi
    // DEMANGLED: user_creature::CCreature::IsEvolutionCreature(int)
    bool IsEvolutionCreature(int creature_id);

    // IDA_EA: 0x8338BFA
    // IDA_NAME: _ZN13user_creature9CCreature24IsEventEvolutionCreatureEi
    // DEMANGLED: user_creature::CCreature::IsEventEvolutionCreature(int)
    int IsEventEvolutionCreature(int creature_id);

    // Original evolution/death callees retained as member dependencies.
    int Die(CUser* user);
    int IsEvolutionCreatureMulty(int creature_id);
    int EvoluteMulty(CUser* user, int creature_id);
    int ItemEvolute(CUser* user, int member_count);

    // IDA_EA: 0x833DB2E
    // IDA_NAME: _ZN13user_creature9CCreature12EventEvoluteEP5CUserii
    // DEMANGLED: user_creature::CCreature::EventEvolute(CUser *,int,int)
    int EventEvolute(CUser* user,
                     int item_id,
                     int client_creature_index);

    // IDA_EA: 0x83375AE
    // IDA_NAME: _ZN13user_creature9CCreature9SetItemIdEi
    // DEMANGLED: user_creature::CCreature::SetItemId(int)
    CCreature* SetItemId(int item_id);
    // IDA_EA: 0x833769C
    // IDA_NAME: _ZN13user_creature9CCreature13GetCreatureIdEi
    // DEMANGLED: user_creature::CCreature::GetCreatureId(int)
    int GetCreatureId(int item_id);

    // IDA_EA: 0x833715A
    // IDA_NAME: _ZN13user_creature9CCreature8SetStateEP5CUserNS_14CREATURE_STATEE
    // DEMANGLED: user_creature::CCreature::SetState(CUser *,user_creature::CREATURE_STATE)
    CCreature* SetState(CUser* user, CREATURE_STATE state);

    // IDA_EA: 0x833F064
    // IDA_NAME: _ZN13user_creature9CCreature8GetStateEv
    // DEMANGLED: user_creature::CCreature::GetState(void)
    int GetState();

    // IDA_EA: 0x83373D4
    // IDA_NAME: _ZN13user_creature9CCreature7UseFeedEP5CUserRi
    // DEMANGLED: user_creature::CCreature::UseFeed(CUser *,int &)
    int UseFeed(CUser* user, int& exp);

    // IDA_EA: 0x83374D2
    // IDA_NAME: _ZN13user_creature9CCreature11GetExpLevelEi
    // DEMANGLED: user_creature::CCreature::GetExpLevel(int)
    int GetExpLevel(int exp);

    // IDA_EA: 0x83376F6
    // IDA_NAME: _ZN13user_creature9CCreature19IsAvailableArtifactEi
    // DEMANGLED: user_creature::CCreature::IsAvailableArtifact(int)
    int IsAvailableArtifact(int artifact_slot);

    // IDA_EA: 0x833756E
    // IDA_NAME: _ZN13user_creature9CCreature10isMaxLevelEv
    // DEMANGLED: user_creature::CCreature::isMaxLevel(void)
    int isMaxLevel();

    // IDA_EA: 0x83378AA
    // IDA_NAME: _ZN13user_creature9CCreature12vaildItemExpEi
    // DEMANGLED: user_creature::CCreature::vaildItemExp(int)
    int vaildItemExp(int item_id);

    // IDA_EA: 0x8338C28
    // IDA_NAME: _ZN13user_creature9CCreature20GetSkillRecoveryTimeEv
    // DEMANGLED: user_creature::CCreature::GetSkillRecoveryTime(void)
    int GetSkillRecoveryTime();

    // IDA_EA: 0x8338C50
    // IDA_NAME: _ZN13user_creature9CCreature24GetOverSkillRecoveryTimeEv
    // DEMANGLED: user_creature::CCreature::GetOverSkillRecoveryTime(void)
    int GetOverSkillRecoveryTime();

    // IDA_EA: 0x8338C78
    // IDA_NAME: _ZN13user_creature9CCreature22GetLearnOverSkillLevelEv
    // DEMANGLED: user_creature::CCreature::GetLearnOverSkillLevel(void)
    int GetLearnOverSkillLevel();

    // IDA_EA: 0x83378FC
    // IDA_NAME: _ZN13user_creature9CCreature7GainExpEiP5CUser
    // DEMANGLED: user_creature::CCreature::GainExp(int,CUser *)
    int GainExp(int item_id, CUser* user);

    // IDA_EA: 0x83379C0
    // IDA_NAME: _ZN13user_creature9CCreature19GainItemExpCreatureEiP5CUser
    // DEMANGLED: user_creature::CCreature::GainItemExpCreature(int,CUser *)
    int GainItemExpCreature(int item_id, CUser* user);

    // IDA_EA: 0x8337BBC
    // IDA_NAME: _ZN13user_creature9CCreature15GainExpCreatureEiP5CUser
    // DEMANGLED: user_creature::CCreature::GainExpCreature(int,CUser *)
    int GainExpCreature(int item_id, CUser* user);

    // IDA_EA: 0x8337724
    // IDA_NAME: _ZN13user_creature9CCreature9GetAddExpEiP5CUser
    // DEMANGLED: user_creature::CCreature::GetAddExp(int,CUser *)
    long double GetAddExp(int artifact_slot, CUser* user);

    // IDA_EA: 0x83377B6
    // IDA_NAME: _ZN13user_creature9CCreature12CalculateExpEiRbP5CUser
    // DEMANGLED: user_creature::CCreature::CalculateExp(int,bool &,CUser *)
    CCreature* CalculateExp(int item_id, bool& level_up, CUser* user);

    // IDA_EA: 0x8337D58
    // IDA_NAME: _ZN13user_creature9CCreature19IsAbleSelectEvoluteEP5CUser
    // DEMANGLED: user_creature::CCreature::IsAbleSelectEvolute(CUser *)
    int IsAbleSelectEvolute(CUser* user);

    // IDA_EA: 0x8337E48
    // IDA_NAME: _ZN13user_creature9CCreature13SelectEvoluteEP5CUser
    // DEMANGLED: user_creature::CCreature::SelectEvolute(CUser *)
    int SelectEvolute(CUser* user);

    // IDA_EA: 0x83380FC
    // IDA_NAME: _ZNK13user_creature9CCreature13IsAbleEvoluteEP5CUser
    // DEMANGLED: user_creature::CCreature::IsAbleEvolute(CUser *)const
    int IsAbleEvolute(CUser* user) const;

    // IDA_EA: 0x83381EA
    // IDA_NAME: _ZN13user_creature9CCreature7EvoluteEP5CUser
    // DEMANGLED: user_creature::CCreature::Evolute(CUser *)
    int Evolute(CUser* user);

    // IDA_EA: 0x833F0CA
    // IDA_NAME: _ZN13user_creature9CCreature14IsGrowCreatureEv
    // DEMANGLED: user_creature::CCreature::IsGrowCreature(void)
    int IsGrowCreature();

    // IDA_EA: 0x8338478
    // IDA_NAME: _ZN13user_creature9CCreature13IsDieCreatureEv
    // DEMANGLED: user_creature::CCreature::IsDieCreature(void)
    bool IsDieCreature();

    // IDA_EA: 0x833F058
    // IDA_NAME: _ZN13user_creature9CCreature7GetNameEv
    // DEMANGLED: user_creature::CCreature::GetName(void)
    char* GetName();

    // IDA_EA: 0x833F09A
    // IDA_NAME: _ZN13user_creature9CCreature8GetLevelEv
    // DEMANGLED: user_creature::CCreature::GetLevel(void)
    int GetLevel();

    // IDA_EA: 0x83387F6
    // IDA_NAME: _ZN13user_creature9CCreature16SaveCreatureItemEP10Inven_ItemP29SIG_CREATURE_ITEM_DETAIL_INFOP5CUser
    // DEMANGLED: user_creature::CCreature::SaveCreatureItem(Inven_Item *,SIG_CREATURE_ITEM_DETAIL_INFO *,CUser *)
    virtual int SaveCreatureItem(Inven_Item* item,
                                 SIG_CREATURE_ITEM_DETAIL_INFO* detail,
                                 CUser* user);

    // IDA_EA: 0x83385AA
    // IDA_NAME: _ZN13user_creature9CCreature16LoadCreatureItemEPNS_12CCreatureMgrEP10Inven_ItemP29SIG_CREATURE_ITEM_DETAIL_INFOP5CUser
    // DEMANGLED: user_creature::CCreature::LoadCreatureItem(user_creature::CCreatureMgr *,Inven_Item *,SIG_CREATURE_ITEM_DETAIL_INFO *,CUser *)
    virtual int LoadCreatureItem(CCreatureMgr* manager,
                                 Inven_Item* item,
                                 SIG_CREATURE_ITEM_DETAIL_INFO* detail,
                                 CUser* user);

    // IDA_EA: 0x833F04C
    // IDA_NAME: _ZN13user_creature9CCreature6IsSaveEv
    // DEMANGLED: user_creature::CCreature::IsSave(void)
    virtual int IsSave();

    // IDA_EA: 0x83384EE
    // IDA_NAME: _ZN13user_creature9CCreature10GetStomachEv
    // DEMANGLED: user_creature::CCreature::GetStomach(void)
    int GetStomach();

    // Reconstructed CMD549 bridge to the canonical CStomach clamp.
    int IncrementStomachValue(int value);

    // IDA_EA: 0x833F070
    // IDA_NAME: _ZN13user_creature9CCreature10SetStomachEi
    // DEMANGLED: user_creature::CCreature::SetStomach(int)
    CStomach* SetStomach(int value);

    // IDA_EA: 0x833F08E
    // IDA_NAME: _ZN13user_creature9CCreature6GetExpEv
    // DEMANGLED: user_creature::CCreature::GetExp(void)
    int GetExp();

    // IDA_EA: 0x833F0A6
    // IDA_NAME: _ZN13user_creature9CCreature10IsNoChargeEv
    // DEMANGLED: user_creature::CCreature::IsNoCharge(void)
    int IsNoCharge();

    // IDA_EA: 0x833F0B2
    // IDA_NAME: _ZN13user_creature9CCreature11SetNoChargeEb
    // DEMANGLED: user_creature::CCreature::SetNoCharge(bool)
    CCreature* SetNoCharge(bool no_charge);

    // IDA_EA: 0x833F0DA
    // IDA_NAME: _ZN13user_creature9CCreature13GetCreatureIdEv
    // DEMANGLED: user_creature::CCreature::GetCreatureId(void)
    int GetCreatureId();

    // IDA_EA: 0x833F0E6
    // IDA_NAME: _ZN13user_creature9CCreature16GetOverSkillTimeEv
    // DEMANGLED: user_creature::CCreature::GetOverSkillTime(void)
    COverSkillTime* GetOverSkillTime();

    // IDA_EA: 0x833F0F2
    // IDA_NAME: _ZN13user_creature9CCreature16IsCreatureScriptEv
    // DEMANGLED: user_creature::CCreature::IsCreatureScript(void)
    bool IsCreatureScript();

private:
    friend struct CCreatureLayoutVerifier;

    char name_[16];                       // +0x1c
    int level_;                           // +0x2c
    int experience_;                      // +0x30
    CStomach stomach_;                    // +0x34
    CREATURE_STATE state_;                // +0x3c
    uint8_t state_dirty_;                 // +0x40
    uint8_t pad_041_043_[3];              // +0x41
    CCreatureScript* creature_script_;    // +0x44
    int creature_id_;                     // +0x48
    uint8_t item_grow_state_;             // +0x4c
    uint8_t item_grow_balance_;           // +0x4d
    uint8_t pad_04e_04f_[2];              // +0x4e
    COverSkillTime over_skill_time_;       // +0x50
};

#if defined(__i386__)
struct CCreatureLayoutVerifier {
    static_assert(offsetof(CCreature, name_) == 0x1cu,
                  "CCreature name must stay at IDA +0x1c");
    static_assert(offsetof(CCreature, level_) == 0x2cu,
                  "CCreature level must stay at IDA +0x2c");
    static_assert(offsetof(CCreature, experience_) == 0x30u,
                  "CCreature experience must stay at IDA +0x30");
    static_assert(offsetof(CCreature, stomach_) == 0x34u,
                  "CCreature stomach must stay at IDA +0x34");
    static_assert(offsetof(CCreature, state_) == 0x3cu,
                  "CCreature state must stay at IDA +0x3c");
    static_assert(offsetof(CCreature, state_dirty_) == 0x40u,
                  "CCreature state-dirty flag must stay at IDA +0x40");
    static_assert(offsetof(CCreature, creature_script_) == 0x44u,
                  "CCreature script must stay at IDA +0x44");
    static_assert(offsetof(CCreature, creature_id_) == 0x48u,
                  "CCreature id must stay at IDA +0x48");
    static_assert(offsetof(CCreature, item_grow_state_) == 0x4cu,
                  "CCreature growth kind must stay at IDA +0x4c");
    static_assert(offsetof(CCreature, item_grow_balance_) == 0x4du,
                  "CCreature growth balance must stay at IDA +0x4d");
    static_assert(offsetof(CCreature, over_skill_time_) == 0x50u,
                  "CCreature over-skill timer must stay at IDA +0x50");
};
static_assert(sizeof(CCreature) == 0x54u,
              "CCreature IDA ABI size must stay 0x54");
#endif

// IDA unit: user_creature::CCreatureItemFactory.  The factory owns the two
// 0x5c-byte DynamicPool instances directly: eggs at +0x00 and creatures at
// +0x5c.  Keeping them as typed members gives the original constructor and
// destruction sequence instead of a raw-storage surrogate.
class CCreatureItemFactory {
public:
    // IDA_EA: 0x833F67A
    // IDA_NAME: _ZN13user_creature20CCreatureItemFactoryC2Ev
    // DEMANGLED: user_creature::CCreatureItemFactory::CCreatureItemFactory(void)
    CCreatureItemFactory();
    // IDA_EA: 0x833F6D0
    // IDA_NAME: _ZN13user_creature20CCreatureItemFactoryD2Ev
    // DEMANGLED: user_creature::CCreatureItemFactory::~CCreatureItemFactory(void)
    ~CCreatureItemFactory();

    // IDA_EA: 0x833F510
    // IDA_NAME: _ZN13user_creature20CCreatureItemFactory13GetRemainSizeENS_18CREATURE_ITME_TYPEE
    // DEMANGLED: user_creature::CCreatureItemFactory::GetRemainSize(user_creature::CREATURE_ITME_TYPE)
    int GetRemainSize(CREATURE_ITME_TYPE type);

    // IDA_EA: 0x833F546
    // IDA_NAME: _ZN13user_creature20CCreatureItemFactory7GetSizeENS_18CREATURE_ITME_TYPEE
    // DEMANGLED: user_creature::CCreatureItemFactory::GetSize(user_creature::CREATURE_ITME_TYPE)
    int GetSize(CREATURE_ITME_TYPE type);

    // IDA_EA: 0x833F57C
    // IDA_NAME: _ZN13user_creature20CCreatureItemFactory19AcquireCreatureItemENS_18CREATURE_ITME_TYPEE
    // DEMANGLED: user_creature::CCreatureItemFactory::AcquireCreatureItem(user_creature::CREATURE_ITME_TYPE)
    CCreatureItem* AcquireCreatureItem(CREATURE_ITME_TYPE type);

    // IDA_EA: 0x833F618
    // IDA_NAME: _ZN13user_creature20CCreatureItemFactory16FreeCreatureItemEPNS_13CCreatureItemE
    // DEMANGLED: user_creature::CCreatureItemFactory::FreeCreatureItem(user_creature::CCreatureItem *)
    int FreeCreatureItem(CCreatureItem* item);

private:
    DynamicPool<CEgg> egg_pool_;
    DynamicPool<CCreature> creature_pool_;
};

#if defined(__i386__)
static_assert(sizeof(CCreatureItemFactory) == 0xb8u,
              "CCreatureItemFactory IDA ABI size must stay 0xb8");
#endif

// CMD372 uses only these verified portions of the two runtime objects.  They
// are real typed ABI views, so the new provider does not recalculate byte
// offsets or use alias declarations for its calls.
struct CCreatureCmd372Layout {
    uint8_t creature_item[0x1cu];
    uint8_t before_creature_script[0x28u];
    CCreatureScript* creature_script;
    uint8_t between_script_and_over_skill_time[0x08u];
    COverSkillTime over_skill_time;
};

static_assert(offsetof(CCreatureCmd372Layout, creature_item) == 0x00u,
              "CCreature CMD372 item base must stay at IDA +0x00");
static_assert(offsetof(CCreatureCmd372Layout, creature_script) == 0x44u,
              "CCreature CMD372 script must stay at IDA +0x44");
static_assert(offsetof(CCreatureCmd372Layout, over_skill_time) == 0x50u,
              "CCreature CMD372 timer must stay at IDA +0x50");
static_assert(sizeof(CCreatureCmd372Layout) == 0x54u,
              "CCreature CMD372 layout must stay 0x54");

inline CCreatureCmd372Layout& CCreatureCmd372View(CCreature& creature) {
    return *reinterpret_cast<CCreatureCmd372Layout*>(&creature);
}

// IDA unit: user_creature::CDBMsgSender.  It owns only the C-side request
// writer state; DB dispatch and SQL remain B-owned.
struct CDBMsgSender {
    // IDA_EA: 0x833F8AE
    // IDA_NAME: _ZN13user_creature12CDBMsgSenderC2E19ENUM_INTERNALPACKETP5CUser
    // DEMANGLED: user_creature::CDBMsgSender::CDBMsgSender(ENUM_INTERNALPACKET,CUser *)
    CDBMsgSender(ENUM_INTERNALPACKET signal, CUser* user);

    // IDA_EA: 0x833F8D8
    // IDA_NAME: _ZN13user_creature12CDBMsgSenderD2Ev
    // DEMANGLED: user_creature::CDBMsgSender::~CDBMsgSender(void)
    ~CDBMsgSender();

    // IDA_EA: 0x833F7A8
    // IDA_NAME: _ZN13user_creature12CDBMsgSender4InitE19ENUM_INTERNALPACKETP5CUser
    // DEMANGLED: user_creature::CDBMsgSender::Init(ENUM_INTERNALPACKET,CUser *)
    int Init(ENUM_INTERNALPACKET signal, CUser* user);

    template <typename T>
    T* GetInBuffer();

    // IDA_EA: 0x833F950
    // IDA_NAME: _ZN13user_creature12CDBMsgSender4SendEv
    // DEMANGLED: user_creature::CDBMsgSender::Send(void)
    int Send();

    // IDA_EA: 0x833F994
    // IDA_NAME: _ZN13user_creature12CDBMsgSender6CancelEv
    // DEMANGLED: user_creature::CDBMsgSender::Cancel(void)
    CDBMsgSender* Cancel();

    // IDA_EA: 0x833FA0C
    // IDA_NAME: _ZN13user_creature12CDBMsgSender16SwapCreatureItemEP5CUserP10Inven_ItemiS4_i
    // DEMANGLED: user_creature::CDBMsgSender::SwapCreatureItem(CUser *,Inven_Item *,int,Inven_Item *,int)
    int SwapCreatureItem(CUser* user,
                         Inven_Item* first,
                         int second_slot,
                         Inven_Item* second,
                         int first_slot);

    // IDA_EA: 0x833FBDE
    // IDA_NAME: _ZN13user_creature12CDBMsgSender18RenameCreatureItemEiPKc
    // DEMANGLED: user_creature::CDBMsgSender::RenameCreatureItem(int,char const*)
    int RenameCreatureItem(int ui_id, const char* name);

    // IDA_EA: 0x833FC5A
    // IDA_NAME: _ZN13user_creature12CDBMsgSender15UseFeedCreatureEii
    // DEMANGLED: user_creature::CDBMsgSender::UseFeedCreature(int,int)
    int UseFeedCreature(int ui_id, int stomach);

    // IDA_EA: 0x833FCC2
    // IDA_NAME: _ZN13user_creature12CDBMsgSender13HatchCreatureEiii
    // DEMANGLED: user_creature::CDBMsgSender::HatchCreature(int,int,int)
    int HatchCreature(int ui_id, int creature_type, int item_id);

    // IDA_EA: 0x833FD74
    // IDA_NAME: _ZN13user_creature12CDBMsgSender15EvoluteCreatureEii
    // DEMANGLED: user_creature::CDBMsgSender::EvoluteCreature(int,int)
    int EvoluteCreature(int ui_id, int item_id);

    // IDA_EA: 0x833FDDC
    // IDA_NAME: _ZN13user_creature12CDBMsgSender20EventEvoluteCreatureEiii
    // DEMANGLED: user_creature::CDBMsgSender::EventEvoluteCreature(int,int,int)
    int EventEvoluteCreature(int ui_id, int item_id, int experience);

    // IDA_EA: 0x833FE8C
    // IDA_NAME: _ZN13user_creature12CDBMsgSender8SendMailEP10Inven_Itemiiii
    // DEMANGLED: user_creature::CDBMsgSender::SendMail(Inven_Item *,int,int,int,int)
    int SendMail(Inven_Item* item,
                 int charac_no,
                 int slot,
                 int creature_flag,
                 int value);

    // IDA_EA: 0x833FF34
    // IDA_NAME: _ZN13user_creature12CDBMsgSender8RecvMailEP10Inven_Itemiiii
    // DEMANGLED: user_creature::CDBMsgSender::RecvMail(Inven_Item *,int,int,int,int)
    int RecvMail(Inven_Item* item,
                 int charac_no,
                 int slot,
                 int creature_flag,
                 int value);

    // IDA_EA: 0x833FFE8
    // IDA_NAME: _ZN13user_creature12CDBMsgSender11ChangeOwnerEP10Inven_Itemiiiiii
    // DEMANGLED: user_creature::CDBMsgSender::ChangeOwner(Inven_Item *,int,int,int,int,int,int)
    int ChangeOwner(Inven_Item* item,
                    int charac_no,
                    int slot,
                    int creature_flag,
                    int value,
                    int old_charac_no,
                    int old_creature_flag);

    // IDA_EA: 0x83400A4
    // IDA_NAME: _ZN13user_creature12CDBMsgSender12ChangeSlotNoEP10Inven_Itemiii
    // DEMANGLED: user_creature::CDBMsgSender::ChangeSlotNo(Inven_Item *,int,int,int)
    int ChangeSlotNo(Inven_Item* item,
                     int charac_no,
                     int empty_slot,
                     int creature_flag);

    // IDA_EA: 0x8340144
    // IDA_NAME: _ZN13user_creature12CDBMsgSender13AuctionRegistEP10Inven_Itemiiii
    // DEMANGLED: user_creature::CDBMsgSender::AuctionRegist(Inven_Item *,int,int,int,int)
    int AuctionRegist(Inven_Item* item,
                      int charac_no,
                      int slot,
                      int creature_flag,
                      int value);

    // IDA_EA: 0x833FAAA
    // IDA_NAME: _ZN13user_creature12CDBMsgSender18InsertCreatureItemEP5CUserP10Inven_Itemiiiii
    // DEMANGLED: user_creature::CDBMsgSender::InsertCreatureItem(CUser *,Inven_Item *,int,int,int,int,int)
    int InsertCreatureItem(CUser* user,
                           Inven_Item* item,
                           int slot,
                           int creature_flag,
                           int item_count,
                           int duration_days,
                           int delete_tick);

    // IDA_EA: 0x83401EC
    // IDA_NAME: _ZN13user_creature12CDBMsgSender12UseKeepRightEii
    // DEMANGLED: user_creature::CDBMsgSender::UseKeepRight(int,int)
    int UseKeepRight(int ui_id, int no_charge);

    Stream* stream;
    CUser* user;
};

typedef char CDBMsgSender_Size_Must_Be_0x08[
    (sizeof(CDBMsgSender) == 0x08) ? 1 : -1];

}  // namespace user_creature

namespace WongWork {

// IDA unit: WongWork::Avatar_Item.  The packed 0x43-byte record follows the
// original offsets: the 25-byte agency string begins at +0x08, the 0x1e-byte
// emblem record at +0x21, and the four-byte expansion record at +0x3f.
class __attribute__((packed)) Avatar_Item {
public:
    Avatar_Item();
    ~Avatar_Item();

    // IDA_EA: 0x82FA1EA
    // IDA_NAME: _ZNK8WongWork11Avatar_Item13GetExpireDateEv
    // DEMANGLED: WongWork::Avatar_Item::GetExpireDate(void)const
    int GetExpireDate() const;

    // IDA_EA: 0x82FA1F4
    // IDA_NAME: _ZNK8WongWork11Avatar_Item13GetRemainDateEi
    // DEMANGLED: WongWork::Avatar_Item::GetRemainDate(int)const
    int GetRemainDate(int tick) const;

    // IDA_EA: 0x82FA2EE
    // IDA_NAME: _ZNK8WongWork11Avatar_Item18getJewelSocketDataEv
    // DEMANGLED: WongWork::Avatar_Item::getJewelSocketData(void)const
    const stAvatarEmblemInfo_t* getJewelSocketData() const;

    // IDA_EA: 0x82FA336
    // IDA_NAME: _ZNK8WongWork11Avatar_Item16getExpansionInfoEv
    // DEMANGLED: WongWork::Avatar_Item::getExpansionInfo(void)const
    const stAvatarExpansionInfo_t* getExpansionInfo() const;

    // IDA_EA: 0x822D202
    // IDA_NAME: _ZNK8WongWork11Avatar_Item14GetIPGAgencyNoEv
    // DEMANGLED: WongWork::Avatar_Item::GetIPGAgencyNo(void)const
    const char* GetIPGAgencyNo() const;

    // IDA_EA: 0x82FA228
    // IDA_NAME: _ZNK8WongWork11Avatar_Item13GetRegistTimeEv
    // DEMANGLED: WongWork::Avatar_Item::GetRegistTime(void)const
    int GetRegistTime() const;

    // IDA_EA: 0x82FA234
    // IDA_NAME: _ZN8WongWork11Avatar_Item7SetDataEiiPKc
    // DEMANGLED: WongWork::Avatar_Item::SetData(int,int,char const*)
    char* SetData(int expire_date, int regist_time, const char* agency_no);

    // IDA_EA: 0x82FA26A
    // IDA_NAME: _ZN8WongWork11Avatar_Item19setEmblemSocketDataERK20stAvatarEmblemInfo_t
    // DEMANGLED: WongWork::Avatar_Item::setEmblemSocketData(stAvatarEmblemInfo_t const&)
    Avatar_Item* setEmblemSocketData(const stAvatarEmblemInfo_t& emblem);

    // IDA_EA: 0x82FA2A6
    // IDA_NAME: _ZN8WongWork11Avatar_Item18updateEmblemSocketEjj
    // DEMANGLED: WongWork::Avatar_Item::updateEmblemSocket(unsigned int,unsigned int)
    int updateEmblemSocket(unsigned int socket, unsigned int item_id);

    // IDA_EA: 0x82FA2FA
    // IDA_NAME: _ZN8WongWork11Avatar_Item8setColorEss
    // DEMANGLED: WongWork::Avatar_Item::setColor(short,short)
    Avatar_Item* setColor(short first, short second);

    // IDA_EA: 0x82FA326
    // IDA_NAME: _ZN8WongWork11Avatar_Item16setExpansionInfoERK23stAvatarExpansionInfo_t
    // DEMANGLED: WongWork::Avatar_Item::setExpansionInfo(stAvatarExpansionInfo_t const&)
    Avatar_Item* setExpansionInfo(const stAvatarExpansionInfo_t& expansion);

    // IDA_EA: 0x82FA342
    // IDA_NAME: _ZNK8WongWork11Avatar_Item19getExnansionInfoRefEv
    // DEMANGLED: WongWork::Avatar_Item::getExnansionInfoRef(void)const
    const stAvatarExpansionInfo_t* getExnansionInfoRef() const;

    // IDA_EA: 0x82FA35A
    // IDA_NAME: _ZN8WongWork11Avatar_Item4initEv
    // DEMANGLED: WongWork::Avatar_Item::init(void)
    void* init();

private:
    int expire_date_;
    int regist_time_;
    char ipg_agency_no_[0x19u];
    stAvatarEmblemInfo_t jewel_socket_data_;
    stAvatarExpansionInfo_t expansion_info_;
};

static_assert(sizeof(Avatar_Item) == 0x43u,
              "Avatar_Item IDA ABI size must stay 0x43");

class CAvatarItemMgr {
public:
    static DynamicPool<Avatar_Item> m_AvatarItemPool;

    // IDA_EA: 0x822D20D
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr9IsTempKeyEi
    // DEMANGLED: WongWork::CAvatarItemMgr::IsTempKey(int)
    static unsigned int IsTempKey(int key);

    // IDA_EA: 0x822D22C
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgrC2Ev
    // DEMANGLED: WongWork::CAvatarItemMgr::CAvatarItemMgr(void)
    CAvatarItemMgr();
    // IDA_EA: 0x869937C
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgrC2ERKS0_
    // DEMANGLED: WongWork::CAvatarItemMgr::CAvatarItemMgr(WongWork::CAvatarItemMgr const&)
    CAvatarItemMgr(const CAvatarItemMgr& other);
    // IDA_EA: 0x822D2E0
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgrD2Ev
    // DEMANGLED: WongWork::CAvatarItemMgr::~CAvatarItemMgr(void)
    ~CAvatarItemMgr();
    // IDA_EA: 0x869D692
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgraSERKS0_
    // DEMANGLED: WongWork::CAvatarItemMgr::operator=(WongWork::CAvatarItemMgr const&)
    CAvatarItemMgr& operator=(const CAvatarItemMgr& other);
    // IDA_EA: 0x82F9AE4
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr5ResetEv
    // DEMANGLED: WongWork::CAvatarItemMgr::Reset(void)
    int Reset();
    // IDA_EA: 0x82F981A
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr13GetExpireDateEi
    // DEMANGLED: WongWork::CAvatarItemMgr::GetExpireDate(int)const
    int GetExpireDate(int key) const;

    // IDA_EA: 0x82F94BC
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr27RegistNotAssignedAvatarItemEiiPKcbRK20stAvatarEmblemInfo_t
    // DEMANGLED: WongWork::CAvatarItemMgr::RegistNotAssignedAvatarItem(int,int,char const*,bool,stAvatarEmblemInfo_t const&)
    int RegistNotAssignedAvatarItem(
        int usable_period,
        int regist_time,
        const char* agency_no,
        bool period_is_seconds,
        const stAvatarEmblemInfo_t& emblem);

    // IDA_EA: 0x82F8FB0 / 0x82F901C.
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr10RegistItemERK16AvatarDetailInfob /
    //           _ZN8WongWork14CAvatarItemMgr10RegistItemEiiiPKcRK20stAvatarEmblemInfo_tRK23stAvatarExpansionInfo_tb
    // DEMANGLED: WongWork::CAvatarItemMgr::RegistItem(AvatarDetailInfo const&,bool) /
    //            WongWork::CAvatarItemMgr::RegistItem(int,int,int,char const*,stAvatarEmblemInfo_t const&,stAvatarExpansionInfo_t const&,bool)
    int RegistItem(const AvatarDetailInfo& detail, bool period_in_days);
    int RegistItem(int key,
                   int expire_or_period,
                   int regist_time,
                   const char* agency_no,
                   const stAvatarEmblemInfo_t& emblem,
                   const stAvatarExpansionInfo_t& expansion,
                   bool period_in_days);

    // IDA_EA: 0x82F913E
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr18updateEmblemSocketEijPcPm
    // DEMANGLED: WongWork::CAvatarItemMgr::updateEmblemSocket(int,unsigned int,char *,unsigned long *)
    int updateEmblemSocket(int key,
                           unsigned int socket_count,
                           char* socket_types,
                           unsigned long* emblem_item_ids);

    // IDA_EA: 0x82F95F4
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr27RegistNotAssignedAvatarItemE10Inven_Item
    // DEMANGLED: WongWork::CAvatarItemMgr::RegistNotAssignedAvatarItem(Inven_Item)
    int RegistNotAssignedAvatarItem(Inven_Item item);

    // IDA_EA: 0x82F96A6
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr20AssignAvatarItemInfoEiiiiPKc
    // DEMANGLED: WongWork::CAvatarItemMgr::AssignAvatarItemInfo(int,int,int,int,char const*)
    int AssignAvatarItemInfo(int temporary_key,
                             int avatar_ui_id,
                             int expire_date,
                             int regist_time,
                             const char* agency_no);

    // IDA_EA: 0x82F9772.
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr19AssignAvatarItemKeyEii
    // DEMANGLED: WongWork::CAvatarItemMgr::AssignAvatarItemKey(int,int)
    int AssignAvatarItemKey(int temporary_key, int avatar_ui_id);

    // IDA_EA: 0x82F988C
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr13GetRegistTimeEi
    // DEMANGLED: WongWork::CAvatarItemMgr::GetRegistTime(int)const
    int GetRegistTime(int key) const;

    // IDA_EA: 0x82F98C2
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr14GetIPGAgencyNoEi
    // DEMANGLED: WongWork::CAvatarItemMgr::GetIPGAgencyNo(int)const
    const char* GetIPGAgencyNo(int key) const;

    // IDA_EA: 0x82F9E02
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr14_AssignTempKeyEv
    // DEMANGLED: WongWork::CAvatarItemMgr::_AssignTempKey(void)
    int _AssignTempKey();

    // IDA_EA: 0x82F9402
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr12UnRegistItemEi
    // DEMANGLED: WongWork::CAvatarItemMgr::UnRegistItem(int)
    int UnRegistItem(int key);

    // IDA_EA: 0x82F9F9E
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr19GetEmblemReduceRateEi
    // DEMANGLED: WongWork::CAvatarItemMgr::GetEmblemReduceRate(int)
    int GetEmblemReduceRate(int key);

    // IDA_EA: 0x82FA008
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr15UpdateEnduranceEii
    // DEMANGLED: WongWork::CAvatarItemMgr::UpdateEndurance(int,int)
    void UpdateEndurance(int key, int endurance);

    // IDA_EA: 0x82FA0A8
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr13SaveEnduranceEv
    // DEMANGLED: WongWork::CAvatarItemMgr::SaveEndurance(void)
    int SaveEndurance();

    // IDA_EA: 0x82FA3B2
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr12_MakeTempKeyEi
    // DEMANGLED: WongWork::CAvatarItemMgr::_MakeTempKey(int)const
    unsigned int _MakeTempKey(int key) const;

    // IDA_EA: 0x82F9850
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr13GetRemainDateEii
    // DEMANGLED: WongWork::CAvatarItemMgr::GetRemainDate(int,int)const
    int GetRemainDate(int key, int tick) const;

    // IDA_EA: 0x82F98F8
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr18getJewelSocketDataEi
    // DEMANGLED: WongWork::CAvatarItemMgr::getJewelSocketData(int)const
    const stAvatarEmblemInfo_t* getJewelSocketData(int key) const;

    // IDA_EA: 0x82F91CE
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr19setEmblemSocketDataEiRK20stAvatarEmblemInfo_t
    // DEMANGLED: WongWork::CAvatarItemMgr::setEmblemSocketData(int,stAvatarEmblemInfo_t const&)
    int setEmblemSocketData(int key, const stAvatarEmblemInfo_t& emblem);

    // IDA_EA: 0x82F9228
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr14isSocketAvatarEi
    // DEMANGLED: WongWork::CAvatarItemMgr::isSocketAvatar(int)
    int isSocketAvatar(int key);

    // IDA_EA: 0x82F992E
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr16GetExpansionInfoEi
    // DEMANGLED: WongWork::CAvatarItemMgr::GetExpansionInfo(int)const
    const stAvatarExpansionInfo_t* GetExpansionInfo(int key) const;

    // IDA_EA: 0x82F9964
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr14SetAvatarColorEiss
    // DEMANGLED: WongWork::CAvatarItemMgr::SetAvatarColor(int,short,short)
    int SetAvatarColor(int key, short first_color, short second_color);

    // IDA_EA: 0x82F99BC
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr13GetAvatarInfoEi
    // DEMANGLED: WongWork::CAvatarItemMgr::GetAvatarInfo(int)const
    Avatar_Item* GetAvatarInfo(int key) const;

    // IDA_EA: 0x82F925C
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr7GetItemEP16AvatarDetailInfoPi
    // DEMANGLED: WongWork::CAvatarItemMgr::GetItem(AvatarDetailInfo *,int *)const
    int GetItem(AvatarDetailInfo* details, int* count) const;

    // IDA_EA: 0x82F9CAC
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr12_FindItemAllEi
    // DEMANGLED: WongWork::CAvatarItemMgr::_FindItemAll(int)const
    Avatar_Item* _FindItemAll(int key) const;

    // IDA_EA: 0x82F9D34
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr9_FindItemEi
    // DEMANGLED: WongWork::CAvatarItemMgr::_FindItem(int)const
    Avatar_Item* _FindItem(int key) const;

    // IDA_EA: 0x82F9D98
    // IDA_NAME: _ZNK8WongWork14CAvatarItemMgr19_FindWaitAssignItemEi
    // DEMANGLED: WongWork::CAvatarItemMgr::_FindWaitAssignItem(int)const
    Avatar_Item* _FindWaitAssignItem(int key) const;

    // IDA_EA: 0x82F9A42
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr15EraseAvatarItemEi
    // DEMANGLED: WongWork::CAvatarItemMgr::EraseAvatarItem(int)
    Avatar_Item* EraseAvatarItem(int key);

    // IDA_EA: 0x82F99D6
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr13AddAvatarItemEiPNS_11Avatar_ItemE
    // DEMANGLED: WongWork::CAvatarItemMgr::AddAvatarItem(int,WongWork::Avatar_Item *)
    int AddAvatarItem(int key, Avatar_Item* item);

    // IDA_EA: 0x82F9E4C
    // IDA_NAME: _ZN8WongWork14CAvatarItemMgr18AddEmblemEnduranceEiPKNS_11Avatar_ItemE
    // DEMANGLED: WongWork::CAvatarItemMgr::AddEmblemEndurance(int,WongWork::Avatar_Item const*)
    int AddEmblemEndurance(int key, const Avatar_Item* item);

private:
    // C2 @0x822D22C constructs these maps at +0x00, +0x18, +0x54 and
    // +0x6c.  The two scalar fields between them are reset by Reset()
    // @0x82F9AE4; they are not anonymous storage.
    std::map<int, Avatar_Item*> assigned_items_;      // +0x00
    std::map<int, Avatar_Item*> wait_assign_items_;   // +0x18
    int next_temp_key_;                               // +0x30
    stAvatarEmblemInfo_t default_emblem_;             // +0x34
    uint16_t default_emblem_alignment_;               // +0x52
    std::map<int, int> emblem_reduce_rates_;          // +0x54
    std::map<int, int> pending_endurance_;            // +0x6c
};

#if defined(__i386__)
static_assert(sizeof(CAvatarItemMgr) == 0x84u,
              "CAvatarItemMgr IDA ABI size must stay 0x84");
#endif

}  // namespace WongWork

#endif  // OLDSRV_OLD_GAME_CREATURE_AVATAR_CREATURE_H_
