#ifndef OLDSRV_GLOBAL_INVALID_CHAR_DATA_H_
#define OLDSRV_GLOBAL_INVALID_CHAR_DATA_H_

#include <stdint.h>
#include <vector>


// IDA unit: 99_misc_global/InvalidCharData.
// IDA_EA: ctor/dtor @0x837D168/0x837D1FA.
struct InvalidCharData {
    template <typename T>
    struct stRangeElement {
        stRangeElement() : begin(), end() {}
        stRangeElement(T range_begin, T range_end)
            : begin(range_begin), end(range_end) {}

        T begin;
        T end;
    };

    InvalidCharData();
    ~InvalidCharData();

    std::vector<unsigned char> one_byte_chars;
    std::vector<stRangeElement<unsigned char> > one_byte_ranges;
    std::vector<unsigned short> two_byte_chars;
    std::vector<stRangeElement<unsigned short> > two_byte_ranges;
};

static_assert(sizeof(InvalidCharData) == 0x30u,
              "InvalidCharData IDA ABI size must stay 0x30");


#endif  // OLDSRV_GLOBAL_INVALID_CHAR_DATA_H_
