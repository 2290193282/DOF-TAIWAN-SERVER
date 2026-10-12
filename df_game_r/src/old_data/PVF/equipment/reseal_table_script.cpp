#include "old_data/PVF/equipment/reseal_table_script.h"

#include "old_data/PVF/common/rdar_script.h"
#include "infra/log/format/trace.h"

#include <string>


// IDA_EA: 0x890096F
// IDA_NAME: _Z17importResealTablePKcP14reseal_table_t
// DEMANGLED: importResealTable(char const*,reseal_table_t*)
int importResealTable(const char* filename, reseal_table_t* reseal_table) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    while (ScanType(tag, true) == 1) {
        if (tag == "[rarity weight]") {
            for (int index = 0; index <= 5; ++index) {
                bool success = false;
                const int weight = ScanInt(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importResealTable",
                        0xA9D,
                        "importResealTable - invalid rarity table",
                        filename);
                }
                reseal_table->rarity_weights[index] = weight;
            }
        } else if (tag == "[extended rarity weight]") {
            for (int index = 0; index <= 5; ++index) {
                bool success = false;
                const int weight = ScanInt(&success);
                if (!success) {
                    LogManager::logFormat(
                        0,
                        "../RDARScriptEtcParameter.cpp",
                        "importResealTable",
                        0xAAF,
                        "importResealTable - invalid extended rarity table",
                        filename);
                }
                reseal_table->extended_rarity_weights[index] = weight;
            }
        } else if (tag == "[grade]") {
            while (1) {
                bool success = false;
                const unsigned int grade =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                reseal_table->grade_costs.push_back(grade);
            }
        }
    }

    return 1;
}
