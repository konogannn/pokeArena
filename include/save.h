#pragma once

#include "pokemon.h"
#include "team.h"

int save_team(const char *path, const Team *team);
int load_team(const char *path, const Pokemon *pokedex, int pokedex_count, Team *out_team);
