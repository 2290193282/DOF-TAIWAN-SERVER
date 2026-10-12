#include "old_data/PVF/etc/avatar_roulette_script.h"

#include "common/config.h"
#include "old_data/PVF/common/rdar_script.h"

#include <utility>
#include <vector>


// IDA_EA: 0x89115B1
// IDA_NAME: _Z30importAvatarRouletteListDetailR14AvatarRouletteRSs17ENUM_CHARACTERJOB
// DEMANGLED: importAvatarRouletteListDetail(AvatarRoulette&,std::string&,ENUM_CHARACTERJOB)
int importAvatarRouletteListDetail(AvatarRoulette& roulette,
                                   std::string& tag,
                                   ENUM_CHARACTERJOB job) {
    std::vector<std::pair<unsigned long, int> > avatar_items;
    int item_no = 0;
    bool is_type = false;
    bool success = false;

    for (int index = 0; index <= 8; ++index) {
        item_no = ScanTypeOrInt(is_type, tag, success, true);
        if (is_type) {
            break;
        }
        avatar_items.push_back(
            std::make_pair(static_cast<unsigned long>(item_no), 3));
    }

    if (!avatar_items.empty()) {
        const int job_index = static_cast<int>(job);
        roulette.avatar_lists_by_job[job_index].push_back(avatar_items);
        item_no = ScanInt(static_cast<bool*>(0));
        roulette.sticker_lists_by_job[job_index].push_back(item_no);
    }

    return 1;
}

// IDA_EA: 0x89116EB
// IDA_NAME: _Z24importAvatarRouletteListPcR14AvatarRoulette
// DEMANGLED: importAvatarRouletteList(char*,AvatarRoulette&)
int importAvatarRouletteList(char* filename, AvatarRoulette& roulette) {
    if (!loadRDARScriptFile("", filename)) {
        return 0;
    }

    std::string tag;
    bool is_type = false;
    bool success = false;
    roulette.clear();

    while (1) {
        const int value = ScanTypeOrInt(is_type, tag, success, true);
        if (!success && !value) {
            break;
        }

        if (tag == "[avatar coin info]") {
            roulette.avatar_coin_item_no = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[rare clone avatar rate]") {
            roulette.rare_clone_avatar_rate = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[rare avatar rate]") {
            roulette.rare_avatar_rate = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[upper avatar rate]") {
            roulette.upper_avatar_rate = ScanInt(static_cast<bool*>(0));
        } else if (tag == "[normal avatar rate]") {
            roulette.normal_avatar_rate = ScanInt(static_cast<bool*>(0));
        } else {
            if (tag == "[swordman]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobSwordman));
                } while (tag != "[/swordman]");
            }
            if (tag == "[fighter]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobFighter));
                } while (tag != "[/fighter]");
            }
            if (tag == "[gunner]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobGunner));
                } while (tag != "[/gunner]");
            }
            if (tag == "[mage]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobMage));
                } while (tag != "[/mage]");
            }
            if (tag == "[priest]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobPriest));
                } while (tag != "[/priest]");
            }
            if (tag == "[at_gunner]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobAtGunner));
                } while (tag != "[/at_gunner]");
            }
            if (tag == "[thief]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobThief));
                } while (tag != "[/thief]");
            }
            if (tag == "[at_fighter]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobAtFighter));
                } while (tag != "[/at_fighter]");
            }
            if (tag == "[at_mage]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobAtMage));
                } while (tag != "[/at_mage]");
            }
            if (tag == "[DemonicSwordman]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobDemonicSwordman));
                } while (tag != "[/DemonicSwordman]");
            }
            if (tag == "[creator mage]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobCreatorMage));
                } while (tag != "[/creator mage]");
            }
            if (tag == "[at_swordman]") {
                do {
                    importAvatarRouletteListDetail(
                        roulette, tag,
                        static_cast<ENUM_CHARACTERJOB>(
                            oldsrv::config::kCharacterJobAtSwordman));
                } while (tag != "[/at_swordman]");
            }
        }
    }

    return 1;
}
