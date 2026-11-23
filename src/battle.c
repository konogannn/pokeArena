#include "battle.h"
#include "ai.h"
#include "color.h"
#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_pokemon_alive(Pokemon pokemon)
{
    return pokemon.hp > 0;
}

int is_team_defeated(Team team)
{
    for (int i = 0; i < team.count; i++)
    {
        if (is_pokemon_alive(team.pokemons[i]))
        {
            return 0;
        }
    }
    return 1;
}

int calculate_damage(Pokemon attacker, Pokemon defender, Move move)
{
    double mult = type_effectiveness(move.type, defender.type);
    if (mult == 0.0)
    {
        return 0;
    }

    double stab = (move.type == attacker.type) ? 1.5 : 1.0;
    int def = defender.defense > 0 ? defender.defense : 1;
    double raw = ((double)move.power * ((double)attacker.attack / (double)def) * 0.5 + 2.0) *
                 mult * stab;
    int damage = (int)raw;

    return damage < 1 ? 1 : damage;
}

static void apply_heal(Pokemon *target, int percent)
{
    int old_hp = target->hp;
    int heal = (target->max_hp * percent) / 100;
    if (heal < 1)
    {
        heal = 1;
    }
    target->hp += heal;
    if (target->hp > target->max_hp)
    {
        target->hp = target->max_hp;
    }
    print_slow(ANSI_GREEN, "  Restoring HP...\n", 15);
    animate_hp_change(target, old_hp, target->hp);
}

static void apply_move_effect(Pokemon *attacker, Pokemon *defender, Move move)
{
    if (move.effect == EFFECT_NONE || rand() % 100 >= move.effect_chance)
    {
        return;
    }

    if (move.effect == EFFECT_HEAL && is_pokemon_alive(*attacker))
    {
        apply_heal(attacker, move.effect_chance);
        return;
    }

    if (!is_pokemon_alive(*defender) || defender->status != STATUS_NONE)
    {
        return;
    }

    if (move.effect == EFFECT_POISON)
    {
        defender->status = STATUS_POISON;
        print_colored(ANSI_MAGENTA, "  ");
        printf("%s was poisoned!\n", defender->name);
    }
    else if (move.effect == EFFECT_BURN)
    {
        defender->status = STATUS_BURN;
        print_colored(ANSI_RED, "  ");
        printf("%s was burned!\n", defender->name);
    }
    else if (move.effect == EFFECT_PARALYSIS)
    {
        defender->status = STATUS_PARALYSIS;
        print_colored(ANSI_YELLOW, "  ");
        printf("%s is paralyzed!\n", defender->name);
    }
}

static void consume_move_pp(Pokemon *attacker, const char *move_name)
{
    for (int i = 0; i < attacker->move_count; i++)
    {
        if (strcmp(attacker->moves[i].name, move_name) == 0)
        {
            if (attacker->moves[i].pp > 0)
            {
                attacker->moves[i].pp--;
            }
            return;
        }
    }
}

static int execute_attack_tracked(Pokemon *attacker, Pokemon *defender, Move move, int *out_ko)
{
    if (out_ko)
    {
        *out_ko = 0;
    }
    if (!attacker || !defender)
    {
        return 0;
    }

    if (attacker->status == STATUS_PARALYSIS && rand() % 4 == 0)
    {
        printf("%s%s is paralyzed and can't move!%s\n",
               color_code(ANSI_YELLOW), attacker->name, color_code(ANSI_RESET));
        return 0;
    }

    consume_move_pp(attacker, move.name);

    char msg[128];
    snprintf(msg, sizeof(msg), "%s uses %s!\n", attacker->name, move.name);
    print_slow(ANSI_BOLD, msg, 15);
    if (rand() % 100 >= move.accuracy)
    {
        print_slow(ANSI_YELLOW, "  The attack missed!\n", 15);
        return 0;
    }

    if (move.power == 0 && move.effect == EFFECT_HEAL)
    {
        apply_heal(attacker, move.effect_chance);
        return 0;
    }

    double mult = type_effectiveness(move.type, defender->type);
    if (mult == 0.0)
    {
        snprintf(msg, sizeof(msg), "  It doesn't affect %s!\n", defender->name);
        print_slow(ANSI_RED, msg, 15);
        return 0;
    }

    if (move.power == 0)
    {
        if (defender->status != STATUS_NONE)
        {
            printf("  But it failed! (%s already has a status)\n", defender->name);
            return 0;
        }
        apply_move_effect(attacker, defender, move);
        return 0;
    }

    int damage = calculate_damage(*attacker, *defender, move);
    if (move.effect == EFFECT_CRIT && rand() % 100 < move.effect_chance)
    {
        damage *= 2;
        print_slow(ANSI_BOLD, "  A critical hit!\n", 15);
    }

    if (mult > 1.0)
    {
        print_slow(ANSI_GREEN, "  It's super effective!\n", 15);
    }
    else if (mult < 1.0)
    {
        print_slow(ANSI_YELLOW, "  It's not very effective...\n", 15);
    }

    int old_hp = defender->hp;
    defender->hp -= damage;
    if (defender->hp < 0)
    {
        defender->hp = 0;
    }

    printf("  Dealt %d damage!\n", damage);
    animate_hp_change(defender, old_hp, defender->hp);
    if (!is_pokemon_alive(*defender))
    {
        snprintf(msg, sizeof(msg), "  %s fainted!\n", defender->name);
        print_slow(ANSI_RED, msg, 20);
        if (out_ko)
        {
            *out_ko = 1;
        }
        return damage;
    }

    apply_move_effect(attacker, defender, move);
    return damage;
}

