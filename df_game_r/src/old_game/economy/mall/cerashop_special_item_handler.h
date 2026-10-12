#ifndef OLDSRV_OLD_GAME_NPC_CERASHOP_SPECIAL_ITEM_HANDLER_H_
#define OLDSRV_OLD_GAME_NPC_CERASHOP_SPECIAL_ITEM_HANDLER_H_

namespace WongWork {
class CCeraShop;
}  // namespace WongWork

struct STSpecailItem_Param;
struct STSpecailItem_Result;

// IDA unit: 99_misc_global/SpecialItemHandler.
class SpecialItemHandler {
public:
    typedef int (SpecialItemHandler::*Response)(
        int special_item_type,
        const STSpecailItem_Param& param,
        STSpecailItem_Result& result);

    struct ResponseSlot {
        Response response;
    };

    // IDA_EA: 0x827BCEC
    // IDA_NAME: _ZN18SpecialItemHandlerC2EPN8WongWork9CCeraShopE
    // DEMANGLED: SpecialItemHandler::SpecialItemHandler(WongWork::CCeraShop *)
    explicit SpecialItemHandler(WongWork::CCeraShop* cera_shop);

    // IDA_EA: 0x8328284
    // IDA_NAME: _ZN18SpecialItemHandlerD2Ev
    // DEMANGLED: SpecialItemHandler::~SpecialItemHandler()
    ~SpecialItemHandler();

    // IDA_EA: 0x827BE0A
    // IDA_NAME: _ZN18SpecialItemHandler7executeEiRK19STSpecailItem_ParamR20STSpecailItem_Result
    // DEMANGLED: SpecialItemHandler::execute(int,STSpecailItem_Param const&,STSpecailItem_Result&)
    int execute(int special_item_type,
                const STSpecailItem_Param& param,
                STSpecailItem_Result& result);

private:
    int respond_special_item_cargo(int special_item_type,
                                   const STSpecailItem_Param& param,
                                   STSpecailItem_Result& result);
    int respond_special_item_cargo_abs(int special_item_type,
                                       const STSpecailItem_Param& param,
                                       STSpecailItem_Result& result);
    int respond_special_item_inventory_upgrade(
        int special_item_type,
        const STSpecailItem_Param& param,
        STSpecailItem_Result& result);
    int respond_special_item_charac_slot_extend(
        int special_item_type,
        const STSpecailItem_Param& param,
        STSpecailItem_Result& result);
    int respond_special_item_package(int special_item_type,
                                     const STSpecailItem_Param& param,
                                     STSpecailItem_Result& result);
    int respond_special_item_booster(int special_item_type,
                                     const STSpecailItem_Param& param,
                                     STSpecailItem_Result& result);
    int respond_special_item_booster_selection(
        int special_item_type,
        const STSpecailItem_Param& param,
        STSpecailItem_Result& result);
    int respond_special_item_gift_ticket(int special_item_type,
                                         const STSpecailItem_Param& param,
                                         STSpecailItem_Result& result);
    int respond_special_item_skill_confirm(int special_item_type,
                                           const STSpecailItem_Param& param,
                                           STSpecailItem_Result& result);
    int respond_special_item_skill_tree_style(
        int special_item_type,
        const STSpecailItem_Param& param,
        STSpecailItem_Result& result);
    int respond_special_item_premium(int special_item_type,
                                     const STSpecailItem_Param& param,
                                     STSpecailItem_Result& result);
    int respond_special_item_cerapoint_giftcard(
        int special_item_type,
        const STSpecailItem_Param& param,
        STSpecailItem_Result& result);

    WongWork::CCeraShop* cera_shop_;  // +0x00
    ResponseSlot response_slots_[14]; // +0x04
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(SpecialItemHandler::ResponseSlot) == 0x08u,
              "SpecialItemHandler response slot must stay 0x08");
static_assert(sizeof(SpecialItemHandler) == 0x74u,
              "SpecialItemHandler IDA ABI size must stay 0x74");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_CERASHOP_SPECIAL_ITEM_HANDLER_H_
