#include "battle.h"
#include "ai.h"
#include "color.h"
#include "tsv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BATTLE_INPUT_MAX 64

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
    double raw =
        ((double)move.power * ((double)attacker.attack / (double)def) * 0.5 + 2.0) * mult * stab;
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

void execute_attack(Pokemon *attacker, Pokemon *defender, Move move)
{
    if (!attacker || !defender)
    {
        return;
    }

    if (attacker->status == STATUS_PARALYSIS && rand() % 4 == 0)
    {
        printf("%s%s is paralyzed and can't move!%s\n", color_code(ANSI_YELLOW), attacker->name,
               color_code(ANSI_RESET));
        return;
    }

    consume_move_pp(attacker, move.name);

    char msg[128];
    snprintf(msg, sizeof(msg), "%s uses %s!\n", attacker->name, move.name);
    print_slow(ANSI_BOLD, msg, 15);
    if (rand() % 100 >= move.accuracy)
    {
        print_slow(ANSI_YELLOW, "  The attack missed!\n", 15);
        return;
    }

    if (move.power == 0 && move.effect == EFFECT_HEAL)
    {
        apply_heal(attacker, move.effect_chance);
        return;
    }

    double mult = type_effectiveness(move.type, defender->type);
    if (mult == 0.0)
    {
        snprintf(msg, sizeof(msg), "  It doesn't affect %s!\n", defender->name);
        print_slow(ANSI_RED, msg, 15);
        return;
    }

    if (move.power == 0)
    {
        if (defender->status != STATUS_NONE)
        {
            printf("  But it failed! (%s already has a status)\n", defender->name);
            return;
        }
        apply_move_effect(attacker, defender, move);
        return;
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
        return;
    }

    apply_move_effect(attacker, defender, move);
}

static int read_int_choice(const char *prompt, int min, int max)
{
    char line[BATTLE_INPUT_MAX];

    while (1)
    {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin))
        {
            return -1;
        }
        int val = 0;
        if (tsv_parse_int(tsv_trim(line), &val) == 0 && val >= min && val <= max)
        {
            return val;
        }
        printf("Invalid choice. Enter a number between %d and %d.\n", min, max);
    }
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

static int count_alive_switches(const Battle *battle)
{
    int count = 0;
    for (int i = 0; i < battle->player_team.count; i++)
    {
        if (i != battle->player_active && is_pokemon_alive(battle->player_team.pokemons[i]))
        {
            count++;
        }
    }
    return count;
}

static int prompt_switch(Battle *battle)
{
    if (count_alive_switches(battle) == 0)
    {
        printf("No other healthy Pokemon to switch to!\n");
        return -1;
    }

    printf("Choose a Pokemon to send out:\n");
    for (int i = 0; i < battle->player_team.count; i++)
    {
        if (is_pokemon_alive(battle->player_team.pokemons[i]))
        {
            printf("  [%d] ", i + 1);
            display_pokemon(&battle->player_team.pokemons[i]);
        }
    }

    while (1)
    {
        int slot = read_int_choice("Switch to (1-6): ", 1, battle->player_team.count);
        if (slot < 0)
        {
            return -1;
        }
        int idx = slot - 1;
        if (idx == battle->player_active)
        {
            printf("%s is already in battle!\n", battle->player_team.pokemons[idx].name);
            continue;
        }
        if (!is_pokemon_alive(battle->player_team.pokemons[idx]))
        {
            printf("%s has fainted!\n", battle->player_team.pokemons[idx].name);
            continue;
        }
        battle->player_active = idx;
        printf("Go, %s!\n", battle->player_team.pokemons[idx].name);
        return 0;
    }
}

static int has_usable_move(const Pokemon *pokemon)
{
    for (int i = 0; i < pokemon->move_count; i++)
    {
        if (pokemon->moves[i].pp > 0)
        {
            return 1;
        }
    }
    return 0;
}

