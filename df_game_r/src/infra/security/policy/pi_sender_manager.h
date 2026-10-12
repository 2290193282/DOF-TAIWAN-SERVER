#ifndef OLDSRV_SECURITY_PI_SENDER_MANAGER_H_
#define OLDSRV_SECURITY_PI_SENDER_MANAGER_H_


// IDA: DoSuperFastHash(unsigned char *, unsigned int, unsigned int &) @0x808C174.
int DoSuperFastHash(unsigned char* data, unsigned int length,
                    unsigned int& hash);

// IDA: MakeHash(unsigned char *, unsigned int, unsigned char) @0x808C311.
char MakeHash(unsigned char* data, unsigned int length,
              unsigned char previous_key);

// IDA: DoSuperFastHash(char *, unsigned int, unsigned int &) @0x808CA7A.
int DoSuperFastHash(char* data, unsigned int length, unsigned int& hash);

// IDA: PISenderManager object stores its rolling key byte at old offset +0x04.
class PISenderManager {
public:
    PISenderManager();

    // IDA: destructor @0x808C0EC and deleting destructor @0x808C112.
    virtual ~PISenderManager();

    // IDA: PISenderManager::MakeHashForSending @0x808BE6E.
    int MakeHashForSending(unsigned char* payload, unsigned int length,
                           bool reset);

private:
    unsigned char key_;
};


#endif  // OLDSRV_SECURITY_PI_SENDER_MANAGER_H_
