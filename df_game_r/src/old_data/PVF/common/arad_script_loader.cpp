#include "old_data/PVF/common/arad_script_loader.h"

#include <string.h>


typedef char AradScriptLoader_Size_Must_Be_0x04[
    (sizeof(Arad_ScriptLoader) == 0x04) ? 1 : -1];

// IDA_EA: 0x8188B42
// IDA_NAME: _ZN17Arad_ScriptLoaderC2Ev
// DEMANGLED: Arad_ScriptLoader::Arad_ScriptLoader(void)
Arad_ScriptLoader::Arad_ScriptLoader() {}

// IDA_EA: 0x81A1EEC
// IDA_NAME: _ZN17Arad_ScriptLoader4openEPKc
// DEMANGLED: Arad_ScriptLoader::open(char const*)
int Arad_ScriptLoader::open(const char* filename) {
    if (loadRDARScriptFile("", filename) != true) {
        return 16;
    }

    const int input_size = g_dnfLex.getInputStreamSize();
    char* buffer = new char[input_size + 1];
    memset(buffer, 0, input_size + 1);

    std::istream* input = g_dnfLex.getInputStream();
    input->read(buffer, input_size);
    buffer[input_size] = static_cast<char>(-1);

    const int result = parsing(buffer);
    delete[] buffer;
    if (result) {
        return result;
    }
    return 0;
}

// IDA_EA: 0x81A4054
// IDA_NAME: _ZN17Arad_ScriptLoader7parsingEPc
// DEMANGLED: Arad_ScriptLoader::parsing(char *)
int Arad_ScriptLoader::parsing(char* buffer) {
    (void)buffer;
    return 1;
}
