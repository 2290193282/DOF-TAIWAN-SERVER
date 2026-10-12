#include "old_game/economy/equipment/equipment_ani_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "infra/log/format/trace.h"

#include <utility>

std::map<std::string, STEquipmentAniScript>* g_mapEquipmentAniScript = 0;

namespace {

struct EquipmentAniTagEntry {
    const char* name;
    int value;
};

const EquipmentAniTagEntry kEquipmentAniTags[] = {
    {"[LAYER]", 12610},
    {"[waiting motion]", 12613},
    {"[move motion]", 12617},
    {"[sit motion]", 12621},
    {"[damage motion 1]", 12625},
    {"[damage motion 2]", 12629},
    {"[down motion]", 12633},
    {"[overturn motion]", 12637},
    {"[jump motion]", 12641},
    {"[jumpattack motion]", 12645},
    {"[rest motion]", 12649},
    {"[throw motion 1-1]", 12653},
    {"[throw motion 1-2]", 12657},
    {"[throw motion 2-1]", 12661},
    {"[throw motion 2-2]", 12665},
    {"[throw motion 3-1]", 12669},
    {"[throw motion 3-2]", 12673},
    {"[throw motion 4-1]", 12677},
    {"[throw motion 4-2]", 12681},
    {"[dash motion]", 12685},
    {"[dashattack motion]", 12689},
    {"[getitem motion]", 12693},
    {"[buff motion]", 12697},
    {"[back motion]", 12701},
    {"[attack motion]", 12705},
    {"[etc motion]", 12711},
};

}  // namespace

// IDA_EA: 0x89C2FC4
// IDA_NAME: _ZN19GrowthEquipmentInfoC2Ev
// DEMANGLED: GrowthEquipmentInfo::GrowthEquipmentInfo(void)
GrowthEquipmentInfo::GrowthEquipmentInfo() : ani_name() {
    clear();
}

// IDA_EA: 0x89C300C
// IDA_NAME: _ZN19GrowthEquipmentInfo5clearEv
// DEMANGLED: GrowthEquipmentInfo::clear(void)
GrowthEquipmentInfo* GrowthEquipmentInfo::clear() {
    item_index = 0;
    equipment_slot = -1;
    required_level = 0;
    grow_type = 0;
    unknown_10 = 0;
    ani_name.clear();
    return this;
}

// IDA_EA: 0x89C4FBE
// IDA_NAME: _ZN19GrowthEquipmentInfoD2Ev
// DEMANGLED: GrowthEquipmentInfo::~GrowthEquipmentInfo()
GrowthEquipmentInfo::~GrowthEquipmentInfo() {}

// IDA_EA: 0x89CA12C
// IDA_NAME: _ZN19GrowthEquipmentInfoC2ERKS_
// DEMANGLED: GrowthEquipmentInfo::GrowthEquipmentInfo(GrowthEquipmentInfo const&)
GrowthEquipmentInfo::GrowthEquipmentInfo(const GrowthEquipmentInfo& other)
    : item_index(other.item_index),
      equipment_slot(other.equipment_slot),
      required_level(other.required_level),
      grow_type(other.grow_type),
      unknown_10(other.unknown_10),
      ani_name(other.ani_name) {}

// IDA_EA: 0x89CA20A
// IDA_NAME: _ZN19GrowthEquipmentInfoaSERKS_
// DEMANGLED: GrowthEquipmentInfo::operator=(GrowthEquipmentInfo const&)
GrowthEquipmentInfo& GrowthEquipmentInfo::operator=(
    const GrowthEquipmentInfo& other) {
    item_index = other.item_index;
    equipment_slot = other.equipment_slot;
    required_level = other.required_level;
    grow_type = other.grow_type;
    unknown_10 = other.unknown_10;
    ani_name = other.ani_name;
    return *this;
}

// IDA_EA: 0x898CA36
// IDA_NAME: _ZN20STEquipmentAniScriptC2Ev
// DEMANGLED: STEquipmentAniScript::STEquipmentAniScript(void)
STEquipmentAniScript::STEquipmentAniScript()
    : layer(0),
      waiting_motion(),
      move_motion(),
      sit_motion(),
      damage_motion(),
      down_motion(),
      overturn_motion(),
      jump_motion(),
      jumpattack_motion(),
      rest_motion(),
      throw_motion_first(),
      throw_motion_second(),
      dash_motion(),
      dashattack_motion(),
      getitem_motion(),
      buff_motion(),
      back_motion(),
      reserved_motion(),
      attack_motion(),
      etc_motion() {
    clear();
}