void execute_attack(Pokemon *attacker, Pokemon *defender, Move move)
{
    execute_attack_tracked(attacker, defender, move, NULL);
}

static int find_next_alive(const Team *team)
{
    for (int i = 0; i < team->count; i++)
    {
        if (is_pokemon_alive(team->pokemons[i]))
        {
            return i;
        }
    }
    return -1;
}

static int switch_player_pokemon(Battle *battle)
{
    int idx = ui_prompt_switch(&battle->player_team, battle->player_active);
    if (idx < 0)
    {
        return -1;
    }
    battle->player_active = idx;
    return 0;
}

static int player_goes_first(const Pokemon *player, const Pokemon *opponent)
{
    if (player->speed > opponent->speed)
    {
        return 1;
    }
    if (player->speed < opponent->speed)
    {
        return 0;
    }
    return rand() % 2 == 0;
}

static void resolve_attacks(Battle *battle, Pokemon *player, Move player_move,
                            Pokemon *opponent, Move opponent_move)
{
    int ko = 0;

    if (player_goes_first(player, opponent))
    {
        battle->damage_dealt += execute_attack_tracked(player, opponent, player_move, &ko);
        battle->kos_inflicted += ko;
        if (is_pokemon_alive(*opponent))
        {
            battle->damage_taken += execute_attack_tracked(opponent, player, opponent_move, &ko);
            battle->kos_suffered += ko;
        }
    }
    else
    {
        battle->damage_taken += execute_attack_tracked(opponent, player, opponent_move, &ko);
        battle->kos_suffered += ko;
        if (is_pokemon_alive(*player))
        {
            battle->damage_dealt += execute_attack_tracked(player, opponent, player_move, &ko);
            battle->kos_inflicted += ko;
        }
    }
}

static void handle_fainted(Battle *battle)
{
    Pokemon *player = &battle->player_team.pokemons[battle->player_active];
    Pokemon *opponent = &battle->opponent_team.pokemons[battle->opponent_active];

    if (!is_pokemon_alive(*player) && !is_team_defeated(battle->player_team))
    {
        int idx = ui_prompt_switch_for(battle->is_pvp ? "Player 1" : NULL,
                                       &battle->player_team, battle->player_active);
        if (idx < 0)
        {
            battle->forfeited = 1;
            battle->forfeited_player = 1;
        }
        else
        {
            battle->player_active = idx;
        }
    }
    if (!is_pokemon_alive(*opponent) && !is_team_defeated(battle->opponent_team))
    {
        if (battle->is_pvp)
        {
            int idx = ui_prompt_switch_for("Player 2",
                                           &battle->opponent_team, battle->opponent_active);
            if (idx < 0)
            {
                battle->forfeited = 1;
                battle->forfeited_player = 2;
            }
            else
            {
                battle->opponent_active = idx;
            }
        }
        else
        {
            int next = ai_choose_switch(&battle->opponent_team, player,
                                        (enum AiDifficulty)battle->difficulty);
            if (next >= 0)
            {
                battle->opponent_active = next;
                printf("Opponent sends out %s!\n", battle->opponent_team.pokemons[next].name);
            }
        }
    }
}

