#include "old_data/PVF/itemshop/secret_shop_script.h"

#include "old_data/PVF/common/rdar_script.h"
#include "common/manager/mt_rand.h"

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <utility>


namespace secretshop {

// IDA_EA: 0x8A832AA
// IDA_NAME: _ZN10secretshop9SALE_INFOC2Ev
// DEMANGLED: secretshop::SALE_INFO::SALE_INFO(void)
SALE_INFO::SALE_INFO()
    : item_index(0),
      buy_rule(0),
      price(0),
      value_0c(0),
      value_10(0),
      value_14(0),
      event_item(0) {}

// IDA_EA: 0x8A85BCA
// IDA_NAME: _ZN10secretshop9SALE_INFOD2Ev
// DEMANGLED: secretshop::SALE_INFO::~SALE_INFO()
SALE_INFO::~SALE_INFO() {}

// IDA_EA: 0x8A832EE
// IDA_NAME: _ZN10secretshop9SALE_INFOeqEi
// DEMANGLED: secretshop::SALE_INFO::operator==(int)
bool SALE_INFO::operator==(int item_index_value) {
    return item_index == item_index_value;
}

}  // namespace secretshop

// IDA_EA: 0x8A832FE
// IDA_NAME: _ZNK14stLevelSectionltERKS_
// DEMANGLED: stLevelSection::operator<(stLevelSection const&)const
bool stLevelSection::operator<(const stLevelSection& other) const {
    return section_index < other.section_index;
}

// IDA_EA: 0x83720C8
// IDA_NAME: _ZN14stDungeonSalesC2Ev
// DEMANGLED: stDungeonSales::stDungeonSales(void)
stDungeonSales::stDungeonSales() : sales() {}

// IDA_EA: 0x83720DE
// IDA_NAME: _ZN14stDungeonSalesD2Ev
// DEMANGLED: stDungeonSales::~stDungeonSales()
stDungeonSales::~stDungeonSales() {}

// IDA_EA: 0x8A8354E
// IDA_NAME: _ZN14stDungeonSalesC2ERKS_
// DEMANGLED: stDungeonSales::stDungeonSales(stDungeonSales const&)
stDungeonSales::stDungeonSales(const stDungeonSales& other)
    : selector_index(other.selector_index),
      sale_count(other.sale_count),
      sales(other.sales) {}

// IDA_EA: 0x8A83312
// IDA_NAME: _ZN10stNpcSalesC2Ev
// DEMANGLED: stNpcSales::stNpcSales(void)
stNpcSales::stNpcSales() : dungeon_sales(), level_sales() {}

// IDA_EA: 0x83CE6D6
// IDA_NAME: _ZN10stNpcSalesD2Ev
// DEMANGLED: stNpcSales::~stNpcSales()
stNpcSales::~stNpcSales() {}

// IDA_EA: 0x8A83622
// IDA_NAME: _ZN10stNpcSalesC2ERKS_
// DEMANGLED: stNpcSales::stNpcSales(stNpcSales const&)
stNpcSales::stNpcSales(const stNpcSales& other)
    : npc_index(other.npc_index),
      dungeon_sales(other.dungeon_sales),
      level_sales(other.level_sales) {}

// IDA_EA: 0x8372020
// IDA_NAME: _ZN16SecretShopScript5clearEv
// DEMANGLED: SecretShopScript::clear(void)
void SecretShopScript::clear() {
    price_random_percent = 0;
    eplp_wait_time = 0;
    dungeon_npcs.clear();
    level_sections.clear();
    level_npcs.clear();
    npc_sales.clear();
}

// IDA_EA: 0x83720F4
// IDA_NAME: _ZN16SecretShopScriptC2Ev
// DEMANGLED: SecretShopScript::SecretShopScript(void)
SecretShopScript::SecretShopScript()
    : dungeon_npcs(),
      level_npcs(),
      level_sections(),
      npc_sales(),
      event_sales() {
    clear();
}

// IDA_EA: 0x837D756
// IDA_NAME: _ZN16SecretShopScriptD2Ev
// DEMANGLED: SecretShopScript::~SecretShopScript()
SecretShopScript::~SecretShopScript() {}

