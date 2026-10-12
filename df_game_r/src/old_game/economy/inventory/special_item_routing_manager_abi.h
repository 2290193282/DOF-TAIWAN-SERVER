#ifndef OLDSRV_GLOBAL_SPECIAL_ITEM_ROUTING_MANAGER_ABI_H_
#define OLDSRV_GLOBAL_SPECIAL_ITEM_ROUTING_MANAGER_ABI_H_


class CParty;
class CUser;
struct Inven_Item;

// IDA unit: 99_misc_global/CSpecialItemRoutingManager.
class CSpecialItemRoutingManager {
public:
    CSpecialItemRoutingManager();
    ~CSpecialItemRoutingManager();

    void ProcessFailEtc(CParty* party, int reason);
    int cancelTimer(CParty* party);

    // IDA_EA: 0x860AB2E
    // IDA_NAME: _ZN26CSpecialItemRoutingManager21ArriveMemberSelectionEP5CUserP6CPartyc
    // DEMANGLED: CSpecialItemRoutingManager::ArriveMemberSelection(CUser *,CParty *,char)
    int ArriveMemberSelection(CUser* user, CParty* party, char selection);

    // IDA_EA: 0x860A9D4
    // IDA_NAME: _ZN26CSpecialItemRoutingManager12StartRoutingEP5CUserR10Inven_Itemj
    // DEMANGLED: CSpecialItemRoutingManager::StartRouting(CUser *,Inven_Item &,unsigned int)
    int StartRouting(CUser* user, Inven_Item& item, unsigned int unique_id);

    // The exact provider is a separate reward-distribution chain.  Keep its
    // native declaration so the selection state machine does not invent a
    // replacement for that chain while it remains an explicit L1 link debt.
    // IDA_EA: 0x860AC6A
    // IDA_NAME: _ZN26CSpecialItemRoutingManager14processRoutingEP6CParty
    // DEMANGLED: CSpecialItemRoutingManager::processRouting(CParty *)
    void processRouting(CParty* party);

    // IDA_EA: 0x860B2C2
    // IDA_NAME: _ZN26CSpecialItemRoutingManager13ProcessGiveUpEP6CParty
    // DEMANGLED: CSpecialItemRoutingManager::ProcessGiveUp(CParty *)
    void ProcessGiveUp(CParty* party);

    // IDA_EA: 0x860B45C
    // IDA_NAME: _ZN26CSpecialItemRoutingManager11insertTimerEP6CParty
    // DEMANGLED: CSpecialItemRoutingManager::insertTimer(CParty *)
    int insertTimer(CParty* party);
};


#endif  // OLDSRV_GLOBAL_SPECIAL_ITEM_ROUTING_MANAGER_ABI_H_
