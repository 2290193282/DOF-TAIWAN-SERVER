#include "old_game/player/core/cuser.h"

#include <stdint.h>

// This source owns only the seven directly evidenced cUserHistoryLog leaves
// that were absent from the canonical C++ tree.  The formatting strings and
// the ExpAdd reason gate are copied from their individual IDA bodies; they
// are not compatibility wrappers around a different history ABI.

// IDA_EA: 0x8683CEE
// IDA_NAME: _ZN15cUserHistoryLog12ItemAddCargoEiii
// DEMANGLED: cUserHistoryLog::ItemAddCargo(int,int,int)
int cUserHistoryLog::ItemAddCargo(int item_id,
                                  int item_count,
                                  int resulting_count) {
    return owner_->LogHistory("Inven+,%d,%d,%d",
                              item_id,
                              item_count,
                              resulting_count);
}

// IDA_EA: 0x8684BD0
// IDA_NAME: _ZN15cUserHistoryLog6ExpAddEii13eExpAddReasoni
// DEMANGLED: cUserHistoryLog::ExpAdd(int,int,eExpAddReason,int)
int cUserHistoryLog::ExpAdd(int current_exp,
                            int added_exp,
                            eExpAddReason reason,
                            int source_value) {
    const unsigned int reason_value = static_cast<unsigned int>(reason);
    if (reason_value > 0x0cu) {
        return owner_->LogHistory("Exp+,%d,%d,%d",
                                  current_exp,
                                  added_exp,
                                  source_value);
    }

    const unsigned int suppressed = (1u << reason_value) & 0x1892u;
    if (suppressed == 0u) {
        return owner_->LogHistory("Exp+,%d,%d,%d",
                                  current_exp,
                                  added_exp,
                                  source_value);
    }
    return static_cast<int>(suppressed);
}

// IDA_EA: 0x8685418
// IDA_NAME: _ZN15cUserHistoryLog15HadesHistoryLogEjjjj
// DEMANGLED: cUserHistoryLog::HadesHistoryLog(unsigned int,unsigned int,unsigned int,unsigned int)
int cUserHistoryLog::HadesHistoryLog(unsigned int value0,
                                     unsigned int value1,
                                     unsigned int value2,
                                     unsigned int value3) {
    return owner_->LogHistory("Hades,%d,%d,%d,%d",
                              value0,
                              value1,
                              value2,
                              value3);
}

// IDA_EA: 0x8685A08
// IDA_NAME: _ZN15cUserHistoryLog9useEmblemEmmjPKcj
// DEMANGLED: cUserHistoryLog::useEmblem(unsigned long,unsigned long,unsigned int,char const*,unsigned int)
int cUserHistoryLog::useEmblem(unsigned long avatar_key,
                               unsigned long emblem_key,
                               unsigned int emblem_item_id,
                               const char* agency_no,
                               unsigned int value) {
    return owner_->LogHistory("UseEmblem,%d,%d,%d,%s,%d",
                              avatar_key,
                              emblem_key,
                              emblem_item_id,
                              agency_no,
                              value);
}

// IDA_EA: 0x868683A
// IDA_NAME: _ZN15cUserHistoryLog12CashInvenAddE19eCashInvenAddReasonjjm
// DEMANGLED: cUserHistoryLog::CashInvenAdd(eCashInvenAddReason,unsigned int,unsigned int,unsigned long)
int cUserHistoryLog::CashInvenAdd(eCashInvenAddReason reason,
                                  unsigned int item_id,
                                  unsigned int item_count,
                                  unsigned long serial) {
    return owner_->LogHistory("CashInven+,%u,%u,%u,%u",
                              static_cast<unsigned int>(reason),
                              item_id,
                              item_count,
                              serial);
}

// IDA_EA: 0x8686874
// IDA_NAME: _ZN15cUserHistoryLog12CashInvenDelE19eCashInvenDelReasonjjmj
// DEMANGLED: cUserHistoryLog::CashInvenDel(eCashInvenDelReason,unsigned int,unsigned int,unsigned long,unsigned int)
int cUserHistoryLog::CashInvenDel(eCashInvenDelReason reason,
                                  unsigned int item_id,
                                  unsigned int item_count,
                                  unsigned long serial,
                                  unsigned int value) {
    return owner_->LogHistory("CashInven-,%u,%u,%u,%u,%u",
                              static_cast<unsigned int>(reason),
                              item_id,
                              item_count,
                              serial,
                              value);
}

// IDA_EA: 0x8686C36
// IDA_NAME: _ZN15cUserHistoryLog9seal_ItemEis15eSealItemReason
// DEMANGLED: cUserHistoryLog::seal_Item(int,short,eSealItemReason)
CUser* cUserHistoryLog::seal_Item(int item_id,
                                  short reseal_count,
                                  eSealItemReason reason) {
    if (owner_ == 0) {
        return owner_;
    }
    return reinterpret_cast<CUser*>(static_cast<uintptr_t>(
        owner_->LogHistory("SealItem ,%d,%d,%d",
                           item_id,
                           reseal_count,
                           reason)));
}