// IDA_EA: 0x898CE62
// IDA_NAME: _ZN20STEquipmentAniScript5clearEv
// DEMANGLED: STEquipmentAniScript::clear(void)
std::string* STEquipmentAniScript::clear() {
    layer = 0;
    waiting_motion = "";
    move_motion = "";
    sit_motion = "";
    for (int index = 0; index <= 1; ++index) {
        damage_motion[index] = "";
    }
    down_motion = "";
    overturn_motion = "";
    jump_motion = "";
    jumpattack_motion = "";
    rest_motion = "";
    for (int index = 0; index <= 3; ++index) {
        throw_motion_first[index] = "";
        throw_motion_second[index] = "";
    }
    dash_motion = "";
    dashattack_motion = "";
    getitem_motion = "";
    buff_motion = "";
    attack_motion.clear();
    etc_motion.clear();
    reserved_motion = "";
    return &reserved_motion;
}

// IDA_EA: 0x89C4C26
// IDA_NAME: _ZN20STEquipmentAniScriptD2Ev
// DEMANGLED: STEquipmentAniScript::~STEquipmentAniScript()
STEquipmentAniScript::~STEquipmentAniScript() {}

// IDA_EA: 0x89C7820
// IDA_NAME: _ZN20STEquipmentAniScriptC2ERKS_
// DEMANGLED: STEquipmentAniScript::STEquipmentAniScript(STEquipmentAniScript const&)
STEquipmentAniScript::STEquipmentAniScript(
    const STEquipmentAniScript& other)
    : layer(other.layer),
      waiting_motion(other.waiting_motion),
      move_motion(other.move_motion),
      sit_motion(other.sit_motion),
      damage_motion(),
      down_motion(other.down_motion),
      overturn_motion(other.overturn_motion),
      jump_motion(other.jump_motion),
      jumpattack_motion(other.jumpattack_motion),
      rest_motion(other.rest_motion),
      throw_motion_first(),
      throw_motion_second(),
      dash_motion(other.dash_motion),
      dashattack_motion(other.dashattack_motion),
      getitem_motion(other.getitem_motion),
      buff_motion(other.buff_motion),
      back_motion(other.back_motion),
      reserved_motion(other.reserved_motion),
      attack_motion(other.attack_motion),
      etc_motion(other.etc_motion) {
    for (int index = 0; index <= 1; ++index) {
        damage_motion[index] = other.damage_motion[index];
    }
    for (int index = 0; index <= 3; ++index) {
        throw_motion_first[index] = other.throw_motion_first[index];
        throw_motion_second[index] = other.throw_motion_second[index];
    }
}

// IDA_EA: 0x89C9BC4
// IDA_NAME: _ZN20STEquipmentAniScriptaSERKS_
// DEMANGLED: STEquipmentAniScript::operator=(STEquipmentAniScript const&)
STEquipmentAniScript& STEquipmentAniScript::operator=(
    const STEquipmentAniScript& other) {
    layer = other.layer;
    waiting_motion = other.waiting_motion;
    move_motion = other.move_motion;
    sit_motion = other.sit_motion;
    for (int index = 0; index <= 1; ++index) {
        damage_motion[index] = other.damage_motion[index];
    }
    down_motion = other.down_motion;
    overturn_motion = other.overturn_motion;
    jump_motion = other.jump_motion;
    jumpattack_motion = other.jumpattack_motion;
    rest_motion = other.rest_motion;
    for (int index = 0; index <= 3; ++index) {
        throw_motion_first[index] = other.throw_motion_first[index];
        throw_motion_second[index] = other.throw_motion_second[index];
    }
    dash_motion = other.dash_motion;
    dashattack_motion = other.dashattack_motion;
    getitem_motion = other.getitem_motion;
    buff_motion = other.buff_motion;
    back_motion = other.back_motion;
    reserved_motion = other.reserved_motion;
    attack_motion = other.attack_motion;
    etc_motion = other.etc_motion;
    return *this;
}

