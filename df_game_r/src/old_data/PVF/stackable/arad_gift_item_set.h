#ifndef OLDSRV_OLD_DATA_PVF_STACKABLE_ARAD_GIFT_ITEM_SET_H_
#define OLDSRV_OLD_DATA_PVF_STACKABLE_ARAD_GIFT_ITEM_SET_H_

#include <stdint.h>

#include <map>
#include <string>

#include "common/manager/global_instance.h"
#include "old_data/PVF/common/arad_script_loader.h"

struct STStackableScript;
class CStackableItem;
class CUser;

// IDA unit: Arad_GiftItem_Set.  This is populated while stackable PVF items
// are imported; runtime mail handling consumes the same key/value map.
class Arad_GiftItem_Set : public Arad_ScriptLoader {
public:
    enum ST_ITEM_KIND {
        ST_EVENT_GIFT_ITEM = 1,
        ST_EVENT_GIFT_ITEM2 = 2,
        ST_EVENT_GIFT_ITEM3 = 3,
        ST_EVENT_GIFT_ITEM4 = 4,
        ST_EVENT_GIFT_ITEM5 = 5,
        ST_EVENT_GIFT_ITEM6 = 6,
    };

    struct Data {
        Data();

        int32_t item_index;
        std::string title;
        std::string content;
    };

    Arad_GiftItem_Set();
    ~Arad_GiftItem_Set();

    int str_to_enum(const char* tag) const;
    int open(STStackableScript* script);
    const Data* find(int kind, int charac_job, int usable_level) const;

    // IDA_EA: 0x81A329E
    // IDA_NAME: _ZN17Arad_GiftItem_Set9send_mailEP5CUseriiii
    // DEMANGLED: Arad_GiftItem_Set::send_mail(CUser *,int,int,int,int)
    int send_mail(CUser* user,
                  int receiver_charac_no,
                  int usable_level,
                  int charac_job,
                  int item_kind);

    // IDA_EA: 0x81A31C4
    // IDA_NAME: _ZN17Arad_GiftItem_Set11level_checkEP5CUserPK14CStackableItem
    // DEMANGLED: Arad_GiftItem_Set::level_check(CUser *,CStackableItem const*)
    int level_check(CUser* user, const CStackableItem* item);

private:
    std::map<long long, Data> data_;
    std::map<std::string, int> kinds_;
};

static_assert(sizeof(Arad_GiftItem_Set::Data) == 0x0cu,
              "Arad_GiftItem_Set::Data IDA ABI size must stay 0x0c");
static_assert(sizeof(Arad_GiftItem_Set) == 0x34u,
              "Arad_GiftItem_Set IDA ABI size must stay 0x34");

template <>
Arad_GiftItem_Set* GlobalInstance<Arad_GiftItem_Set>::create();
template <>
Arad_GiftItem_Set* GlobalInstance<Arad_GiftItem_Set>::inst();

#endif  // OLDSRV_OLD_DATA_PVF_STACKABLE_ARAD_GIFT_ITEM_SET_H_
