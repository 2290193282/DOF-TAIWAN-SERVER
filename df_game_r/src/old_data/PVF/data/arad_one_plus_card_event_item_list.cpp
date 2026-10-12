#include "old_data/PVF/data/arad_one_plus_card_event_item_list.h"

#include "old_data/PVF/common/arad_script.h"
#include "old_data/PVF/common/dnf_lex.h"
#include "infra/log/format/trace.h"

#include <algorithm>

namespace {

char* Mutable(const char* text) {
    return const_cast<char*>(text);
}

}  // namespace

// IDA_EA: 0x81A41E8
// IDA_NAME: _ZN30Arad_OnePlusCardEvent_ItemList4DataC2Etm
// DEMANGLED: Arad_OnePlusCardEvent_ItemList::Data::Data(unsigned short,unsigned long)
Arad_OnePlusCardEvent_ItemList::Data::Data(
    uint16_t web_id_value,
    unsigned long item_index_value)
    : web_id(web_id_value), item_index(item_index_value) {}

// IDA_EA: 0x81A420A
// IDA_NAME: _ZN30Arad_OnePlusCardEvent_ItemList4DataeqERKS0_
// DEMANGLED: Arad_OnePlusCardEvent_ItemList::Data::operator==(Arad_OnePlusCardEvent_ItemList::Data const&)
bool Arad_OnePlusCardEvent_ItemList::Data::operator==(
    const Data& rhs) {
    return web_id == rhs.web_id;
}

// IDA_EA: 0x81A80F0
// IDA_NAME: _ZN30Arad_OnePlusCardEvent_ItemList4DataD2Ev
// DEMANGLED: Arad_OnePlusCardEvent_ItemList::Data::~Data()
Arad_OnePlusCardEvent_ItemList::Data::~Data() {}

// IDA_EA: 0x8188D0A
// IDA_NAME: _ZN30Arad_OnePlusCardEvent_ItemListC2Ev
// DEMANGLED: Arad_OnePlusCardEvent_ItemList::Arad_OnePlusCardEvent_ItemList(void)
Arad_OnePlusCardEvent_ItemList::Arad_OnePlusCardEvent_ItemList()
    : Arad_ScriptLoader(), item_list_(), limited_maps_() {}

// IDA_EA: 0x8188D6A
// IDA_NAME: _ZN30Arad_OnePlusCardEvent_ItemListD2Ev
// DEMANGLED: Arad_OnePlusCardEvent_ItemList::~Arad_OnePlusCardEvent_ItemList()
Arad_OnePlusCardEvent_ItemList::~Arad_OnePlusCardEvent_ItemList() {}

// IDA_EA: 0x81A36FC
// IDA_NAME: _ZN30Arad_OnePlusCardEvent_ItemList7parsingEPc
// DEMANGLED: Arad_OnePlusCardEvent_ItemList::parsing(char *)
int Arad_OnePlusCardEvent_ItemList::parsing(char* buffer) {
    Arad_Script script;
    if (script.Load(buffer, "item") != 1) {
        return 475;
    }

    for (int row = 0; row < script.GetRowNum(); ++row) {
        const uint16_t web_id = static_cast<uint16_t>(
            script.GetInt(row, Mutable("web_id")));
        const unsigned long item_index = static_cast<unsigned long>(
            script.GetInt(row, Mutable("item_index")));
        if (find_item(web_id) != 0) {
            LogManager::logFormat(
                1, "localjapan/Arad_Script_Loader.cpp",
                "virtual int Arad_OnePlusCardEvent_ItemList::parsing(char*)",
                490,
                "[ERROR] already exist data id OnePlusCardEvent(id:%d, item:%u)",
                web_id, item_index);
            return 491;
        }
        item_list_.push_back(Data(web_id, item_index));
    }

    if (script.Load(g_dnfLex.getInputStreamBuffer(), "limited_map") != 1) {
        return 499;
    }
    for (int row = 0; row < script.GetRowNum(); ++row) {
        const unsigned int map_index = static_cast<unsigned int>(
            script.GetInt(row, Mutable("index")));
        if (is_limited_map(map_index)) {
            LogManager::logFormat(
                1, "localjapan/Arad_Script_Loader.cpp",
                "virtual int Arad_OnePlusCardEvent_ItemList::parsing(char*)",
                510, "[ERROR] already exist limited map index (map:%u)",
                map_index);
            return 511;
        }
        limited_maps_.push_back(map_index);
    }
    return 0;
}

// IDA_EA: 0x81A398E
// IDA_NAME: _ZN30Arad_OnePlusCardEvent_ItemList9find_itemEtRNS_4DataE
// DEMANGLED: Arad_OnePlusCardEvent_ItemList::find_item(unsigned short,Arad_OnePlusCardEvent_ItemList::Data &)
int Arad_OnePlusCardEvent_ItemList::find_item(uint16_t web_id_value,
                                              Data& out) {
    const Data key(web_id_value, 0);
    std::vector<Data>::iterator it =
        std::find(item_list_.begin(), item_list_.end(), key);
    if (it == item_list_.end()) {
        return 0;
    }
    out = *it;
    return 1;
}

// IDA_EA: 0x81A3A6E
// IDA_NAME: _ZN30Arad_OnePlusCardEvent_ItemList9find_itemEt
// DEMANGLED: Arad_OnePlusCardEvent_ItemList::find_item(unsigned short)
int Arad_OnePlusCardEvent_ItemList::find_item(uint16_t web_id_value) {
    Data out(0, 0);
    return find_item(web_id_value, out);
}

// IDA_EA: 0x81A3AB2
// IDA_NAME: _ZN30Arad_OnePlusCardEvent_ItemList14is_limited_mapEj
// DEMANGLED: Arad_OnePlusCardEvent_ItemList::is_limited_map(unsigned int)
bool Arad_OnePlusCardEvent_ItemList::is_limited_map(
    unsigned int map_index) {
    return std::find(limited_maps_.begin(), limited_maps_.end(), map_index) !=
           limited_maps_.end();
}
