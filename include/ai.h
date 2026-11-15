#pragma once

#include "moves.h"
#include "pokemon.h"
#include "team.h"

enum AiDifficulty
{
    AI_EASY = 1,
    AI_MEDIUM = 2,
    AI_HARD = 3
};

Team ai_generate_team(const Pokemon *pokedex, int pokedex_count, int count);
Move ai_choose_move(const Pokemon *opponent, const Pokemon *player, enum AiDifficulty difficulty);
int ai_choose_switch(const Team *opponent_team, const Pokemon *player,
                     enum AiDifficulty difficulty);
int ai_should_switch(const Team *opponent_team, int active_idx, const Pokemon *player,
                     enum AiDifficulty difficulty);
const char *ai_difficulty_to_string(enum AiDifficulty difficulty);
