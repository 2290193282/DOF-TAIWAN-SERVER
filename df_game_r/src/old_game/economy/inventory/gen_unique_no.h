#ifndef OLDSRV_OLD_GAME_INVENTORY_GEN_UNIQUE_NO_H_
#define OLDSRV_OLD_GAME_INVENTORY_GEN_UNIQUE_NO_H_

#include <stdint.h>

namespace WongWork {

class CGenUniqueNo {
public:
    enum ENUM_IPGNO_TYPE {
        ENUM_IPGNO_TYPE_0 = 0,
    };

    // IDA_EA: 0x84B7750
    // IDA_NAME: _ZN8WongWork12CGenUniqueNo8genIPGNoENS0_15ENUM_IPGNO_TYPEEjPc
    // DEMANGLED: WongWork::CGenUniqueNo::genIPGNo(WongWork::CGenUniqueNo::ENUM_IPGNO_TYPE,unsigned int,char *)
    static char* genIPGNo(ENUM_IPGNO_TYPE type,
                          unsigned int charac_no,
                          char* destination);

    // IDA_EA: 0x84B76AC
    // IDA_NAME: _ZN8WongWork12CGenUniqueNo16_updateTimeCountEv
    // DEMANGLED: WongWork::CGenUniqueNo::_updateTimeCount(void)
    static int _updateTimeCount();

    // IDA_EA: 0x84B76D8
    // IDA_NAME: _ZN8WongWork12CGenUniqueNo11GenUniqueNoEPc
    // DEMANGLED: WongWork::CGenUniqueNo::GenUniqueNo(char *)
    static char* GenUniqueNo(char* destination);

private:
    // IDA global: _ZN8WongWork12CGenUniqueNo12m_nTimeCountE @ 0x943DC8C.
    static uint16_t m_nTimeCount;
};

}  // namespace WongWork

#endif  // OLDSRV_OLD_GAME_INVENTORY_GEN_UNIQUE_NO_H_
