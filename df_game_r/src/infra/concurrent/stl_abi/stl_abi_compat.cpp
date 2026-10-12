#include <set>
#include <utility>

typedef std::pair<unsigned char, unsigned short> PairUCharUShort;
typedef std::set<PairUCharUShort> SetPairUCharUShort;

// Hex-Rays/STL ABI compatibility for old code that references the out-of-line
// std::set clear symbol directly.
namespace std {

template void set<pair<unsigned char, unsigned short> >::clear();

}