// IDA_EA: 0x898D03E
// IDA_NAME: _ZN22EquipmentAniInfoScriptC2Ev
// DEMANGLED: EquipmentAniInfoScript::EquipmentAniInfoScript(void)
EquipmentAniInfoScript::EquipmentAniInfoScript()
    : base_ani(),
      unknown_0C(0),
      unknown_10(0),
      unknown_14(0),
      pad_15(),
      layer_ani(),
      unknown_24(0),
      unknown_28(0),
      unknown_2C(0),
      unknown_30(0),
      unknown_34(0),
      unknown_38(0),
      unknown_3C(0),
      string_list(),
      unknown_string_4C(),
      growth_infos(),
      layer_lists() {
    clear();
}

// IDA_EA: 0x898D142
// IDA_NAME: _ZN22EquipmentAniInfoScript5clearEv
// DEMANGLED: EquipmentAniInfoScript::clear(void)
int EquipmentAniInfoScript::clear() {
    base_ani.clear();
    unknown_0C = -1;
    unknown_10 = -1;
    layer_ani.clear();
    unknown_24 = -1;
    unknown_28 = 1;
    unknown_2C = 0;
    unknown_30 = -1;
    unknown_34 = -1;
    unknown_string_4C.clear();
    unknown_38 = 0;
    unknown_3C = 0;
    string_list.clear();
    unknown_14 = 0;
    growth_infos.clear();
    layer_lists.clear();
    return 0;
}

// IDA_EA: 0x89C586A
// IDA_NAME: _ZN22EquipmentAniInfoScriptD2Ev
// DEMANGLED: EquipmentAniInfoScript::~EquipmentAniInfoScript()
EquipmentAniInfoScript::~EquipmentAniInfoScript() {}

// IDA_EA: 0x89C8A46
// IDA_NAME: _ZN22EquipmentAniInfoScriptC2ERKS_
// DEMANGLED: EquipmentAniInfoScript::EquipmentAniInfoScript(EquipmentAniInfoScript const&)
EquipmentAniInfoScript::EquipmentAniInfoScript(
    const EquipmentAniInfoScript& other)
    : base_ani(other.base_ani),
      unknown_0C(other.unknown_0C),
      unknown_10(other.unknown_10),
      unknown_14(other.unknown_14),
      pad_15(),
      layer_ani(other.layer_ani),
      unknown_24(other.unknown_24),
      unknown_28(other.unknown_28),
      unknown_2C(other.unknown_2C),
      unknown_30(other.unknown_30),
      unknown_34(other.unknown_34),
      unknown_38(other.unknown_38),
      unknown_3C(other.unknown_3C),
      string_list(other.string_list),
      unknown_string_4C(other.unknown_string_4C),
      growth_infos(other.growth_infos),
      layer_lists(other.layer_lists) {
    pad_15[0] = other.pad_15[0];
    pad_15[1] = other.pad_15[1];
    pad_15[2] = other.pad_15[2];
}

// IDA_EA: 0x89C0586
// IDA_NAME: _Z25InitMapEquipmentAniScriptv
// DEMANGLED: InitMapEquipmentAniScript(void)
int InitMapEquipmentAniScript() {
    g_mapEquipmentAniScript =
        new std::map<std::string, STEquipmentAniScript>();
    return ImportEquipmentAniScript(g_mapEquipmentAniScript);
}

// IDA_EA: 0x89C05DD
// IDA_NAME: _Z24getMapEquipmentAniScriptv
// DEMANGLED: getMapEquipmentAniScript(void)
std::map<std::string, STEquipmentAniScript>* getMapEquipmentAniScript() {
    if (!g_mapEquipmentAniScript) {
        InitMapEquipmentAniScript();
    }
    return g_mapEquipmentAniScript;
}

