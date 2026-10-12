#include "old_data/PVF/passiveobject/passive_object_script_types.h"

#include <string>

#include "common/manager/character_script_public_abi.h"
#include "old_data/PVF/common/rdar_script.h"

extern char g_szPassiveObjectScriptBaseDirectory[256];
extern STScriptFileList g_stPassiveObjectScriptFileList;

// External original provider: isValidExtension @ 0x8A3C67D
// (`_Z16isValidExtensionRKSsS0_`), restored by the monster PVF provider.
// Keep this narrow ABI declaration here: including monster_script.h would
// import unrelated gameplay/IPG layout assertions into this PVF-only parser.
int isValidExtension(const std::string& filename, const std::string& extension)
    __asm__("_Z16isValidExtensionRKSsS0_");

namespace {

bool read_int(int32_t& value) { return ScanInt(&value) == 1; }
bool read_string(std::string& value) { return ScanStr(&value) == 1; }
bool read_float(float& value) { return ScanFloat(&value) == 1; }

bool read_int_list(std::vector<int>& values, const char* closing_tag) {
    values.clear();
    for (;;) {
        bool is_type = false;
        bool success = false;
        std::string token;
        const int value = ScanTypeOrInt(is_type, token, success, true);
        if (!success) return false;
        if (is_type) return token == closing_tag;
        values.push_back(value);
    }
}

bool read_string_list(std::vector<std::string>& values, const char* closing_tag,
                      bool animation_only) {
    values.clear();
    for (;;) {
        bool is_type = false;
        std::string value;
        if (ScanTypeOrStr(is_type, value, true) != 1) return false;
        if (is_type) return value == closing_tag;
        if (animation_only && isValidExtension(value, std::string(".ani")) != 1)
            return false;
        values.push_back(value);
    }
}

bool read_destroy_condition(STPassiveObjectScript& script) {
    std::string value;
    if (!read_string(value)) return false;
    if (value == "[on end of animation]") {
        script.destroy.destroy_type = 0;
    } else if (value == "[z pos equal zero]") {
        script.destroy.destroy_type = 1;
    } else if (value == "[time limite]") {
        script.destroy.destroy_type = 2;
        if (!read_int(script.destroy.field_3c)) return false;
    } else if (value == "[not destroy and z pos equal zero]") {
        script.destroy.destroy_type = 3;
    } else if (value == "[parent dead]") {
        script.destroy.destroy_type = 4;
    } else {
        script.destroy.destroy_type = -1;
    }
    return true;
}

bool read_destroy_action(STPassiveObjectScript& script) {
    std::string value;
    if (!read_string(value)) return false;
    if (value == "[only destroy]") script.destroy.destroy_delay = 0;
    else if (value == "[object create after destroy]") script.destroy.destroy_delay = 1;
    else script.destroy.destroy_delay = -1;
    return true;
}

bool read_destroy_on_attack(STPassiveObjectScript& script) {
    std::string value;
    if (!read_string(value)) return false;
    if (value == "[only destroy]") script.destroy.field_40 = 0;
    else if (value == "[object create after destroy]") script.destroy.field_40 = 1;
    else script.destroy.field_40 = -1;
    return true;
}

bool read_counted_ints(std::vector<int>& values) {
    int32_t count = 0;
    if (!read_int(count) || count < 0) return false;
    values.reserve(values.size() + static_cast<size_t>(count));
    for (int32_t i = 0; i != count; ++i) {
        int32_t value = 0;
        if (!read_int(value)) return false;
        values.push_back(value);
    }
    return true;
}

bool read_counted_strings(std::vector<std::string>& values) {
    int32_t count = 0;
    if (!read_int(count) || count < 0) return false;
    values.reserve(values.size() + static_cast<size_t>(count));
    for (int32_t i = 0; i != count; ++i) {
        std::string value;
        if (!read_string(value)) return false;
        values.push_back(value);
    }
    return true;
}

bool read_sound_category(STPassiveObjectDestroy& destroy) {
    destroy.sound_files.assign(5, std::string());
    destroy.value_list.assign(5, 0);
    for (;;) {
        bool is_type = false;
        std::string tag;
        if (ScanTypeOrStr(is_type, tag, true) != 1) return false;
        if (is_type) return tag == "[/sound category]";

        int index = -1;
        if (tag == "[after object create]") index = 0;
        else if (tag == "[on frame]") index = 1;
        else if (tag == "[on attack]") index = 2;
        else if (tag == "[on damage]") index = 3;
        else if (tag == "[on destroyobject]") index = 4;
        else return false;

        if (!read_string(destroy.sound_files[index]) ||
            !read_int(destroy.value_list[index]))
            return false;
        // The old ABI carries the frame position only for the [on frame]
        // entry, at STPassiveObjectDestroy + 0x38.
        if (index == 1 && !read_int(destroy.field_38)) return false;
    }
}

bool read_destroy_section(STPassiveObjectScript& script) {
    for (;;) {
        bool is_type = false;
        std::string tag;
        if (ScanTypeOrStr(is_type, tag, true) != 1) return false;
        if (is_type) return tag == "[/object destroy condition]";

        if (tag == "[destroy condition]") {
            if (!read_destroy_condition(script)) return false;
        } else if (tag == "[destroy action]") {
            if (!read_destroy_action(script)) return false;
        } else if (tag == "[on attack]") {
            if (!read_destroy_on_attack(script)) return false;
        } else if (tag == "[add object index]") {
            if (!read_counted_ints(script.destroy.item_indices)) return false;
        } else if (tag == "[add object particle file]") {
            if (!read_counted_strings(script.destroy.effect_files)) return false;
        } else if (tag == "[add object index on attack]") {
            if (!read_counted_ints(script.destroy.additional_values)) return false;
        } else if (tag == "[sound category]") {
            if (!read_sound_category(script.destroy)) return false;
        } else {
            return false;
        }
    }
}

bool read_add_object_effect(STPassiveObjectScript& script) {
    for (;;) {
        bool is_type = false;
        std::string filename;
        if (ScanTypeOrStr(is_type, filename, true) != 1) return false;
        if (is_type) return filename == "[/add object effect]";
        int32_t value = 0;
        if (!read_int(value)) return false;
        script.add_effect.effect_files.push_back(filename);
        script.add_effect.effect_values.push_back(value);
    }
}

bool read_add_particles(STPassiveObjectScript& script) {
    for (;;) {
        std::string tag;
        if (ScanType(tag, true) != 1) return false;
        if (tag == "[/add particles]") return true;
        if (tag != "[particle]") return false;

        STPassiveObjectAdditionalParticleInfo* particle =
            new STPassiveObjectAdditionalParticleInfo();
        bool completed = false;
        for (;;) {
            if (ScanType(tag, true) != 1) break;
            if (tag == "[/particle]") {
                completed = true;
                break;
            }
            if (tag == "[file]") {
                if (!read_string(particle->file_name)) break;
            } else if (tag == "[create delay]") {
                if (!read_int(particle->create_delay)) break;
            } else if (tag == "[create sound]") {
                if (!read_string(particle->create_sound)) break;
            } else if (tag == "[follow parent]") {
                if (!read_int(particle->follow_parent[0]) ||
                    !read_int(particle->follow_parent[1]) ||
                    !read_int(particle->follow_parent[2]))
                    break;
            } else if (tag == "[loop]") {
                int32_t loop = 0;
                if (!read_int(loop)) break;
                particle->loop = loop != 0;
            } else {
                break;
            }
        }
        if (!completed) {
            delete particle;
            return false;
        }
        script.additional_particles.push_back(particle);
    }
}

bool read_homing(STPassiveObjectScript& script) {
    for (;;) {
        std::string tag;
        if (ScanType(tag, true) != 1) return false;
        if (tag == "[/homing]") return true;
        if (tag == "[homing use]") {
            int32_t value = 0;
            if (!read_int(value)) return false;
            script.field_9c = value != 0;
        } else if (tag == "[homing follow]") {
            std::string target;
            if (!read_string(target)) return false;
            if (target == "[ME]") script.field_a0 = 0;
            else if (target == "[ENEMY]") script.field_a0 = 1;
            else if (target == "[CHARACTER]") {
                script.field_a0 = 3;
                if (!read_int(script.field_a4)) return false;
            } else if (target == "[MONSTER]") {
                script.field_a0 = 2;
                if (!read_int(script.field_a4)) return false;
            } else {
                return false;
            }
        } else if (tag == "[homing velocity]") {
            if (!read_float(script.field_a8) || !read_float(script.field_ac)) return false;
        } else if (tag == "[homing check gap]") {
            if (!read_int(script.field_b0)) return false;
        } else if (tag == "[sync animation rotation]") {
            int32_t value = 0;
            if (!read_int(value)) return false;
            script.state_b4 = value != 0;
        } else if (tag == "[max rotation]") {
            if (!read_float(script.field_b8)) return false;
        } else if (tag == "[init rotation]") {
            if (!read_float(script.field_bc)) return false;
        } else if (tag == "[diff rotation]") {
            if (!read_float(script.field_c0)) return false;
        } else if (tag == "[homing time]") {
            if (!read_int(script.field_c4)) return false;
        } else if (tag == "[homing z axis]") {
            if (!read_int(script.field_c8)) return false;
        } else if (tag == "[homing direction]") {
            if (!read_int(script.field_cc)) return false;
        } else if (tag == "[homing keep distance]") {
            if (!read_int(script.field_d0)) return false;
        } else if (tag == "[sync target move]") {
            script.state_d4 = 1;
        } else if (tag == "[straight homing]") {
            script.state_d5 = 1;
        } else {
            return false;
        }
    }
}

bool read_speed_control_objects(STPassiveObjectScript& script) {
    script.speed_control_objects.clear();
    for (;;) {
        std::string tag;
        if (ScanType(tag, true) != 1) return false;
        if (tag == "[/speed control passive objects]") return true;
        if (tag != "[passive object]") return false;

        SpeedCtrlPassiveObjInfo object;
        for (;;) {
            if (ScanType(tag, true) != 1) return false;
            if (tag == "[/passive object]") break;
            if (tag == "[index]") {
                if (!read_int(object.passive_object_index)) return false;
            } else if (tag == "[speed]") {
                std::string speed;
                if (!read_string(speed)) return false;
                if (speed == "[up]") object.enabled = 1;
                else if (speed == "[down]") object.enabled = 0;
                else return false;
            } else {
                return false;
            }
        }
        if (object.passive_object_index == -1) return false;
        script.speed_control_objects.push_back(object);
    }
}

bool read_generator_objects(std::vector<PassiveObjGeneratorObject>& objects) {
    objects.clear();
    for (;;) {
        std::string tag;
        if (ScanType(tag, true) != 1) return false;
        if (tag == "[/objects]") return true;
        if (tag != "[object]") return false;

        PassiveObjGeneratorObject object;
        for (;;) {
            if (ScanType(tag, true) != 1) return false;
            if (tag == "[/object]") break;
            if (tag == "[index]") {
                if (!read_int(object.object_index)) return false;
            } else if (tag == "[probability]") {
                if (!read_float(object.probability)) return false;
            } else if (tag == "[warning]") {
                if (!read_string(object.warning_name)) return false;
                if (object.warning_name != "none" &&
                    object.warning_name != "danger" &&
                    object.warning_name != "recovery")
                    return false;
            } else {
                return false;
            }
        }
        objects.push_back(object);
    }
}

bool read_generator_patterns(std::vector<PassiveObjGeneratorPattern>& patterns) {
    patterns.clear();
    for (;;) {
        std::string tag;
        if (ScanType(tag, true) != 1) return false;
        if (tag == "[/patterns]") return true;
        if (tag != "[pattern]") return false;

        PassiveObjGeneratorPattern pattern;
        for (;;) {
            if (ScanType(tag, true) != 1) return false;
            if (tag == "[/pattern]") break;
            if (tag == "[name]") {
                if (!read_string(pattern.pattern_name)) return false;
            } else if (tag == "[interval]") {
                if (!read_int(pattern.second_value)) return false;
            } else if (tag == "[columns]") {
                if (!read_int(pattern.first_value)) return false;
            } else if (tag == "[data]") {
                pattern.values.clear();
                for (;;) {
                    bool is_type = false;
                    bool success = false;
                    std::string closing;
                    const int value = ScanTypeOrInt(is_type, closing, success, true);
                    if (!success) return false;
                    if (is_type) {
                        if (closing != "[/data]") return false;
                        break;
                    }
                    if (value < 0 || value > 255) return false;
                    pattern.values.push_back(static_cast<uint8_t>(value));
                }
            } else {
                return false;
            }
        }
        patterns.push_back(pattern);
    }
}

bool read_generator_sequences(std::vector<PassiveObjGeneratorSequence>& sequences) {
    sequences.clear();
    for (;;) {
        std::string tag;
        if (ScanType(tag, true) != 1) return false;
        if (tag == "[/sequences]") return true;
        if (tag != "[pattern]") return false;

        PassiveObjGeneratorSequence sequence;
        bool is_type = false;
        if (ScanTypeOrStr(is_type, sequence.sequence_name, true) != 1 ||
            is_type || !read_int(sequence.value))
            return false;
        std::string closing;
        if (ScanTypeOrStr(is_type, closing, true) != 1 || !is_type ||
            closing != "[/pattern]")
            return false;
        sequences.push_back(sequence);
    }
}

bool read_warning(STPassiveObjectScript& script) {
    for (;;) {
        std::string tag;
        if (ScanType(tag, true) != 1) return false;
        if (tag == "[/warning]") return true;
        WarningDistance* distance = 0;
        if (tag == "[min distance]") distance = &script.warning_distance_near;
        else if (tag == "[max distance]") distance = &script.warning_distance_far;
        else return false;
        if (!read_int(distance->first) || !read_int(distance->second) ||
            !read_int(distance->third))
            return false;
    }
}

bool read_sniper_type(STPassiveObjectScript& script) {
    PassiveIObjSniperInfo info;
    for (;;) {
        std::string tag;
        if (ScanType(tag, true) != 1) return false;
        if (tag == "[/sniper type]") {
            script.sniper_info = info;
            return true;
        }
        if (tag == "[accelerate]") {
            if (!read_int(info.field_30)) return false;
        } else if (tag == "[init velocity]") {
            if (!read_int(info.field_34)) return false;
        } else if (tag == "[shot area]") {
            for (int i = 0; i != 6; ++i)
                if (!read_int(info.fields_44_58[i])) return false;
        } else if (tag == "[shot distance]") {
            info.field_38 = 1;
            if (!read_int(info.field_3c)) return false;
        } else if (tag == "[move check time]") {
            if (!read_int(info.field_5c)) return false;
        } else if (tag == "[invoke check time]") {
            if (!read_int(info.field_60)) return false;
        } else if (tag == "[invoke delay time]") {
            if (!read_int(info.field_64)) return false;
        } else if (tag == "[wait time]") {
            if (!read_int(info.field_68)) return false;
        } else if (tag == "[shake area]") {
            for (int i = 0; i != 3; ++i)
                if (!read_int(info.fields_74_7c[i])) return false;
        } else if (tag == "[invoke time]") {
            if (!read_int(info.field_6c)) return false;
        } else if (tag == "[invoke count]") {
            if (!read_int(info.field_70)) return false;
        } else {
            return false;
        }
    }
}

bool read_random_item_fairy(STPassiveObjectScript& script) {
    RandomItemFairy entry;
    for (;;) {
        std::string tag;
        if (ScanType(tag, true) != 1) return false;
        if (tag == "[/random item fairy]") return true;
        if (tag == "[entry]") {
            entry.clear();
        } else if (tag == "[passiveobject]") {
            if (!read_int(entry.item_index) || !read_int(entry.item_count) ||
                !read_int(entry.probability) || !read_int(entry.condition))
                return false;
        } else if (tag == "[racing dungeon probability]") {
            // CLIENT/PVF_EVIDENCE: every 0828 racing entry owns exactly three
            // grade weights.  This field belongs to the current entry, not to
            // the outer random-item-fairy collection.
            entry.racing_dungeon_probabilities.clear();
            for (int index = 0; index != 3; ++index) {
                int32_t probability = 0;
                if (!read_int(probability)) return false;
                entry.racing_dungeon_probabilities.push_back(probability);
            }
        } else if (tag == "[/entry]") {
            script.random_item_fairies.push_back(entry);
        } else {
            return false;
        }
    }
}

bool read_party_hp_max_rates(STPassiveObjectScript& script) {
    script.party_hp_max_rates.clear();
    for (;;) {
        bool success = false;
        const int party_count = ScanInt(&success);
        if (!success) {
            return g_dnfLex.getLastToken() == DNF_TOK_TYPE &&
                   std::string(g_dnfLex.getType()) ==
                       "[/party count for hp max rate]";
        }
        int32_t rate = 0;
        if (!read_int(rate)) return false;
        script.party_hp_max_rates.push_back(
            std::make_pair(static_cast<int32_t>(party_count), rate));
    }
}

}  // namespace

