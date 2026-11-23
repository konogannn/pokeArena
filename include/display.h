#pragma once

#include "moves.h"
#include "pokemon.h"
#include "team.h"

void display_move(const Move *move);
void display_pokemon(const Pokemon *pokemon);
void display_pokemon_stats(const Pokemon *pokemon);
void display_team(Team team);
void animate_hp_change(const Pokemon *pokemon, int old_hp, int new_hp);

int ui_read_int_choice(const char *prompt, int min, int max);
int ui_prompt_battle_action(int turn, const Pokemon *player, const Pokemon *opponent);
int ui_prompt_battle_action_for(const char *label, int turn,
                                const Pokemon *self, const Pokemon *foe);
int ui_prompt_move(const Pokemon *pokemon);
int ui_prompt_switch(const Team *team, int active_idx);
int ui_prompt_switch_for(const char *label, const Team *team, int active_idx);