// IDA_EA: 0x89C05F8
// IDA_NAME: _Z24ImportEquipmentAniScriptPSt3mapISs20STEquipmentAniScriptSt4lessISsESaISt4pairIKSsS0_EEE
// DEMANGLED: ImportEquipmentAniScript(std::map<std::string,STEquipmentAniScript>*)
int ImportEquipmentAniScript(
    std::map<std::string, STEquipmentAniScript>* scripts) {
    if (!loadRDARScriptFile("", "Etc/equipmentAniScript.etc")) {
        LogManager::logFormat(
            0,
            "../RDARScriptItem.cpp",
            "ImportEquipmentAniScript",
            0x3116,
            "CRITICAL ERROR : EquipmentAniScript!");
        return 0;
    }

    std::string tag;
    std::vector<std::string> filenames;
    if (ScanType(tag, true) != 1) {
        return 0;
    }

    if (tag == "[equipment ani script]") {
        std::string filename;
        while (ScanStr(&filename) == 1 &&
               tag != "[/equipment ani script]") {
            filenames.push_back(filename);
        }
    }

    for (std::vector<std::string>::iterator it = filenames.begin();
         it != filenames.end();
         ++it) {
        if (!loadRDARScriptFile("", it->c_str())) {
            return 0;
        }

        std::string section_tag;
        std::string scanned;
        STEquipmentAniScript script;
        while (ScanType(section_tag, true) == 1) {
            int section = -1;
            for (unsigned int index = 0;
                 index < sizeof(kEquipmentAniTags) /
                             sizeof(kEquipmentAniTags[0]);
                 ++index) {
                if (section_tag == kEquipmentAniTags[index].name) {
                    section = kEquipmentAniTags[index].value;
                    break;
                }
            }

            switch (section) {
                case 12610:
                    script.layer = ScanInt(static_cast<bool*>(0));
                    break;
                case 12613:
                    ScanStr(&scanned);
                    script.waiting_motion = scanned;
                    break;
                case 12617:
                    ScanStr(&scanned);
                    script.move_motion = scanned;
                    break;
                case 12621:
                    ScanStr(&scanned);
                    script.sit_motion = scanned;
                    break;
                case 12625:
                    ScanStr(&scanned);
                    script.damage_motion[0] = scanned;
                    break;
                case 12629:
                    ScanStr(&scanned);
                    script.damage_motion[1] = scanned;
                    break;
                case 12633:
                    ScanStr(&scanned);
                    script.down_motion = scanned;
                    break;
                case 12637:
                    ScanStr(&scanned);
                    script.overturn_motion = scanned;
                    break;
                case 12641:
                    ScanStr(&scanned);
                    script.jump_motion = scanned;
                    break;
                case 12645:
                    ScanStr(&scanned);
                    script.jumpattack_motion = scanned;
                    break;
                case 12649:
                    ScanStr(&scanned);
                    script.rest_motion = scanned;
                    break;
                case 12653:
                    ScanStr(&scanned);
                    script.throw_motion_first[0] = scanned;
                    break;
                case 12657:
                    ScanStr(&scanned);
                    script.throw_motion_second[0] = scanned;
                    break;
                case 12661:
                    ScanStr(&scanned);
                    script.throw_motion_first[1] = scanned;
                    break;
                case 12665:
                    ScanStr(&scanned);
                    script.throw_motion_second[1] = scanned;
                    break;
                case 12669:
                    ScanStr(&scanned);
                    script.throw_motion_first[2] = scanned;
                    break;
                case 12673:
                    ScanStr(&scanned);
                    script.throw_motion_second[2] = scanned;
                    break;
                case 12677:
                    ScanStr(&scanned);
                    script.throw_motion_first[3] = scanned;
                    break;
                case 12681:
                    ScanStr(&scanned);
                    script.throw_motion_second[3] = scanned;
                    break;
                case 12685:
                    ScanStr(&scanned);
                    script.dash_motion = scanned;
                    break;
                case 12689:
                    ScanStr(&scanned);
                    script.dashattack_motion = scanned;
                    break;
                case 12693:
                    ScanStr(&scanned);
                    script.getitem_motion = scanned;
                    break;
                case 12697:
                    ScanStr(&scanned);
                    script.buff_motion = scanned;
                    break;
                case 12701:
                    ScanStr(&scanned);
                    script.back_motion = scanned;
                    break;
                case 12705:
                    while (ScanStr(&scanned)) {
                        script.attack_motion.push_back(scanned);
                    }
                    break;
                case 12711:
                    while (ScanStr(&scanned)) {
                        script.etc_motion.push_back(scanned);
                    }
                    break;
                default:
                    break;
            }
        }

        scripts->insert(std::make_pair(*it, script));
    }

    return 1;
}

typedef char GrowthEquipmentInfo_Size_Must_Be_0x18[
    (sizeof(GrowthEquipmentInfo) == 0x18) ? 1 : -1];
typedef char STEquipmentAniScript_Size_Must_Be_0x7C[
    (sizeof(STEquipmentAniScript) == 0x7C) ? 1 : -1];
typedef char EquipmentAniInfoScript_Size_Must_Be_0x68[
    (sizeof(EquipmentAniInfoScript) == 0x68) ? 1 : -1];
typedef char EquipmentAniMap_Size_Must_Be_0x18[
    (sizeof(std::map<std::string, STEquipmentAniScript>) == 0x18) ? 1 : -1];
