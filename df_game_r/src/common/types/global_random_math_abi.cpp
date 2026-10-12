#include <cmath>
#include <cstdlib>

// IDA_EA: 0x8539D90
// IDA_NAME: _Z14GetRandomRangeii
// DEMANGLED: GetRandomRange(int,int)
int GetRandomRange(int minimum, int maximum) {
    const int random_value = std::rand();
    if (maximum >= minimum) {
        return minimum + random_value % (maximum - minimum + 1);
    }
    return maximum + random_value % (minimum - maximum + 1);
}

// IDA_EA: 0x853ABE5
// IDA_NAME: _Z10roundFloatf
// DEMANGLED: roundFloat(float)
int roundFloat(float value) {
    return static_cast<int>(std::round(value));
}