// IDA_EA: 0x8A82804
// IDA_NAME: _ZN16SecretShopScript9verifyNpcERSt8multimapIi12stDungeonNpcSt4lessIiESaISt4pairIKiS1_EEE
// DEMANGLED: SecretShopScript::verifyNpc(std::multimap<int,stDungeonNpc> &)
void SecretShopScript::verifyNpc(NpcMultiMap& npcs) {
    for (NpcMultiMap::iterator it = npcs.begin(); it != npcs.end(); ++it) {
        NpcRange range = npcs.equal_range(it->first);
        int total = 0;
        for (NpcMultiMap::iterator scan = range.first; scan != range.second;
             ++scan) {
            total += scan->second.ratio;
        }
        if (total != 10000) {
            std::printf("[SecretShop Error] NPC ratio is wrong!(index:%d)\n",
                        range.first->second.selector_index);
        }
    }
}

// IDA_EA: 0x8A828F8
// IDA_NAME: _ZN16SecretShopScript18GetNpcByDungeonIdxERiii
// DEMANGLED: SecretShopScript::GetNpcByDungeonIdx(int &,int,int)
int SecretShopScript::GetNpcByDungeonIdx(int& npc_index,
                                         int random_value,
                                         int dungeon_index) {
    NpcRange range = dungeon_npcs.equal_range(dungeon_index);
    return GetNpcIdx(npc_index, random_value, range);
}

// IDA_EA: 0x8A8293C
// IDA_NAME: _ZN16SecretShopScript18GetNpcByDungeonLevERiii
// DEMANGLED: SecretShopScript::GetNpcByDungeonLev(int &,int,int)
int SecretShopScript::GetNpcByDungeonLev(int& npc_index,
                                         int random_value,
                                         int standard_level) {
    const int level_index = GetLevelIdx(standard_level);
    NpcRange range = level_npcs.equal_range(level_index);
    return GetNpcIdx(npc_index, random_value, range);
}

