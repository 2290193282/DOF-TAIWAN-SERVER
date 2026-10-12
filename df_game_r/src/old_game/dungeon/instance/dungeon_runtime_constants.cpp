
// IDA_EA: referenced as CONST_SINGLE_MAP_LOADING_WAIT_TIMEOUT in CParty and
// WongWork::CDeathTower map-loading timer paths.  The original ELF32 local
// object is 0x78 (120).  TimerQueue::InsertTimer accepts seconds and performs
// the seconds-to-milliseconds conversion itself.
int CONST_SINGLE_MAP_LOADING_WAIT_TIMEOUT
    __asm__("CONST_SINGLE_MAP_LOADING_WAIT_TIMEOUT") = 120;

// IDA_EA: referenced as CONST_PARTY_MAP_LOADING_WAIT_TIMEOUT in CParty and
// WongWork::CDeathTower map-loading timer paths.  The original ELF32 local
// object is 0x5a (90), likewise expressed in seconds.
int CONST_PARTY_MAP_LOADING_WAIT_TIMEOUT
    __asm__("CONST_PARTY_MAP_LOADING_WAIT_TIMEOUT") = 90;
