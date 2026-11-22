#pragma once

#include "ai.h"
#include "pokemon.h"
#include "report.h"
#include "team.h"

#define GYM_COUNT    8
#define LEAGUE_COUNT 5

typedef struct
{
    const char *leader;
    const char *badge;
    const char *specialty;
    enum AiDifficulty difficulty;
    const char *pokemon_names[4];
    int team_size;
} GymLeader;

void display_gyms(int badges_mask);
int challenge_gym(int index, const Pokemon *pokedex, int pokedex_count, Team *player_team,
                  int *badges_mask, BattleReport *report);
int challenge_league(const Pokemon *pokedex, int pokedex_count, Team *player_team,
                     int badges_mask, BattleReport *report);
int find_gym_index(const char *arg);
