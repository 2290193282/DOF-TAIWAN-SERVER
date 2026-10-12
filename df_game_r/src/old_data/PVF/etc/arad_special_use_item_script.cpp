#include "old_data/PVF/etc/arad_special_use_item_script.h"

#include "old_data/PVF/common/arad_script.h"
#include "infra/log/format/trace.h"

namespace {

char* Mutable(const char* text) {
    return const_cast<char*>(text);
}

}  // namespace

// IDA_EA: 0x8188B50
// IDA_NAME: _ZN13ExceptionItemC2Ev
// DEMANGLED: ExceptionItem::ExceptionItem(void)
ExceptionItem::ExceptionItem() : Arad_ScriptLoader(), item_indexes_() {}

// IDA_EA: 0x8188B7A
// IDA_NAME: _ZN13ExceptionItemD2Ev
// DEMANGLED: ExceptionItem::~ExceptionItem()
ExceptionItem::~ExceptionItem() {}

// IDA_EA: 0x81A1FCE
// IDA_NAME: _ZN13ExceptionItem7parsingEPc
// DEMANGLED: ExceptionItem::parsing(char *)
int ExceptionItem::parsing(char* buffer) {
    Arad_Script script;
    if (script.Load(buffer, "material_exception_item") != 1) {
        return 39;
    }
    if (script.GetRowNum() == 0) {
        return 42;
    }
    for (int row = 0; row < script.GetRowNum(); ++row) {
        const int item_index =
            script.GetInt(row, Mutable("material_exception_item_index"));
        if (!item_indexes_.insert(std::make_pair(item_index, item_index)).second) {
            return 51;
        }
    }
    return 0;
}

// IDA_EA: 0x81A210C
// IDA_NAME: _ZN13ExceptionItem10find_indexEi
// DEMANGLED: ExceptionItem::find_index(int)
bool ExceptionItem::find_index(int item_index) {
    return item_indexes_.find(item_index) != item_indexes_.end();
}

// IDA_EA: 0x81A405E
// IDA_NAME: _ZN17PurchaseLimitItem4InfoC2Ev
// DEMANGLED: PurchaseLimitItem::Info::Info(void)
PurchaseLimitItem::Info::Info()
    : purchase_type(0), purchase_limit_count(0), reserved(0) {}

// IDA_EA: 0x8188BAE
// IDA_NAME: _ZN17PurchaseLimitItemC2Ev
// DEMANGLED: PurchaseLimitItem::PurchaseLimitItem(void)
PurchaseLimitItem::PurchaseLimitItem() : Arad_ScriptLoader(), items_() {}

// IDA_EA: 0x8188BD8
// IDA_NAME: _ZN17PurchaseLimitItemD2Ev
// DEMANGLED: PurchaseLimitItem::~PurchaseLimitItem()
PurchaseLimitItem::~PurchaseLimitItem() {}

// IDA_EA: 0x81A216E
// IDA_NAME: _ZN17PurchaseLimitItem7parsingEPc
// DEMANGLED: PurchaseLimitItem::parsing(char *)
int PurchaseLimitItem::parsing(char* buffer) {
    Arad_Script script;
    if (script.Load(buffer, "purchase_limit_item") != 1) {
        return 76;
    }
    if (script.GetRowNum() == 0) {
        return 79;
    }
    for (int row = 0; row < script.GetRowNum(); ++row) {
        const uint32_t item_index = static_cast<uint32_t>(
            script.GetInt(row, Mutable("Item_Index")));
        Info info;
        info.purchase_type = static_cast<uint16_t>(
            script.GetInt(row, Mutable("purchase_Type")));
        info.purchase_limit_count = static_cast<uint16_t>(
            script.GetInt(row, Mutable("puchase_limit_count")));
        if (!items_.insert(std::make_pair(item_index, info)).second) {
            return 92;
        }
    }
    return 0;
}

// IDA_EA: 0x81A2310
// IDA_NAME: _ZN17PurchaseLimitItem4findEjRNS_4InfoE
// DEMANGLED: PurchaseLimitItem::find(unsigned int,PurchaseLimitItem::Info &)
int PurchaseLimitItem::find(uint32_t item_index, Info& out) {
    std::map<uint32_t, Info>::iterator it = items_.find(item_index);
    if (it == items_.end()) {
        return 0;
    }
    out = it->second;
    return 1;
}

// IDA_EA: 0x81A407E
// IDA_NAME: _ZN13AradBonusItem4DataC2Ev
// DEMANGLED: AradBonusItem::Data::Data(void)
AradBonusItem::Data::Data() : bonus_value(0), item_index(0), bonus_type(0) {}

// IDA_EA: 0x8188C0C
// IDA_NAME: _ZN13AradBonusItemC2Ev
// DEMANGLED: AradBonusItem::AradBonusItem(void)
AradBonusItem::AradBonusItem() : Arad_ScriptLoader(), items_() {}

