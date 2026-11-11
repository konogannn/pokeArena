#pragma once

#include "moves.h"
#include "pokemon.h"
#include "team.h"

void run_game_loop(const Pokemon *pokedex, int pokedex_count,
                   const Move *moves, int move_count, Team *team);
