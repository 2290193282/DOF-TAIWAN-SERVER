#ifndef OLDSRV_GLOBAL_GM_LIST_ABI_H_
#define OLDSRV_GLOBAL_GM_LIST_ABI_H_

class MySQL;

// IDA unit: 99_misc_global/Gm_List.  The object stores exactly one MySQL
// handle pointer.  Its owner remains the GlobalData lifecycle.
class Gm_List {
public:
    // IDA_EA: 0x8299912
    // IDA_NAME: _ZN7Gm_ListC2Ev
    // DEMANGLED: Gm_List::Gm_List(void)
    Gm_List();

    // IDA_EA: 0x829996C
    // IDA_NAME: _ZN7Gm_List16LoadGmListFromDbEv
    // DEMANGLED: Gm_List::LoadGmListFromDb(void)
    int LoadGmListFromDb();

    // IDA_EA: 0x8299D54
    // IDA_NAME: _ZN7Gm_List25SendGMList_To_UpperServerEi
    // DEMANGLED: Gm_List::SendGMList_To_UpperServer(int)
    void SendGMList_To_UpperServer(int upper_server);

private:
    MySQL* db_handle_;  // +0x00
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(Gm_List) == 0x04u,
              "Gm_List IDA ABI size must stay 4");
#endif

#endif  // OLDSRV_GLOBAL_GM_LIST_ABI_H_
