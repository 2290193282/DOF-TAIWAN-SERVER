#ifndef OLDSRV_07_NPC_SHOP_DIALOG_NPC_NAME_VALUES_H_
#define OLDSRV_07_NPC_SHOP_DIALOG_NPC_NAME_VALUES_H_

#include <string>
#include <vector>

// IDA unit: 07_npc_shop_dialog/CNNpcName.
// IDA_EA: 0x89D7D16.
class CNNpcName : public std::string {
public:
    ~CNNpcName();
};

// IDA unit: 07_npc_shop_dialog/CNNpcNameVector.
// IDA_EA: 0x89C3654 / 0x89C3668 / 0x89DC4BA.
class CNNpcNameVector : public std::vector<CNNpcName> {
public:
    int clear();
    CNNpcNameVector();
    ~CNNpcNameVector();
};

#endif  // OLDSRV_07_NPC_SHOP_DIALOG_NPC_NAME_VALUES_H_
