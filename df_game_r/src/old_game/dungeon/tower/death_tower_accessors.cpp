#include "old_game/pvp/battlefield/battle_field.h"

// IDA_EA: 0x82345FA
// IDA_NAME: _ZN8WongWork11CDeathTower6CStage17getStageClearTimeEv
// DEMANGLED: WongWork::CDeathTower::CStage::getStageClearTime(void)
int WongWork::CDeathTower::CStage::getStageClearTime() {
    return stage_end_tick_ - stage_start_tick_;
}

// IDA_EA: 0x823461E
// IDA_NAME: _ZN8WongWork11CDeathTower9getCStageEv
// DEMANGLED: WongWork::CDeathTower::getCStage(void)
WongWork::CDeathTower::CStage* WongWork::CDeathTower::getCStage() {
    return &stage_;
}
