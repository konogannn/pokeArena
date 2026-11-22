#pragma once

#include "battle.h"

#define REPORT_MAX_BATTLES 64

typedef struct
{
    char label[64];
    char outcome[16];
    int turns;
    int damage_dealt;
    int damage_taken;
    int kos_inflicted;
    int kos_suffered;
} BattleRecord;

typedef struct
{
    BattleRecord records[REPORT_MAX_BATTLES];
    int total_battles;
    int wins;
    int losses;
    int forfeits;
    int total_turns;
    int total_damage_dealt;
    int total_damage_taken;
    int total_kos_inflicted;
    int total_kos_suffered;
} BattleReport;

void report_init(BattleReport *report);
void report_record_battle(BattleReport *report, const char *label, const Battle *battle);
int report_write_file(const char *path, const BattleReport *report);
