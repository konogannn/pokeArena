#pragma once

#include "moves.h"
#include "pokemon.h"
#include "team.h"

void run_game_loop(Pokemon **pokedex, int *pokedex_count,
                   Move **moves, int *move_count, Team *team);
