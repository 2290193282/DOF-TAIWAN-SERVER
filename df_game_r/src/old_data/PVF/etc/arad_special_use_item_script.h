#ifndef OLDSRV_DATAMANAGER_ARAD_SPECIAL_USE_ITEM_SCRIPT_H_
#define OLDSRV_DATAMANAGER_ARAD_SPECIAL_USE_ITEM_SCRIPT_H_

#include "old_data/PVF/common/arad_script_loader.h"

#include <stdint.h>

#include <map>
#include <vector>

// IDA units: 99_misc_global/ExceptionItem, PurchaseLimitItem,
// AradBonusItem, and Arad_Mileage_Data.  They are embedded directly in
// ARAD::Arad_DataManager, so these are typed RDAR script owners rather than
// byte-storage placeholders.
class ExceptionItem : public Arad_ScriptLoader {
public:
    ExceptionItem();
    virtual ~ExceptionItem();

    virtual int parsing(char* buffer);
    bool find_index(int item_index);

private:
    std::map<int, int> item_indexes_;
};

class PurchaseLimitItem : public Arad_ScriptLoader {
public:
    struct Info {
        Info();

        uint16_t purchase_type;
        uint16_t purchase_limit_count;
        uint32_t reserved;
    };

    PurchaseLimitItem();
    virtual ~PurchaseLimitItem();

    virtual int parsing(char* buffer);
    int find(uint32_t item_index, Info& out);

private:
    std::map<uint32_t, Info> items_;
};

class AradBonusItem : public Arad_ScriptLoader {
public:
    struct Data {
        Data();

        int bonus_value;
        uint32_t item_index;
        uint16_t bonus_type;
    };

    AradBonusItem();
    virtual ~AradBonusItem();

    virtual int parsing(char* buffer);
    int find(uint32_t item_index, Data& out);

private:
    std::map<uint32_t, Data> items_;
};

class Arad_Mileage_Data : public Arad_ScriptLoader {
public:
    struct Data {
        Data(int ipg_no_value, int percent_value);
        ~Data();

        int ipg_no;
        int percent;
    };

    Arad_Mileage_Data();
    virtual ~Arad_Mileage_Data();

    virtual int parsing(char* buffer);
    int find(int ipg_no);
    int default_value();

private:
    uint32_t default_percent_;
    std::vector<Data> items_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ExceptionItem) == 0x1Cu,
              "ExceptionItem IDA ABI size must stay 0x1C");
static_assert(sizeof(PurchaseLimitItem::Info) == 0x08u,
              "PurchaseLimitItem::Info IDA ABI size must stay 0x08");
static_assert(sizeof(PurchaseLimitItem) == 0x1Cu,
              "PurchaseLimitItem IDA ABI size must stay 0x1C");
static_assert(sizeof(AradBonusItem::Data) == 0x0Cu,
              "AradBonusItem::Data IDA ABI size must stay 0x0C");
static_assert(sizeof(AradBonusItem) == 0x1Cu,
              "AradBonusItem IDA ABI size must stay 0x1C");
static_assert(sizeof(Arad_Mileage_Data::Data) == 0x08u,
              "Arad_Mileage_Data::Data IDA ABI size must stay 0x08");
static_assert(sizeof(Arad_Mileage_Data) == 0x14u,
              "Arad_Mileage_Data IDA ABI size must stay 0x14");
#endif

#endif  // OLDSRV_DATAMANAGER_ARAD_SPECIAL_USE_ITEM_SCRIPT_H_
