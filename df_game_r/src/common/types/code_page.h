#ifndef OLDSRV_GLOBAL_CODE_PAGE_H_
#define OLDSRV_GLOBAL_CODE_PAGE_H_

// IDA unit: 99_misc_global/CodePage.
// IDA_EA: initCodePage @0x8108D88; conversions @0x8108E6D..0x8108F37.
class CodePage {
public:
    static int initCodePage();
    static const char* script();
    static const char* database();
    static const char* client();
    static int script2Database(char* input, char* outbuf);
    static int database2Script(char* input, char* outbuf);
    static int client2Database(char* input, char* outbuf);
    static int database2Client(char* input, char* outbuf);
};

#endif  // OLDSRV_GLOBAL_CODE_PAGE_H_
