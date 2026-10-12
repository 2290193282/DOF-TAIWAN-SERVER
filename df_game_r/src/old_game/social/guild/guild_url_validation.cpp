#include "old_game/social/guild/guild_url_validation.h"

#include <cstring>


// IDA_EA: 0x8205791
// IDA_NAME: _Z9check_urlPKc
// DEMANGLED: check_url(char const*)
int check_url(const char* text) {
    if (!text || text[0] == '_') {
        return 0;
    }

    for (unsigned int index = 0; index < std::strlen(text); ++index) {
        const unsigned char value =
            static_cast<unsigned char>(text[index]);
        if ((value > 0x60u && value <= 0x7au) ||
            (value > 0x2fu && value <= 0x57u) || value == 0x5fu) {
            continue;
        }
        return 0;
    }
    return 1;
}
