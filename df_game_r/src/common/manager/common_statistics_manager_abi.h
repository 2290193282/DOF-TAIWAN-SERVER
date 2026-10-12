#ifndef OLDSRV_GLOBAL_COMMON_STATISTICS_MANAGER_ABI_H_
#define OLDSRV_GLOBAL_COMMON_STATISTICS_MANAGER_ABI_H_

#include <map>

class CStatisticContainer {
public:
    // CCommonStatisticsMgr::InsertDatabase at 0x860E86E dispatches this
    // first vtable slot for each concrete statistic bucket.
    virtual void InsertDatabase() = 0;
    CStatisticContainer();
    virtual ~CStatisticContainer();

    // IDA_EA: 0x860E7F8
    // IDA_NAME: _ZN19CStatisticContainer16IncreaseQuantityEi
    int* IncreaseQuantity(int key);

protected:
    std::map<int, int> quantities_;
};

class CHelpedPartyCount : public CStatisticContainer {
public:
    CHelpedPartyCount();
    virtual void InsertDatabase();
};

class CPVPPlayerRate : public CStatisticContainer {
public:
    CPVPPlayerRate();
    virtual void InsertDatabase();
};

class CPVPMapCount : public CStatisticContainer {
public:
    CPVPMapCount();
    // IDA_EA: 0x860EB16
    // IDA_NAME: _ZN12CPVPMapCount14InsertDatabaseEv
    // DEMANGLED: CPVPMapCount::InsertDatabase(void)
    virtual void InsertDatabase();
};

class CCommonStatisticsMgr {
public:
    enum ENUM_STATISTIC_CONTAINER {
        ENUM_HELPED_PARTY_COUNT = 0,
        ENUM_PVP_PLAYER_RATE = 1,
        ENUM_PVP_MAP_COUNT = 2,
    };

    CCommonStatisticsMgr();
    ~CCommonStatisticsMgr();

    // IDA_EA: 0x860E86E
    // IDA_NAME: _ZN20CCommonStatisticsMgr14InsertDatabaseEv
    // DEMANGLED: CCommonStatisticsMgr::InsertDatabase(void)
    void InsertDatabase();

    // IDA_EA: 0x860E838
    // IDA_NAME: _ZN20CCommonStatisticsMgr16IncreaseQuantityENS_24ENUM_STATISTIC_CONTAINEREi
    int* IncreaseQuantity(ENUM_STATISTIC_CONTAINER container, int key);

private:
    CStatisticContainer* containers_[3];
};

// IDA_EA: 0x860E765
// IDA_NAME: _Z30GetInstanceCommonStatisticsMgrv
CCommonStatisticsMgr* GetInstanceCommonStatisticsMgr();

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CStatisticContainer) == 0x1cu,
              "CStatisticContainer must stay 0x1c bytes");
static_assert(sizeof(CCommonStatisticsMgr) == 0x0cu,
              "CCommonStatisticsMgr must stay 0x0c bytes");
#endif

#endif  // OLDSRV_GLOBAL_COMMON_STATISTICS_MANAGER_ABI_H_