static void apply_end_turn_status(Pokemon *pokemon, int *dmg_acc, int *ko_acc)
{
    if (!is_pokemon_alive(*pokemon))
    {
        return;
    }
    if (pokemon->status != STATUS_POISON && pokemon->status != STATUS_BURN)
    {
        return;
    }

    int dmg = pokemon->max_hp / 8;
    if (dmg < 1)
    {
        dmg = 1;
    }
    pokemon->hp -= dmg;
    if (pokemon->hp < 0)
    {
        pokemon->hp = 0;
    }
    *dmg_acc += dmg;
    printf("%s is hurt by %s (-%d HP, %d/%d HP)!\n",
           pokemon->name,
           pokemon->status == STATUS_POISON ? "poison" : "burn",
           dmg, pokemon->hp, pokemon->max_hp);
    if (!is_pokemon_alive(*pokemon))
    {
        (*ko_acc)++;
        printf("  %s%s fainted!%s\n", color_code(ANSI_RED), pokemon->name, color_code(ANSI_RESET));
    }
}

static Move move_from_prompt(const Pokemon *pokemon, int move_idx)
{
    if (move_idx == -2)
    {
        Move struggle = {"Struggle", 50, 100, TYPE_NORMAL, EFFECT_NONE, 0, 1, 1};
        return struggle;
    }
    return pokemon->moves[move_idx];
}

static int prompt_pvp_player(const char *label, Team *team, int *active_idx,
                             const Pokemon *foe, Move *out_move)
{
    Pokemon *self = &team->pokemons[*active_idx];
    int action = ui_prompt_battle_action_for(label, 0, self, foe);

    if (action < 0 || action == 3)
    {
        return 3;
    }
    if (action == 2)
    {
        int sw = ui_prompt_switch_for(label, team, *active_idx);
        if (sw < 0)
        {
            return 3;
        }
        *active_idx = sw;
        return 2;
    }

    int move_idx = ui_prompt_move(self);
    if (move_idx == -1)
    {
        return 3;
    }
    *out_move = move_from_prompt(self, move_idx);
    return 1;
}

static void execute_pvp_turn(Battle *battle)
{
    Move p1_move;
    Move p2_move;
    memset(&p1_move, 0, sizeof(p1_move));
    memset(&p2_move, 0, sizeof(p2_move));

    printf("\n%s=== Turn %d ===%s\n", color_code(ANSI_CYAN), battle->turn, color_code(ANSI_RESET));
    int act1 = prompt_pvp_player("Player 1", &battle->player_team, &battle->player_active,
                                 &battle->opponent_team.pokemons[battle->opponent_active],
                                 &p1_move);
    if (act1 == 3)
    {
        battle->forfeited = 1;
        battle->forfeited_player = 1;
        return;
    }

    int act2 = prompt_pvp_player("Player 2", &battle->opponent_team, &battle->opponent_active,
                                 &battle->player_team.pokemons[battle->player_active],
                                 &p2_move);
    if (act2 == 3)
    {
        battle->forfeited = 1;
        battle->forfeited_player = 2;
        return;
    }

    Pokemon *p1 = &battle->player_team.pokemons[battle->player_active];
    Pokemon *p2 = &battle->opponent_team.pokemons[battle->opponent_active];
    int ko = 0;

    if (act1 == 1 && act2 == 1)
    {
        resolve_attacks(battle, p1, p1_move, p2, p2_move);
    }
    else if (act1 == 1)
    {
        battle->damage_dealt += execute_attack_tracked(p1, p2, p1_move, &ko);
        battle->kos_inflicted += ko;
    }
    else if (act2 == 1)
    {
        battle->damage_taken += execute_attack_tracked(p2, p1, p2_move, &ko);
        battle->kos_suffered += ko;
    }

    apply_end_turn_status(&battle->player_team.pokemons[battle->player_active],
                          &battle->damage_taken, &battle->kos_suffered);
    apply_end_turn_status(&battle->opponent_team.pokemons[battle->opponent_active],
                          &battle->damage_dealt, &battle->kos_inflicted);
    handle_fainted(battle);
}