// IDA_EA: 0x8A2C51A
// IDA_NAME: _Z25ImportPassiveObjectScriptP21STPassiveObjectScriptPKcb
// DEMANGLED: ImportPassiveObjectScript(STPassiveObjectScript *,char const*,bool)
int ImportPassiveObjectScript(STPassiveObjectScript* script,
                              const char* filename,
                              bool resolve_file_index) {
    if (!loadRDARScriptFile(g_szPassiveObjectScriptBaseDirectory, filename))
        return 0;

    // Exact original entry order: reset all three value regions before the
    // path is recorded and before the first ScanType call.
    script->clear();
    script->destroy.clear();
    script->add_effect.clear();
    getDirectoryPath(script->script_filename, "%s%s",
                     g_szPassiveObjectScriptBaseDirectory, filename);

    for (;;) {
        std::string tag;
        if (ScanType(tag, true) != 1) {
            // Old LABEL_1374 performs this lookup on the end-of-stream path
            // regardless of the third argument.
            script->object_index =
                g_stPassiveObjectScriptFileList.FindIndexByFullScan(filename);
            return 1;
        }
        if (tag == "[name]") {
            if (!read_string(script->object_name)) return 0;
            script->resource_name = script->object_name;
            // The true branch is a name/index probe: the original returns as
            // soon as the name has been consumed and does not parse the rest
            // of the script body.
            if (resolve_file_index) {
                script->object_index =
                    g_stPassiveObjectScriptFileList.FindIndexByFullScan(filename);
                return 1;
            }
        } else if (tag == "[width]") {
            if (!read_int(script->fields_0c_24[0]) ||
                !read_int(script->fields_0c_24[1])) return 0;
        } else if (tag == "[floating height]") {
            if (!read_int(script->fields_0c_24[2])) return 0;
        } else if (tag == "[layer]") {
            std::string layer;
            if (!read_string(layer)) return 0;
            script->fields_0c_24[3] = getDrawLayer(layer.c_str());
        } else if (tag == "[layer level]") {
            if (!read_int(script->field_15c)) return 0;
        } else if (tag == "[burning death path]") {
            if (!read_string(script->speed_control_name)) return 0;
        } else if (tag == "[is hp by difficulty]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->state_158 = value != 0;
        } else if (tag == "[hp gage]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->state_159 = value == 1;
        } else if (tag == "[pass type]") {
            std::string value;
            if (!read_string(value)) return 0;
            static const char* const kPassTypes[] = {
                "[pass all]", "[pass only float type]", "[do not pass]",
                "[pass only red team]", "[pass only blue team]",
                "[pass only character team]", "[pass only monster team]",
                "[pass only character]"};
            int pass_type = -1;
            for (int i = 0; i != 8; ++i)
                if (value == kPassTypes[i]) pass_type = i;
            if (pass_type < 0) return 0;
            script->fields_0c_24[4] = pass_type;
        } else if (tag == "[piercing power]") {
            if (!read_int(script->fields_0c_24[5])) return 0;
        } else if (tag == "[piercing tolerance]") {
            if (!read_int(script->fields_0c_24[6])) return 0;
        } else if (tag == "[category]") {
            std::vector<std::string> categories;
            if (!read_string_list(categories, "[/category]", false)) return 0;
            script->spawn_values.clear();
            for (size_t i = 0; i != categories.size(); ++i) {
                const int category = getObjectCategory(categories[i]);
                if (category != 44) script->spawn_values.push_back(category);
            }
        } else if (tag == "[basic motion]") {
            if (!read_string(script->display_name) ||
                isValidExtension(script->display_name, std::string(".ani")) != 1)
                return 0;
        } else if (tag == "[etc motion]") {
            if (!read_string_list(script->first_string_values,
                                  "[/etc motion]", true)) return 0;
        } else if (tag == "[basic action]") {
            if (!read_string(script->first_effect)) return 0;
        } else if (tag == "[last action]") {
            if (!read_string(script->second_effect)) return 0;
        } else if (tag == "[etc action]") {
            if (!read_string_list(script->second_string_values,
                                  "[/etc action]", false)) return 0;
        } else if (tag == "[under gravity]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->state_58 = value == 1;
        } else if (tag == "[vanish on move collision]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->state_60 = value == 1;
            script->state_61 = 1;
        } else if (tag == "[trap]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->state_5a = value == 1;
        } else if (tag == "[allow friend hit]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->state_62 = value == 1;
        } else if (tag == "[team]") {
            if (!read_int(script->field_5c)) return 0;
        } else if (tag == "[destroy particle]") {
            if (!read_string(script->third_effect)) return 0;
        } else if (tag == "[attack info]") {
            if (!read_string(script->fourth_effect)) return 0;
        } else if (tag == "[etc attack info]") {
            if (!read_string_list(script->third_string_values,
                                  "[/etc attack info]", false)) return 0;
        } else if (tag == "[int data]") {
            if (!read_int_list(script->additional_spawn_values,
                               "[/int data]")) return 0;
        } else if (tag == "[string data]") {
            if (!read_string_list(script->fourth_string_values,
                                  "[/string data]", false)) return 0;
        } else if (tag == "[quest type]") {
            std::string value;
            if (!read_string(value)) return 0;
            if (value == "[none]") script->field_94 = 0;
            else if (value == "[must be destroyed]") script->field_94 = 1;
            else if (value == "[chaos object]") script->field_94 = 2;
            else return 0;
        } else if (tag == "[hp max]") {
            if (!read_int(script->field_98)) return 0;
        } else if (tag == "[blocking area]") {
            if (!read_int(script->blocking_area)) return 0;
        } else if (tag == "[apply parent ability category]") {
            script->apply_parent_ability_category = 1;
        } else if (tag == "[party count for hp max rate]") {
            if (!read_party_hp_max_rates(*script)) return 0;
        } else if (tag == "[damage absorb]") {
            if (!read_float(script->damage_absorb)) return 0;
        } else if (tag == "[show itembox mark in minimap]") {
            if (!read_int(script->minimap_marker_mode) ||
                !read_string(script->minimap_marker_image)) return 0;
        } else if (tag == "[absolute zpos]") {
            if (!read_int(script->absolute_zpos)) return 0;
        } else if (tag == "[hp destroy]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->state_59 = value == 1;
        } else if (tag == "[object destroy condition]") {
            if (!read_destroy_section(*script)) return 0;
        } else if (tag == "[sound category]") {
            // This is a top-level block in the official 0627 server PVF and
            // in the 0828 TimeSpiral trap object.  It owns the same physical
            // STPassiveObjectDestroy sound fields as the optional nested
            // form, so parse it through the canonical block reader.
            if (!read_sound_category(script->destroy)) return 0;
        } else if (tag == "[add object effect]") {
            if (!read_add_object_effect(*script)) return 0;
        } else if (tag == "[add particles]") {
            if (!read_add_particles(*script)) return 0;
        } else if (tag == "[homing]") {
            if (!read_homing(*script)) return 0;
        } else if (tag == "[for tournament]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->for_tournament = value != 0;
        } else if (tag == "[for school]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->for_school = value != 0;
        } else if (tag == "[die when parent die]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->die_when_parent_dies = value != 0;
        } else if (tag == "[transparent on meet player]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->transparent_on_meet_player = value != 0;
        } else if (tag == "[pvp room transparent value for observer]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            // The original store updates only the signed low half at +0x150;
            // reserved_152 therefore remains untouched.
            script->pvp_observer_transparency =
                static_cast<int16_t>(value);
        } else if (tag == "[create sound]") {
            if (!read_string(script->particle_name)) return 0;
        } else if (tag == "[can beat index]") {
            if (!read_int_list(script->particle_values, "[/can beat index]")) return 0;
        } else if (tag == "[is blocking obj]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->state_16c = value != 0;
        } else if (tag == "[apply physics]") {
            if (!read_float(script->field_170)) return 0;
        } else if (tag == "[following immortal]") {
            int32_t value = 0;
            if (!read_int(value)) return 0;
            script->field_174 = value == 1 ? 2 : 1;
        } else if (tag == "[follow parent]") {
            std::string value;
            if (!read_string(value)) return 0;
            if (value == "[path gate]") script->field_178 = 1;
            else if (value == "[parent]") script->field_178 = 2;
            else if (value == "[from parent]") script->field_178 = 3;
            else if (value == "[keep position]") script->field_178 = 4;
            else return 0;
        } else if (tag == "[speed control passive objects]") {
            if (!read_speed_control_objects(*script)) return 0;
        } else if (tag == "[objects]") {
            if (!read_generator_objects(script->generator_objects)) return 0;
        } else if (tag == "[patterns]") {
            if (!read_generator_patterns(script->generator_patterns)) return 0;
        } else if (tag == "[sequences]") {
            if (!read_generator_sequences(script->generator_sequences)) return 0;
        } else if (tag == "[warning]") {
            if (!read_warning(*script)) return 0;
        } else if (tag == "[sniper type]") {
            if (!read_sniper_type(*script)) return 0;
        } else if (tag == "[passive object type]") {
            std::string value;
            if (!read_string(value)) return 0;
            // The original maps the two named modes to 0/1 and preserves 2
            // as its default for all other strings.
            if (value == "[missile fairy]") script->sniper_info.sniper_type = 0;
            else if (value == "[random item fairy]") script->sniper_info.sniper_type = 1;
            else script->sniper_info.sniper_type = 2;
        } else if (tag == "[random item fairy]") {
            if (!read_random_item_fairy(*script)) return 0;
        } else if (tag == "[passive object icon]") {
            script->state_280 = 1;
            if (!read_string(script->final_name) ||
                !read_int(script->passive_object_icon_x) ||
                !read_int(script->passive_object_icon_y))
                return 0;
        } else if (tag == "[is step on]") {
            // The original consumes this legacy flag but does not retain it in
            // STPassiveObjectScript.  Consume exactly one integer so later
            // tags remain aligned with the RDAR token stream.
            int32_t ignored = 0;
            if (!read_int(ignored)) return 0;
        } else {
            // The original parser rejects unrecognised tags.  Keep that
            // behavior while the remaining nested sections are restored;
            // silently skipping a block would desynchronise the RDAR stream.
            return 0;
        }
    }
}