// IDA_EA: 0x8188C36
// IDA_NAME: _ZN13AradBonusItemD2Ev
// DEMANGLED: AradBonusItem::~AradBonusItem()
AradBonusItem::~AradBonusItem() {}

// IDA_EA: 0x81A238A
// IDA_NAME: _ZN13AradBonusItem7parsingEPc
// DEMANGLED: AradBonusItem::parsing(char *)
int AradBonusItem::parsing(char* buffer) {
    Arad_Script script;
    if (script.Load(buffer, "bonus_item") != 1) {
        return 121;
    }
    if (script.GetRowNum() == 0) {
        return 124;
    }
    for (int row = 0; row < script.GetRowNum(); ++row) {
        Data data;
        data.item_index = static_cast<uint32_t>(
            script.GetInt(row, Mutable("item_id")));
        data.bonus_type = static_cast<uint16_t>(
            script.GetInt(row, Mutable("bonus_type")));
        data.bonus_value = script.GetInt(row, Mutable("bonus_value"));
        if (!items_.insert(std::make_pair(data.item_index, data)).second) {
            return 136;
        }
    }
    return 0;
}

// IDA_EA: 0x81A252C
// IDA_NAME: _ZN13AradBonusItem4findEjRNS_4DataE
// DEMANGLED: AradBonusItem::find(unsigned int,AradBonusItem::Data &)
int AradBonusItem::find(uint32_t item_index, Data& out) {
    std::map<uint32_t, Data>::iterator it = items_.find(item_index);
    if (it == items_.end()) {
        return 0;
    }
    out = it->second;
    return 1;
}

// IDA_EA: 0x81A41D2
// IDA_NAME: _ZN17Arad_Mileage_Data4DataC2Eii
// DEMANGLED: Arad_Mileage_Data::Data::Data(int,int)
Arad_Mileage_Data::Data::Data(int ipg_no_value, int percent_value)
    : ipg_no(ipg_no_value), percent(percent_value) {}

// IDA_EA: 0x81A7F76
// IDA_NAME: _ZN17Arad_Mileage_Data4DataD2Ev
// DEMANGLED: Arad_Mileage_Data::Data::~Data()
Arad_Mileage_Data::Data::~Data() {}

// IDA_EA: 0x8188CC0
// IDA_NAME: _ZN17Arad_Mileage_DataC2Ev
// DEMANGLED: Arad_Mileage_Data::Arad_Mileage_Data(void)
Arad_Mileage_Data::Arad_Mileage_Data()
    : Arad_ScriptLoader(), default_percent_(0), items_() {}

// IDA_EA: 0x8188CEA
// IDA_NAME: _ZN17Arad_Mileage_DataD2Ev
// DEMANGLED: Arad_Mileage_Data::~Arad_Mileage_Data()
Arad_Mileage_Data::~Arad_Mileage_Data() {}

// IDA_EA: 0x81A3416
// IDA_NAME: _ZN17Arad_Mileage_Data7parsingEPc
// DEMANGLED: Arad_Mileage_Data::parsing(char *)
int Arad_Mileage_Data::parsing(char* buffer) {
    Arad_Script script;
    if (script.Load(buffer, "default") != 1) {
        return 396;
    }
    if (script.GetRowNum() == 0) {
        return 399;
    }

    default_percent_ = static_cast<uint32_t>(
        script.GetInt(0, Mutable("percent")));
    if (default_percent_ > 1000u) {
        LogManager::logFormat(
            1, "localjapan/Arad_Script_Loader.cpp",
            "virtual int Arad_Mileage_Data::parsing(char*)", 407,
            "[ERROR] invalid mileage default value. (%u)", default_percent_);
        return 408;
    }

    if (script.Load(buffer, "item") != 1) {
        return 414;
    }
    for (int row = 0; row < script.GetRowNum(); ++row) {
        const int ipg_no = script.GetInt(row, Mutable("ipg_no"));
        const int percent = script.GetInt(row, Mutable("percent"));
        if (find(ipg_no) >= 0) {
            LogManager::logFormat(
                1, "localjapan/Arad_Script_Loader.cpp",
                "virtual int Arad_Mileage_Data::parsing(char*)", 432,
                "[ERROR] already exist item index (IPG:%d, percent:%d)",
                ipg_no, percent);
            return 433;
        }
        items_.push_back(Data(ipg_no, percent));
    }
    return 0;
}

// IDA_EA: 0x81A3668
// IDA_NAME: _ZN17Arad_Mileage_Data4findEi
// DEMANGLED: Arad_Mileage_Data::find(int)
int Arad_Mileage_Data::find(int ipg_no) {
    for (std::vector<Data>::iterator it = items_.begin();
         it != items_.end(); ++it) {
        if (it->ipg_no == ipg_no) {
            return it->percent;
        }
    }
    return -1;
}

// IDA_EA: 0x81A36F0
// IDA_NAME: _ZN17Arad_Mileage_Data13default_valueEv
// DEMANGLED: Arad_Mileage_Data::default_value(void)
int Arad_Mileage_Data::default_value() {
    return static_cast<int>(default_percent_);
}
