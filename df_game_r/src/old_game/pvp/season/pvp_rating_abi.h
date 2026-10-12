#ifndef OLDSRV_GLOBAL_PVP_RATING_ABI_H_
#define OLDSRV_GLOBAL_PVP_RATING_ABI_H_

// The enum is owned by the fair-PvP match ABI.  It is forward declared here
// so the global old-server boundary remains a typed declaration without
// duplicating its value table.
enum ENUM_ELO_RATING_SYSTEM : int;

// Declaration-only external provider.
// IDA_EA: 0x844DCA1
// IDA_NAME: _ZN10CEloRating4calcERfS0_22ENUM_ELO_RATING_SYSTEMi
// DEMANGLED: CEloRating::calc(float &,float &,ENUM_ELO_RATING_SYSTEM,int)
class CEloRating {
public:
    static float& calc(float& rating,
                       float& opposing_rating,
                       ENUM_ELO_RATING_SYSTEM rating_system,
                       int k_factor);
};

// These five values occupy one contiguous initialized global block in the
// original school-match runtime.  DB_UpdatePvPResult consumes the daily-point
// values while live matches consume the two Elo factors.
class PvPGlobalEnvironmentVariable {
public:
    // IDA_EA: 0x93ABB60
    // IDA_NAME: _ZN28PvPGlobalEnvironmentVariable25school_match_daily_point_E
    // DEMANGLED: PvPGlobalEnvironmentVariable::school_match_daily_point_
    static int school_match_daily_point_;

    // IDA_EA: 0x93ABB64
    // IDA_NAME: _ZN28PvPGlobalEnvironmentVariable34school_match_daily_point_weekends_E
    // DEMANGLED: PvPGlobalEnvironmentVariable::school_match_daily_point_weekends_
    static int school_match_daily_point_weekends_;

    // IDA_EA: 0x93ABB68
    // IDA_NAME: _ZN28PvPGlobalEnvironmentVariable40school_match_fight_count_for_gain_point_E
    // DEMANGLED: PvPGlobalEnvironmentVariable::school_match_fight_count_for_gain_point_
    static int school_match_fight_count_for_gain_point_;

    // IDA_EA: 0x93ABB6C
    // IDA_NAME: _ZN28PvPGlobalEnvironmentVariable19school_match_elo_k_E
    // DEMANGLED: PvPGlobalEnvironmentVariable::school_match_elo_k_
    static int school_match_elo_k_;

    // IDA_EA: 0x93ABB70
    // IDA_NAME: _ZN28PvPGlobalEnvironmentVariable20school_match_elo_k2_E
    // DEMANGLED: PvPGlobalEnvironmentVariable::school_match_elo_k2_
    static int school_match_elo_k2_;
};

#endif  // OLDSRV_GLOBAL_PVP_RATING_ABI_H_
