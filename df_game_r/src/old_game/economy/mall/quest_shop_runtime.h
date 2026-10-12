#ifndef OLDSRV_OLD_GAME_NPC_QUEST_SHOP_RUNTIME_H_
#define OLDSRV_OLD_GAME_NPC_QUEST_SHOP_RUNTIME_H_

#include <stddef.h>
#include <map>
#include <stdint.h>

#include "old_game/player/core/charac_expand_cdata.h"
#include "old_game/player/core/character_tag_data.h"
#include "infra/network/packet/message_base.h"


class CUser;
class PacketGuard;

// IDA unit: 07_npc_shop_dialog/MSG_QUEST_SHOP.
struct MSG_QUEST_SHOP : public MSG_BASE {
    MSG_QUEST_SHOP();

    uint8_t status_id;  // +0x0d
    uint8_t count;      // +0x0e
};

static_assert(sizeof(MSG_QUEST_SHOP) == 0x0fu &&
                  offsetof(MSG_QUEST_SHOP, status_id) == 0x0du &&
                  offsetof(MSG_QUEST_SHOP, count) == 0x0eu,
              "quest-shop buy request ABI drift");

// IDA unit: 07_npc_shop_dialog/SIG_QUEST_SHOP_DATA.
struct SIG_QUEST_SHOP_DATA {
    SIG_QUEST_SHOP_DATA();

    CHARAC_QUEST_SHOP_DATA current;
    CHARAC_QUEST_SHOP_DATA tagged;
    uint8_t has_tagged;
};

// IDA unit: 07_npc_shop_dialog/CQuestShop.
class CQuestShop : public charac_expand::CData {
public:
    CQuestShop();
    virtual ~CQuestShop();

    int clearBuyingStatuslist();
    int subQp(uint8_t status_id, uint8_t count);
    int findStatusQp(uint8_t status_id);
    int findStatusType(uint8_t status_id);
    int incStatusBuyingCount(uint8_t status_id, uint8_t count);
    int getStatusBuyingCount(uint8_t status_id);
    bool checkBuyingStatus_LimitLevel(uint8_t status_id,
                                      short buying_count,
                                      int charac_level);
    int getInitCost(int init_count);
    int calUsedQp();
    int buyStatus(CUser* user,
                  uint8_t status_id,
                  uint8_t count,
                  short& buying_count);
    int clearUsedQp(CUser* user);
    int clearQP(CUser* user);
    bool makePacket_BuyingStatusList(PacketGuard& packet,
                                     bool tagged_status_list);
    void sendCharacQp(CUser* user, char reason);
    void sendCharacQuestPiece(CUser* user);
    virtual void getData(char* data) const;
    virtual bool loadData(CUser* user, char* data);
    int SetTagCharacQuestShopData(CHARAC_QUEST_SHOP_DATA* data);
    int getQp() const;
    CQuestShop* subQp(int quest_point);
    CQuestShop* setQp(int quest_point);
    CQuestShop* setInitCount(int init_count);
    int getInitCount() const;
    CQuestShop* incInitCount();
    int getInitQp() const;
    int getPiece() const;
    CQuestShop* addQp(int quest_point);
    CQuestShop* incInitQp(int quest_point);
    CQuestShop* setInitQp(int quest_point);
    CQuestShop* addPiece(int piece);
    CQuestShop* setPiece(int piece);

protected:
    virtual int _saveData(CUser* user);
    virtual int _reset();

private:
    typedef std::map<uint8_t, short> StatusMap;

    StatusMap buying_status_list_;
    StatusMap tagged_buying_status_list_;
    int quest_point_;
    int init_count_;
    int init_quest_point_;
    int quest_piece_;
};

#endif  // OLDSRV_OLD_GAME_NPC_QUEST_SHOP_RUNTIME_H_
