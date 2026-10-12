#include "old_data/PVF/aicharacter/st_ai_info_script.h"

// IDA_EA: 0x8366F48
// IDA_NAME: _ZN14STAIInfoScriptC2Ev
// DEMANGLED: STAIInfoScript::STAIInfoScript(void)
STAIInfoScript::STAIInfoScript()
    : value_04(0),
      value_08(0),
      value_0C(),
      string_lists(),
      values_4C(),
      int_lists(),
      value_1AC(0),
      values_1B0(),
      values_1BC(),
      values_1C8(),
      values_1D4(),
      values_1E0(),
      value_1EC(0),
      value_1F0(0),
      value_1F4(0),
      value_1F8(0),
      value_1FC(0),
      value_200(0) {
    clear();
}

// IDA_EA: 0x83671A4
// IDA_NAME: _ZN14STAIInfoScriptD2Ev
// DEMANGLED: STAIInfoScript::~STAIInfoScript()
STAIInfoScript::~STAIInfoScript() {
    destroy();
}

// IDA_EA: 0x83673B6
// IDA_NAME: _ZN14STAIInfoScriptD0Ev
// DEMANGLED: STAIInfoScript::~STAIInfoScript() deleting destructor
// The virtual destructor above emits the deleting-destructor ABI entry.

// IDA_EA: 0x83673D4
// IDA_NAME: _ZN14STAIInfoScript5clearEv
// DEMANGLED: STAIInfoScript::clear(void)
STAIInfoScript* STAIInfoScript::clear() {
    value_04 = 1000;
    value_08 = 1000;
    values_1B0.clear();
    values_1BC.clear();
    values_1C8.clear();
    values_1D4.clear();
    values_1E0.clear();
    value_0C.clear();

    for (int index = 0; index <= 4; ++index) {
        string_lists[index].clear();
    }

    for (int index = 0; index <= 21; ++index) {
        values_4C[index] = (index == 0) ? 50 : 0;
        int_lists[index].clear();
    }

    value_1AC = 100;
    value_1EC = 100;
    value_1F0 = 100;
    value_1F4 = 5000;
    value_1F8 = 1500;
    value_1FC = 1500;
    value_200 = 1;
    return this;
}

// IDA_EA: 0x836753A
// IDA_NAME: _ZN14STAIInfoScript7destroyEv
// DEMANGLED: STAIInfoScript::destroy(void)
bool STAIInfoScript::destroy() {
    for (int list_index = 0;; ++list_index) {
        const bool result = list_index <= 4;
        if (list_index > 4) {
            return result;
        }

        std::vector<std::string*>& list = string_lists[list_index];
        for (std::size_t index = 0; index < list.size(); ++index) {
            if (list[index]) {
                delete list[index];
                list[index] = 0;
            }
        }
        list.clear();
    }
}

// IDA_EA: 0x8A38BC6
// IDA_NAME: _ZN14STAIInfoScriptC2ERKS_
// DEMANGLED: STAIInfoScript::STAIInfoScript(STAIInfoScript const&)
STAIInfoScript::STAIInfoScript(const STAIInfoScript& other)
    : value_04(0),
      value_08(0),
      value_0C(),
      string_lists(),
      values_4C(),
      int_lists(),
      value_1AC(0),
      values_1B0(),
      values_1BC(),
      values_1C8(),
      values_1D4(),
      values_1E0(),
      value_1EC(0),
      value_1F0(0),
      value_1F4(0),
      value_1F8(0),
      value_1FC(0),
      value_200(0) {
    clear();
    *this = other;
}

// IDA_EA: 0x8A38E34
// IDA_NAME: _ZN14STAIInfoScriptaSERKS_
// DEMANGLED: STAIInfoScript::operator=(STAIInfoScript const&)
STAIInfoScript& STAIInfoScript::operator=(const STAIInfoScript& other) {
    destroy();

    value_04 = other.value_04;
    value_08 = other.value_08;
    value_0C = other.value_0C;

    for (int list_index = 0; list_index <= 4; ++list_index) {
        for (std::size_t index = 0;
             index < other.string_lists[list_index].size();
             ++index) {
            std::string* copy =
                new std::string(*other.string_lists[list_index][index]);
            string_lists[list_index].push_back(copy);
        }
    }

    for (int index = 0; index <= 21; ++index) {
        values_4C[index] = other.values_4C[index];
        int_lists[index] = other.int_lists[index];
    }

    value_1AC = other.value_1AC;
    values_1B0 = other.values_1B0;
    values_1BC = other.values_1BC;
    values_1C8 = other.values_1C8;
    values_1D4 = other.values_1D4;
    values_1E0 = other.values_1E0;
    value_1F8 = other.value_1F8;
    value_1FC = other.value_1FC;
    value_1EC = other.value_1EC;
    value_1F0 = other.value_1F0;
    value_1F4 = other.value_1F4;
    value_200 = other.value_200;
    return *this;
}