static int prompt_move(const Pokemon *pokemon)
{
    if (pokemon->move_count <= 0)
    {
        return -1;
    }
    if (!has_usable_move(pokemon))
    {
        printf("%s has no PP left! Using Struggle!\n", pokemon->name);
        return -2;
    }
    printf("Choose a move:\n");
    for (int i = 0; i < pokemon->move_count; i++)
    {
        printf("  [%d] ", i + 1);
        display_move(&pokemon->moves[i]);
    }
    while (1)
    {
        int choice = read_int_choice("Move: ", 1, pokemon->move_count);
        if (choice < 0)
        {
            return -1;
        }
        if (pokemon->moves[choice - 1].pp <= 0)
        {
            printf("No PP left for %s! Choose another move.\n", pokemon->moves[choice - 1].name);
            continue;
        }
        return choice - 1;
    }
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

static void resolve_attacks(Pokemon *player, Move player_move, Pokemon *opponent,
                            Move opponent_move)
{
    if (player_goes_first(player, opponent))
    {
        execute_attack(player, opponent, player_move);
        if (is_pokemon_alive(*opponent))
        {
            execute_attack(opponent, player, opponent_move);
        }
    }
    else
    {
        execute_attack(opponent, player, opponent_move);
        if (is_pokemon_alive(*player))
        {
            execute_attack(player, opponent, player_move);
        }
    }
}

static void handle_fainted(Battle *battle)
{
    Pokemon *player = &battle->player_team.pokemons[battle->player_active];
    Pokemon *opponent = &battle->opponent_team.pokemons[battle->opponent_active];

    if (!is_pokemon_alive(*player) && !is_team_defeated(battle->player_team))
    {
        if (prompt_switch(battle) != 0)
        {
            battle->forfeited = 1;
        }
    }
    if (!is_pokemon_alive(*opponent) && !is_team_defeated(battle->opponent_team))
    {
        int next = ai_choose_switch(&battle->opponent_team, player, battle->difficulty);
        if (next >= 0)
        {
            battle->opponent_active = next;
            printf("Opponent sends out %s!\n", battle->opponent_team.pokemons[next].name);
        }
    }
}

static void apply_end_turn_status(Pokemon *pokemon)
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
    printf("%s is hurt by %s (-%d HP, %d/%d HP)!\n", pokemon->name,
           pokemon->status == STATUS_POISON ? "poison" : "burn", dmg, pokemon->hp, pokemon->max_hp);
    if (!is_pokemon_alive(*pokemon))
    {
        printf("  %s%s fainted!%s\n", color_code(ANSI_RED), pokemon->name, color_code(ANSI_RESET));
    }
}

void execute_turn(Battle *battle)
{
    if (!battle)
    {
        return;
    }

    Pokemon *player = &battle->player_team.pokemons[battle->player_active];
    Pokemon *opponent = &battle->opponent_team.pokemons[battle->opponent_active];

    printf("\n--- Turn %d ---\n", battle->turn);
    printf("You      : ");
    display_pokemon(player);
    printf("Opponent : ");
    display_pokemon(opponent);

    printf("Action: [1] Attack  [2] Switch  [3] Forfeit\n");
    int action = read_int_choice("Choice: ", 1, 3);
    if (action < 0 || action == 3)
    {
        battle->forfeited = 1;
        return;
    }

    if (action == 1)
    {
        int move_idx = prompt_move(player);
        if (move_idx == -1)
        {
            battle->forfeited = 1;
            return;
        }
        Move chosen = move_idx == -2
                          ? (Move){"Struggle", 50, 100, TYPE_NORMAL, EFFECT_NONE, 0, 1, 1}
                          : player->moves[move_idx];
        int ai_sw = ai_should_switch(&battle->opponent_team, battle->opponent_active, player,
                                     battle->difficulty);
        if (ai_sw >= 0)
        {
            printf("Opponent withdrew %s!\n", opponent->name);
            battle->opponent_active = ai_sw;
            opponent = &battle->opponent_team.pokemons[ai_sw];
            printf("Opponent sends out %s!\n", opponent->name);
            execute_attack(player, opponent, chosen);
        }
        else
        {
            Move ai_move = ai_choose_move(opponent, player, battle->difficulty);
            resolve_attacks(player, chosen, opponent, ai_move);
        }
    }
    else if (action == 2)
    {
        if (prompt_switch(battle) == 0)
        {
            player = &battle->player_team.pokemons[battle->player_active];
            execute_attack(opponent, player, ai_choose_move(opponent, player, battle->difficulty));
        }
    }

    apply_end_turn_status(&battle->player_team.pokemons[battle->player_active]);
    apply_end_turn_status(&battle->opponent_team.pokemons[battle->opponent_active]);
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
    battle->player_active = find_next_alive(&battle->player_team);
    battle->opponent_active = find_next_alive(&battle->opponent_team);

    if (battle->player_active < 0 || battle->opponent_active < 0)
    {
        fprintf(stderr, "battle: both teams need at least one healthy Pokemon\n");
        return;
    }

    printf("\n=== Battle Start! (AI: %s) ===\n", ai_difficulty_to_string(battle->difficulty));
    printf("You send out %s!\n", battle->player_team.pokemons[battle->player_active].name);
    printf("Opponent sends out %s!\n",
           battle->opponent_team.pokemons[battle->opponent_active].name);

    while (!battle->forfeited && !is_team_defeated(battle->player_team) &&
           !is_team_defeated(battle->opponent_team))
    {
        battle->turn++;
        execute_turn(battle);
    }

    printf("\n=== Battle Over ===\n");
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
