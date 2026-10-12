#include "infra/security/policy/pi_sender_manager.h"

#include <cstdlib>


// IDA_EA: 0x808BE58
// IDA_NAME: _ZN15PISenderManagerC2Ev
// DEMANGLED: PISenderManager::PISenderManager(void)
PISenderManager::PISenderManager() : key_(39) {}

// IDA_EA: 0x808BE6E
// IDA_NAME: _ZN15PISenderManager18MakeHashForSendingEPhjb
// DEMANGLED: PISenderManager::MakeHashForSending(unsigned char *,unsigned int,bool)
int PISenderManager::MakeHashForSending(unsigned char* payload,
                                        unsigned int length, bool reset) {
    if (reset) {
        key_ = 39;
    }

    const unsigned char previous_key = key_;
    const unsigned char next_key = static_cast<unsigned char>(std::rand());
    key_ = next_key;

    const unsigned char hash =
        length != 0 ? static_cast<unsigned char>(
                          MakeHash(payload, length, previous_key))
                    : static_cast<unsigned char>(std::rand());

    const unsigned int packed =
        (16u * (next_key & 0x30u)) |
        (4u * (next_key & 0x0Cu)) |
        ((next_key & 0x03u) << 12) |
        (16u * (hash & 0xC0u)) |
        ((hash & 0x30u) >> 2) |
        ((hash & 0x0Cu) << 12) |
        (static_cast<unsigned char>(hash << 6)) |
        (next_key >> 6);

    return static_cast<int>((static_cast<unsigned short>(packed) << 8) |
                            static_cast<unsigned char>(std::rand()));
}

// IDA_EA: 0x808C0EC
// IDA_NAME: _ZN15PISenderManagerD2Ev
// DEMANGLED: PISenderManager::~PISenderManager()
// IDA_EA: 0x808C112
// IDA_NAME: _ZN15PISenderManagerD0Ev
// DEMANGLED: PISenderManager::~PISenderManager() deleting destructor
// NOTE: The original deleting destructor calls this destructor body and then
// operator delete; C++ emits that wrapper from the virtual destructor.
PISenderManager::~PISenderManager() {}

// IDA_EA: 0x808C174
// IDA_NAME: _Z15DoSuperFastHashPhjRj
// DEMANGLED: DoSuperFastHash(unsigned char *,unsigned int,unsigned int &)
int DoSuperFastHash(unsigned char* data, unsigned int length,
                    unsigned int& hash) {
    hash = 0;
    unsigned int value = length;
    if (length == 0 || data == 0) {
        return 0;
    }

    const unsigned int remaining = length & 3u;
    for (unsigned int count = length >> 2; count != 0; --count) {
        const unsigned int low = (static_cast<unsigned int>(data[1]) << 8) +
                                 data[0] + value;
        const unsigned int mixed =
            low ^
            (((static_cast<unsigned int>(data[3]) << 8) + data[2]) << 11) ^
            (low << 16);
        data += 4;
        value = (mixed >> 11) + mixed;
    }

    switch (remaining) {
        case 1u: {
            const unsigned int mixed = ((data[0] + value) << 10) ^
                                       (data[0] + value);
            value = (mixed >> 1) + mixed;
            break;
        }
        case 2u: {
            const unsigned int low = (static_cast<unsigned int>(data[1]) << 8) +
                                     data[0] + value;
            const unsigned int mixed = (low << 11) ^ low;
            value = (mixed >> 17) + mixed;
            break;
        }
        case 3u: {
            const unsigned int low = (static_cast<unsigned int>(data[1]) << 8) +
                                     data[0] + value;
            const unsigned int mixed =
                (static_cast<unsigned int>(data[2]) << 18) ^ (low << 16) ^ low;
            value = (mixed >> 11) + mixed;
            break;
        }
    }

    const unsigned int first = (((8u * value) ^ value) >> 5) +
                               ((8u * value) ^ value);
    const unsigned int second = (((16u * first) ^ first) >> 17) +
                                ((16u * first) ^ first);
    hash = (((second << 25) ^ second) >> 6) + ((second << 25) ^ second);
    return 1;
}

// IDA_EA: 0x808C311
// IDA_NAME: _Z8MakeHashPhjh
// DEMANGLED: MakeHash(unsigned char *,unsigned int,unsigned char)
char MakeHash(unsigned char* data, unsigned int length,
              unsigned char previous_key) {
    unsigned int hash = 0;
    DoSuperFastHash(data, length, hash);
    return static_cast<char>(previous_key ^ ((hash >> 16) & 0xFFu) ^
                             ((hash >> 8) & 0xFFu) ^ (hash & 0xFFu) ^
                             ((hash >> 24) & 0xFFu));
}

// IDA_EA: 0x808CA7A
// IDA_NAME: _Z15DoSuperFastHashPcjRj
// DEMANGLED: DoSuperFastHash(char *,unsigned int,unsigned int &)
int DoSuperFastHash(char* data, unsigned int length, unsigned int& hash) {
    hash = 0;
    unsigned int value = length;
    if (length == 0 || data == 0) {
        return 0;
    }

    const unsigned int remaining = length & 3u;
    for (unsigned int count = length >> 2; count != 0; --count) {
        const unsigned int low =
            (static_cast<unsigned int>(
                 static_cast<unsigned char>(data[1])) << 8) +
            static_cast<unsigned char>(data[0]) + value;
        const unsigned int mixed =
            low ^
            (((static_cast<unsigned int>(
                   static_cast<unsigned char>(data[3])) << 8) +
              static_cast<unsigned char>(data[2])) << 11) ^
            (low << 16);
        data += 4;
        value = (mixed >> 11) + mixed;
    }

    switch (remaining) {
        case 1u: {
            const unsigned int byte = static_cast<unsigned int>(*data);
            const unsigned int mixed = ((byte + value) << 10) ^ (byte + value);
            value = (mixed >> 1) + mixed;
            break;
        }
        case 2u: {
            const unsigned int low =
                (static_cast<unsigned int>(
                     static_cast<unsigned char>(data[1])) << 8) +
                static_cast<unsigned char>(data[0]) + value;
            const unsigned int mixed = (low << 11) ^ low;
            value = (mixed >> 17) + mixed;
            break;
        }
        case 3u: {
            const unsigned int low =
                (static_cast<unsigned int>(
                     static_cast<unsigned char>(data[1])) << 8) +
                static_cast<unsigned char>(data[0]) + value;
            const unsigned int signed_byte =
                static_cast<unsigned int>(static_cast<int>(data[2]));
            const unsigned int mixed = (signed_byte << 18) ^ (low << 16) ^ low;
            value = (mixed >> 11) + mixed;
            break;
        }
    }

    const unsigned int first = (((8u * value) ^ value) >> 5) +
                               ((8u * value) ^ value);
    const unsigned int second = (((16u * first) ^ first) >> 17) +
                                ((16u * first) ^ first);
    hash = (((second << 25) ^ second) >> 6) + ((second << 25) ^ second);
    return 1;
}