void execute_turn(Battle *battle)
{
    if (!battle)
    {
        return;
    }
    if (battle->is_pvp)
    {
        execute_pvp_turn(battle);
        return;
    }

    Pokemon *player = &battle->player_team.pokemons[battle->player_active];
    Pokemon *opponent = &battle->opponent_team.pokemons[battle->opponent_active];
    int ko = 0;

    int action = ui_prompt_battle_action(battle->turn, player, opponent);
    if (action < 0 || action == 3)
    {
        battle->forfeited = 1;
        battle->forfeited_player = 1;
        return;
    }

    if (action == 1)
    {
        int move_idx = ui_prompt_move(player);
        if (move_idx == -1)
        {
            battle->forfeited = 1;
            battle->forfeited_player = 1;
            return;
        }
        Move chosen = move_from_prompt(player, move_idx);
        int ai_sw = ai_should_switch(&battle->opponent_team, battle->opponent_active,
                                     player, (enum AiDifficulty)battle->difficulty);
        if (ai_sw >= 0)
        {
            printf("Opponent withdrew %s!\n", opponent->name);
            battle->opponent_active = ai_sw;
            opponent = &battle->opponent_team.pokemons[ai_sw];
            printf("Opponent sends out %s!\n", opponent->name);
            battle->damage_dealt += execute_attack_tracked(player, opponent, chosen, &ko);
            battle->kos_inflicted += ko;
        }
        else
        {
            Move ai_move = ai_choose_move(opponent, player, (enum AiDifficulty)battle->difficulty);
            resolve_attacks(battle, player, chosen, opponent, ai_move);
        }
    }
    else if (action == 2)
    {
        if (switch_player_pokemon(battle) == 0)
        {
            player = &battle->player_team.pokemons[battle->player_active];
            Move ai_move = ai_choose_move(opponent, player, (enum AiDifficulty)battle->difficulty);
            battle->damage_taken += execute_attack_tracked(opponent, player, ai_move, &ko);
            battle->kos_suffered += ko;
        }
    }

    apply_end_turn_status(&battle->player_team.pokemons[battle->player_active],
                          &battle->damage_taken, &battle->kos_suffered);
    apply_end_turn_status(&battle->opponent_team.pokemons[battle->opponent_active],
                          &battle->damage_dealt, &battle->kos_inflicted);
    handle_fainted(battle);
}

void start_battle(Battle *battle)
{
    if (!battle)
    {
        return;
    }

    battle->turn = 0;
    battle->forfeited = 0;
    battle->forfeited_player = 0;
    battle->damage_dealt = 0;
    battle->damage_taken = 0;
    battle->kos_inflicted = 0;
    battle->kos_suffered = 0;
    battle->player_active = find_next_alive(&battle->player_team);
    battle->opponent_active = find_next_alive(&battle->opponent_team);

    if (battle->player_active < 0 || battle->opponent_active < 0)
    {
        fprintf(stderr, "battle: both teams need at least one healthy Pokemon\n");
        return;
    }

    if (battle->is_pvp)
    {
        printf("\n=== Local PvP Battle Start! ===\n");
        printf("Player 1 sends out %s!\n", battle->player_team.pokemons[battle->player_active].name);
        printf("Player 2 sends out %s!\n",
               battle->opponent_team.pokemons[battle->opponent_active].name);
    }
    else
    {
        printf("\n=== Battle Start! (AI: %s) ===\n",
               ai_difficulty_to_string((enum AiDifficulty)battle->difficulty));
        printf("You send out %s!\n", battle->player_team.pokemons[battle->player_active].name);
        printf("Opponent sends out %s!\n",
               battle->opponent_team.pokemons[battle->opponent_active].name);
    }

    while (!battle->forfeited &&
           !is_team_defeated(battle->player_team) &&
           !is_team_defeated(battle->opponent_team))
    {
        battle->turn++;
        execute_turn(battle);
    }

    printf("\n=== Battle Over ===\n");
    if (battle->is_pvp)
    {
        if (battle->forfeited)
        {
            printf("Player %d forfeited! Player %d wins!\n",
                   battle->forfeited_player, battle->forfeited_player == 1 ? 2 : 1);
        }
        else if (is_team_defeated(battle->player_team))
        {
            printf("All Player 1 Pokemon fainted! Player 2 wins!\n");
        }
        else
        {
            printf("All Player 2 Pokemon fainted! Player 1 wins!\n");
        }
        return;
    }
    if (battle->forfeited)
    {
        printf("You forfeited the battle!\n");
    }
    else if (is_team_defeated(battle->player_team))
    {
        printf("All your Pokemon fainted! You lost.\n");
    }
    else
    {
        printf("All opponent Pokemon fainted! You won!\n");
    }
}
