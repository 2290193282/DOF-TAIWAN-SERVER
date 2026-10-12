#include "common/types/code_page.h"

#include "common/manager/dnff_lib.h"

#include <string>

namespace {

std::string gClientEncoding;
std::string gScriptEncoding;
std::string gDatabaseEncoding;

}  // namespace

// IDA_EA: 0x8108D88
// IDA_NAME: _ZN8CodePage12initCodePageEv
// DEMANGLED: CodePage::initCodePage(void)
int CodePage::initCodePage() {
    gClientEncoding = "UTF-8";
    gScriptEncoding = "BIG5";
    gDatabaseEncoding = "UTF-8";
    return 1;
}

// IDA_EA: 0x8108DCC
// IDA_NAME: _ZN8CodePage6scriptEv
// DEMANGLED: CodePage::script(void)
const char* CodePage::script() {
    return gScriptEncoding.c_str();
}

// IDA_EA: 0x8108DE0
// IDA_NAME: _ZN8CodePage8databaseEv
// DEMANGLED: CodePage::database(void)
const char* CodePage::database() {
    return gDatabaseEncoding.c_str();
}

// IDA_EA: 0x8108DF4
// IDA_NAME: _ZN8CodePage6clientEv
// DEMANGLED: CodePage::client(void)
const char* CodePage::client() {
    return gClientEncoding.c_str();
}

// IDA_EA: 0x8108E08
// IDA_NAME: _ZN8CodePage15script2DatabaseEPcS0_
// DEMANGLED: CodePage::script2Database(char *,char *)
int CodePage::script2Database(char* input, char* outbuf) {
    if (gDatabaseEncoding.compare(gScriptEncoding) == 0) {
        return 1;
    }
    return DNFFLib::CharacSetSwitch(gScriptEncoding.c_str(),
                                    gDatabaseEncoding.c_str(),
                                    input,
                                    outbuf);
}

// IDA_EA: 0x8108E6D
// IDA_NAME: _ZN8CodePage15database2ScriptEPcS0_
// DEMANGLED: CodePage::database2Script(char *,char *)
int CodePage::database2Script(char* input, char* outbuf) {
    if (gDatabaseEncoding.compare(gScriptEncoding) == 0) {
        return 1;
    }
    return DNFFLib::CharacSetSwitch(gDatabaseEncoding.c_str(),
                                    gScriptEncoding.c_str(),
                                    input,
                                    outbuf);
}

// IDA_EA: 0x8108ED2
// IDA_NAME: _ZN8CodePage15client2DatabaseEPcS0_
// DEMANGLED: CodePage::client2Database(char *,char *)
int CodePage::client2Database(char* input, char* outbuf) {
    if (gDatabaseEncoding.compare(gClientEncoding) == 0) {
        return 1;
    }
    return DNFFLib::CharacSetSwitch(gClientEncoding.c_str(),
                                    gDatabaseEncoding.c_str(),
                                    input,
                                    outbuf);
}

// IDA_EA: 0x8108F37
// IDA_NAME: _ZN8CodePage15database2ClientEPcS0_
// DEMANGLED: CodePage::database2Client(char *,char *)
int CodePage::database2Client(char* input, char* outbuf) {
    if (gDatabaseEncoding.compare(gClientEncoding) == 0) {
        return 1;
    }
    return DNFFLib::CharacSetSwitch(gDatabaseEncoding.c_str(),
                                    gClientEncoding.c_str(),
                                    input,
                                    outbuf);
}
