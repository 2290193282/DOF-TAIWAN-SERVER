#ifndef OLDSRV_OLD_DATA_PVF_ETC_RECOMMEND_NEWBIE_OPTION_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_ETC_RECOMMEND_NEWBIE_OPTION_SCRIPT_H_

#include <stdint.h>

#include <vector>

// IDA_EA: 0x891AD16 / 0x891AD2A / 0x893908C.
// The first matrix is filled by the nine avatar-index fields in each grade.
// The second matrix remains -1 except for the two absolute-defense fields.
struct tagRecommendNewbieOption {
    tagRecommendNewbieOption();
    ~tagRecommendNewbieOption();

    bool clear();

    int32_t grow_type;
    int32_t avatar_variation_index[9][3];
    int32_t avatar_variation_value[9][3];
};

// IDA_EA: 0x891AD8C.  The old type is a vector-derived table, not a wrapper
// containing a vector; direct callers use the vector base at offset zero.
class tagRecommendNewbieOptionTable
    : public std::vector<std::vector<tagRecommendNewbieOption> > {
public:
    bool clear();
};

// IDA_EA: 0x890457C
// IDA_NAME: _Z33importRecommendNewbieOptionScriptPKcR29tagRecommendNewbieOptionTable
// DEMANGLED: importRecommendNewbieOptionScript(char const*,tagRecommendNewbieOptionTable &)
int importRecommendNewbieOptionScript(
    const char* filename,
    tagRecommendNewbieOptionTable& table);

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(tagRecommendNewbieOption) == 0xDCu,
              "RecommendNewbie option row must preserve the old 0xDC ABI");
static_assert(sizeof(tagRecommendNewbieOptionTable) == 0x0Cu,
              "RecommendNewbie table must remain the old vector base");
#endif

#endif  // OLDSRV_OLD_DATA_PVF_ETC_RECOMMEND_NEWBIE_OPTION_SCRIPT_H_