// IDA_EA: 0x8A82998
// IDA_NAME: _ZN16SecretShopScript9GetNpcIdxERiiRSt4pairISt17_Rb_tree_iteratorIS1_IKi12stDungeonNpcEES6_E
// DEMANGLED: SecretShopScript::GetNpcIdx(int &,int,std::pair<iterator,iterator> &)
int SecretShopScript::GetNpcIdx(int& npc_index,
                                int random_value,
                                NpcRange& range) {
    int total = 0;
    for (NpcMultiMap::iterator it = range.first; it != range.second; ++it) {
        total += it->second.ratio;
        if (total > random_value) {
            npc_index = it->second.npc_index;
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8A82A0E
// IDA_NAME: _ZN16SecretShopScript19GetItemByDungeonIdxEP7CMTRandRSt6vectorIN10secretshop9SALE_INFOESaIS4_EEiib
// DEMANGLED: SecretShopScript::GetItemByDungeonIdx(CMTRand *,std::vector<secretshop::SALE_INFO> &,int,int,bool)
int SecretShopScript::GetItemByDungeonIdx(
    CMTRand* random,
    std::vector<secretshop::SALE_INFO>& items,
    int npc_index,
    int dungeon_index,
    bool include_event) {
    std::map<int, stNpcSales>::iterator npc_it = npc_sales.find(npc_index);
    if (npc_it == npc_sales.end()) {
        return 0;
    }

    std::map<int, stDungeonSales>::iterator sales_it =
        npc_it->second.dungeon_sales.find(dungeon_index);
    if (sales_it == npc_it->second.dungeon_sales.end()) {
        return 0;
    }

    if (GetItems(random, items, sales_it->second) != 1) {
        return 0;
    }
    if (include_event) {
        GetItemByEvent(random, items);
    }
    return 1;
}

// IDA_EA: 0x8A82B34
// IDA_NAME: _ZN16SecretShopScript19GetItemByDungeonLevEP7CMTRandRSt6vectorIN10secretshop9SALE_INFOESaIS4_EEiib
// DEMANGLED: SecretShopScript::GetItemByDungeonLev(CMTRand *,std::vector<secretshop::SALE_INFO> &,int,int,bool)
int SecretShopScript::GetItemByDungeonLev(
    CMTRand* random,
    std::vector<secretshop::SALE_INFO>& items,
    int npc_index,
    int standard_level,
    bool include_event) {
    std::map<int, stNpcSales>::iterator npc_it = npc_sales.find(npc_index);
    if (npc_it == npc_sales.end()) {
        return 0;
    }

    const int level_index = GetLevelIdx(standard_level);
    std::map<int, stDungeonSales>::iterator sales_it =
        npc_it->second.level_sales.find(level_index);
    if (sales_it == npc_it->second.level_sales.end()) {
        return 0;
    }

    if (GetItems(random, items, sales_it->second) != 1) {
        return 0;
    }
    if (include_event) {
        GetItemByEvent(random, items);
    }
    return 1;
}

// IDA_EA: 0x8A82C70
// IDA_NAME: _ZN16SecretShopScript14GetItemByEventEP7CMTRandRSt6vectorIN10secretshop9SALE_INFOESaIS4_EE
// DEMANGLED: SecretShopScript::GetItemByEvent(CMTRand *,std::vector<secretshop::SALE_INFO> &)
int SecretShopScript::GetItemByEvent(
    CMTRand* random,
    std::vector<secretshop::SALE_INFO>& items) {
    std::vector<secretshop::SALE_INFO> event_items;
    if (GetItems(random, event_items, event_sales) == 1) {
        for (int i = 0; i < static_cast<int>(event_items.size()); ++i) {
            event_items[i].event_item = 1;
            items.push_back(event_items[i]);
        }
        return 1;
    }
    return 0;
}

// IDA_EA: 0x8A82D44
// IDA_NAME: _ZN16SecretShopScript8GetItemsEP7CMTRandRSt6vectorIN10secretshop9SALE_INFOESaIS4_EER14stDungeonSales
// DEMANGLED: SecretShopScript::GetItems(CMTRand *,std::vector<secretshop::SALE_INFO> &,stDungeonSales &)
int SecretShopScript::GetItems(CMTRand* random,
                               std::vector<secretshop::SALE_INFO>& items,
                               stDungeonSales& sales) {
    if (sales.sales.size() >= static_cast<size_t>(sales.sale_count)) {
        int attempts = 0;
        secretshop::SALE_INFO copied;
        do {
            if (items.size() >= static_cast<size_t>(sales.sale_count)) {
                break;
            }

            unsigned int domain_rate =
                static_cast<unsigned int>(GetDomainRate(sales));
            unsigned int random_value = random->randInt(domain_rate);
            unsigned int total = 0;
            for (std::map<int, stSaleInfo>::iterator it = sales.sales.begin();
                 it != sales.sales.end();
                 ++it) {
                total += static_cast<unsigned int>(it->second.ratio);
                if (total > random_value) {
                    if (!IsExistItem(items, it->second.item_index)) {
                        CopyItem(copied, it->second);
                        items.push_back(copied);
                    }
                    break;
                }
            }
            ++attempts;
        } while (attempts <= 10000);
        return 1;
    }

    secretshop::SALE_INFO copied;
    for (std::map<int, stSaleInfo>::iterator it = sales.sales.begin();
         it != sales.sales.end();
         ++it) {
        CopyItem(copied, it->second);
        items.push_back(copied);
    }
    return 1;
}

// IDA_EA: 0x8A82F6A
// IDA_NAME: _ZN16SecretShopScript11IsExistItemERSt6vectorIN10secretshop9SALE_INFOESaIS2_EEi
// DEMANGLED: SecretShopScript::IsExistItem(std::vector<secretshop::SALE_INFO> &,int)
int SecretShopScript::IsExistItem(std::vector<secretshop::SALE_INFO>& items,
                                  int item_index) {
    return std::find(items.begin(), items.end(), item_index) != items.end();
}

// IDA_EA: 0x8A82FF6
// IDA_NAME: _ZN16SecretShopScript13GetDomainRateER14stDungeonSales
// DEMANGLED: SecretShopScript::GetDomainRate(stDungeonSales &)
int SecretShopScript::GetDomainRate(stDungeonSales& sales) {
    int total = 0;
    for (std::map<int, stSaleInfo>::iterator it = sales.sales.begin();
         it != sales.sales.end();
         ++it) {
        total += it->second.ratio;
    }
    return total;
}

// IDA_EA: 0x8A8306C
// IDA_NAME: _ZN16SecretShopScript8CopyItemERN10secretshop9SALE_INFOER10stSaleInfo
// DEMANGLED: SecretShopScript::CopyItem(secretshop::SALE_INFO &,stSaleInfo &)
void SecretShopScript::CopyItem(secretshop::SALE_INFO& dest,
                                stSaleInfo& src) {
    dest.item_index = src.item_index;
    dest.buy_rule = src.buy_rule;
    dest.value_0c = src.value_10;
    if (dest.buy_rule != 0) {
        if (dest.buy_rule == 1) {
            dest.value_10 = src.price;
            dest.value_14 = src.value_0c;
        }
        return;
    }

    dest.price = GetRandItemPrice(src.price);
}

// IDA_EA: 0x8A830E2
// IDA_NAME: _ZN16SecretShopScript11GetLevelIdxEi
// DEMANGLED: SecretShopScript::GetLevelIdx(int)
int SecretShopScript::GetLevelIdx(int standard_level) {
    for (std::set<stLevelSection>::const_iterator it = level_sections.begin();
         it != level_sections.end();
         ++it) {
        if (it->min_level <= standard_level &&
            it->max_level >= standard_level) {
            return it->section_index;
        }
    }
    return 0;
}

// IDA_EA: 0x8A83188
// IDA_NAME: _ZN16SecretShopScript16GetRandItemPriceEi
// DEMANGLED: SecretShopScript::GetRandItemPrice(int)
int SecretShopScript::GetRandItemPrice(int price) {
    const int random_value = std::rand() % 3;
    if (random_value == 0) {
        return price +
               static_cast<int>(static_cast<long double>(price) *
                                (static_cast<long double>(
                                     price_random_percent) /
                                 100.0L));
    }
    if (random_value == 1) {
        return price -
               static_cast<int>(static_cast<long double>(price) *
                                (static_cast<long double>(
                                     price_random_percent) /
                                 100.0L));
    }
    return price;
}

// IDA_EA: 0x8A81CF0
// IDA_NAME: _Z22importSecretShopScriptP16SecretShopScriptPKc
// DEMANGLED: importSecretShopScript(SecretShopScript *,char const*)
int importSecretShopScript(SecretShopScript* script, const char* filename) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    SecretShopScript* target = script;
    target->clear();

    std::string tag;
    bool success = false;
    while (ScanType(tag, true) == 1) {
        if (tag == "[dungeon npc]") {
            while (1) {
                stDungeonNpc npc;
                npc.selector_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                npc.npc_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                npc.ratio = ScanInt(&success);
                if (!success) {
                    break;
                }
                for (int i = 0; i <= 3; ++i) {
                    ScanInt(static_cast<bool*>(0));
                }
                target->dungeon_npcs.insert(
                    std::make_pair(npc.selector_index, npc));
            }
            target->verifyNpc(target->dungeon_npcs);
        } else if (tag == "[level section]") {
            while (1) {
                stLevelSection section;
                section.section_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                section.min_level = ScanInt(static_cast<bool*>(0));
                section.max_level = ScanInt(static_cast<bool*>(0));
                target->level_sections.insert(section);
            }
        } else if (tag == "[level npc]") {
            while (1) {
                stDungeonNpc npc;
                npc.selector_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                npc.npc_index = ScanInt(static_cast<bool*>(0));
                npc.ratio = ScanInt(static_cast<bool*>(0));
                for (int i = 0; i <= 3; ++i) {
                    ScanInt(static_cast<bool*>(0));
                }
                target->level_npcs.insert(
                    std::make_pair(npc.selector_index, npc));
            }
            target->verifyNpc(target->level_npcs);
        } else if (tag == "[npc]") {
            stNpcSales npc_sales;
            npc_sales.npc_index = ScanInt(static_cast<bool*>(0));
            do {
                while (1) {
                    while (1) {
                        const int scan_result = ScanType(tag, true);
                        const DNFLEX_TOKEN token = g_dnfLex.getLastToken();
                        if (scan_result != 1 && token != DNF_TOK_TYPE) {
                            std::fprintf(
                                stderr,
                                "[secret-shop-parser] missing [/npc] "
                                "file=%s line=%d token=%s\n",
                                filename ? filename : "(null)",
                                g_dnfLex.getLastLineNo(),
                                g_dnfLex.getLastTokenDataAsString().c_str());
                            return 0;
                        }
                        if (tag != "[dungeon]") {
                            break;
                        }

                        stDungeonSales dungeon_sales;
                        dungeon_sales.selector_index =
                            ScanInt(static_cast<bool*>(0));
                        dungeon_sales.sale_count =
                            ScanInt(static_cast<bool*>(0));
                        while (1) {
                            stSaleInfo sale;
                            sale.item_index = ScanInt(&success);
                            if (!success) {
                                break;
                            }
                            sale.buy_rule = static_cast<uint8_t>(
                                ScanInt(static_cast<bool*>(0)));
                            sale.price = ScanInt(static_cast<bool*>(0));
                            sale.value_0c = ScanInt(static_cast<bool*>(0));
                            sale.value_10 = ScanInt(static_cast<bool*>(0));
                            sale.ratio = ScanInt(static_cast<bool*>(0));
                            dungeon_sales.sales.insert(
                                std::make_pair(sale.item_index, sale));
                        }
                        npc_sales.dungeon_sales.insert(
                            std::make_pair(dungeon_sales.selector_index,
                                           dungeon_sales));
                    }

                    if (tag != "[level]") {
                        break;
                    }

                    stDungeonSales level_sales;
                    level_sales.selector_index =
                        ScanInt(static_cast<bool*>(0));
                    level_sales.sale_count = ScanInt(static_cast<bool*>(0));
                    while (1) {
                        stSaleInfo sale;
                        sale.item_index = ScanInt(&success);
                        if (!success) {
                            break;
                        }
                        sale.buy_rule = static_cast<uint8_t>(
                            ScanInt(static_cast<bool*>(0)));
                        sale.price = ScanInt(static_cast<bool*>(0));
                        sale.value_0c = ScanInt(static_cast<bool*>(0));
                        sale.value_10 = ScanInt(static_cast<bool*>(0));
                        sale.ratio = ScanInt(static_cast<bool*>(0));
                        level_sales.sales.insert(
                            std::make_pair(sale.item_index, sale));
                    }
                    npc_sales.level_sales.insert(
                        std::make_pair(level_sales.selector_index,
                                       level_sales));
                }
            } while (tag != "[/npc]");

            target->npc_sales.insert(
                std::make_pair(npc_sales.npc_index, npc_sales));
        } else if (tag == "[event item]") {
            target->event_sales.sale_count = ScanInt(static_cast<bool*>(0));
            while (1) {
                stSaleInfo sale;
                sale.item_index = ScanInt(&success);
                if (!success) {
                    break;
                }
                sale.buy_rule = static_cast<uint8_t>(
                    ScanInt(static_cast<bool*>(0)));
                sale.price = ScanInt(static_cast<bool*>(0));
                sale.value_0c = ScanInt(static_cast<bool*>(0));
                sale.value_10 = ScanInt(static_cast<bool*>(0));
                sale.ratio = ScanInt(static_cast<bool*>(0));
                target->event_sales.sales.insert(
                    std::make_pair(sale.item_index, sale));
            }
        } else if (tag == "[price random]") {
            target->price_random_percent = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[eplp wait time]") {
            target->eplp_wait_time = ScanInt(static_cast<bool*>(0));
        } else if (tag != "[add slot cash user stackable count]" &&
                   tag == "[secret shop end]") {
            success = true;
            break;
        }
    }

    return success ? 1 : 0;
}
