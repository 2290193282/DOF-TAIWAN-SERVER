#ifndef OLDSRV_GLOBAL_STATISTICS_ABI_H_
#define OLDSRV_GLOBAL_STATISTICS_ABI_H_

// IDA unit: 99_misc_global/Statistics.
// These methods are static event façades; the first PVPPlay2 integer is a
// room/map value, not a hidden Statistics object.
class Statistics {
public:
    // IDA_EA: 0x860B8B4
    // IDA_NAME: _ZN10Statistics11DungeonFailEi
    static int DungeonFail(int character_job);

    // IDA_EA: 0x860B90A
    // IDA_NAME: _ZN10Statistics16UseShusiaServiceEi
    // DEMANGLED: Statistics::UseShusiaService(int)
    static int UseShusiaService(int character_level);

    // IDA_EA: 0x860B8D8
    // IDA_NAME: _ZN10Statistics8PVPPlay2Eiii
    static int PVPPlay2(int room_number,
                        int play_type,
                        int participant_count);
};

#endif  // OLDSRV_GLOBAL_STATISTICS_ABI_H_
