#include "display.h"
#include "color.h"
#include "utils.h"

#include <stdio.h>

#define DISPLAY_INPUT_MAX 64

static const char *hp_color(int hp, int max_hp)
{
    if (hp <= 0 || max_hp <= 0 || hp * 5 <= max_hp)
    {
        return ANSI_RED;
    }
    if (hp * 2 <= max_hp)
    {
        return ANSI_YELLOW;
    }
    return ANSI_GREEN;
}

static const char *status_color(enum Status status)
{
    switch (status)
    {
        case STATUS_POISON:
            return ANSI_MAGENTA;
        case STATUS_BURN:
            return ANSI_RED;
        case STATUS_PARALYSIS:
            return ANSI_YELLOW;
        default:
            return ANSI_RESET;
    }
}

static void print_hp_bar(int hp, int max_hp)
{
    const int width = 10;
    int filled = 0;
    const char *c = hp_color(hp, max_hp);

    if (max_hp > 0 && hp > 0)
    {
        filled = (hp * width + max_hp - 1) / max_hp;
        if (filled > width)
        {
            filled = width;
        }
    }
    printf("[%s", color_code(c));
    for (int i = 0; i < width; i++)
    {
        putchar(i < filled ? '=' : '-');
    }
    printf("%s]", color_code(ANSI_RESET));
}

void display_move(const Move *move)
{
    if (!move)
    {
        return;
    }

    const char *type_name = type_to_string(move->type);
    const char *tc = color_for_type((int)move->type);
    const char *pc =
        move->pp <= 0 ? ANSI_RED : (move->pp * 3 <= move->max_pp ? ANSI_YELLOW : ANSI_RESET);
    printf("%-14s [%s%-8s%s] Pwr:%3d  Acc:%3d%%  PP:%s%2d/%-2d%s", move->name, color_code(tc),
           type_name ? type_name : "UNKNOWN", color_code(ANSI_RESET), move->power, move->accuracy,
           color_code(pc), move->pp, move->max_pp, color_code(ANSI_RESET));
    if (move->effect != EFFECT_NONE)
    {
        printf("  (%s%s %d%%%s)", color_code(ANSI_CYAN), move_effect_to_string(move->effect),
               move->effect_chance, color_code(ANSI_RESET));
    }
    printf("\n");
}

void display_pokemon(const Pokemon *pokemon)
{
    if (!pokemon)
    {
        return;
    }

    const char *c = hp_color(pokemon->hp, pokemon->max_hp);
    const char *type_name = type_to_string(pokemon->type);
    const char *tc = color_for_type((int)pokemon->type);
    printf("%-11s [%s%-8s%s] ", pokemon->name, color_code(tc), type_name ? type_name : "UNKNOWN",
           color_code(ANSI_RESET));
    print_hp_bar(pokemon->hp, pokemon->max_hp);
    printf(" %s%3d/%-3d HP%s", color_code(c), pokemon->hp, pokemon->max_hp, color_code(ANSI_RESET));
    if (pokemon->status != STATUS_NONE)
    {
        printf(" [%s%s%s]", color_code(status_color(pokemon->status)),
               status_to_string(pokemon->status), color_code(ANSI_RESET));
    }
    printf("\n");
}

void display_pokemon_stats(const Pokemon *pokemon)
{
    if (!pokemon)
    {
        return;
    }

    const char *type_name = type_to_string(pokemon->type);
    const char *tc = color_for_type((int)pokemon->type);
    const char *c = hp_color(pokemon->hp, pokemon->max_hp);
    printf("%s=== %s ===%s\n", color_code(ANSI_BOLD), pokemon->name, color_code(ANSI_RESET));
    printf("  Type    : %s%s%s\n", color_code(tc), type_name ? type_name : "UNKNOWN",
           color_code(ANSI_RESET));
    printf("  Status  : %s\n", status_to_string(pokemon->status));
    printf("  HP      : ");
    print_hp_bar(pokemon->hp, pokemon->max_hp);
    printf(" %s%d/%d%s\n", color_code(c), pokemon->hp, pokemon->max_hp, color_code(ANSI_RESET));
    printf("  Attack  : %d  |  Defense : %d  |  Speed : %d\n", pokemon->attack, pokemon->defense,
           pokemon->speed);
    printf("  Moves   : %d\n", pokemon->move_count);
    for (int i = 0; i < pokemon->move_count; i++)
    {
        printf("    - ");
        display_move(&pokemon->moves[i]);
    }
}

void display_team(Team team)
{
    printf("Team (%d/6):\n", team.count);
    if (team.count <= 0)
    {
        printf("  (empty)\n");
        return;
    }
    for (int i = 0; i < team.count; i++)
    {
        printf("  [%d] ", i + 1);
        display_pokemon(&team.pokemons[i]);
    }
}

void animate_hp_change(const Pokemon *pokemon, int old_hp, int new_hp)
{
    if (!pokemon)
    {
        return;
    }
    if (!color_is_supported() || old_hp == new_hp)
    {
        printf("  %-11s ", pokemon->name);
        print_hp_bar(new_hp, pokemon->max_hp);
        printf(" %s%3d/%-3d HP%s\n", color_code(hp_color(new_hp, pokemon->max_hp)), new_hp,
               pokemon->max_hp, color_code(ANSI_RESET));
        return;
    }

    int diff = new_hp - old_hp;
    int steps = diff < 0 ? -diff : diff;
    if (steps > 15)
    {
        steps = 15;
    }

    for (int s = 0; s <= steps; s++)
    {
        int cur = old_hp + (diff * s) / steps;
        printf("\r  %-11s ", pokemon->name);
        print_hp_bar(cur, pokemon->max_hp);
        printf(" %s%3d/%-3d HP%s   ", color_code(hp_color(cur, pokemon->max_hp)), cur,
               pokemon->max_hp, color_code(ANSI_RESET));
        fflush(stdout);
        sleep_ms(25);
    }
    printf("\n");
}

int ui_read_int_choice(const char *prompt, int min, int max)
{
    char line[DISPLAY_INPUT_MAX];

    while (1)
    {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin))
        {
            return -1;
        }
        int val = 0;
        if (str_parse_int(str_trim(line), &val) == 0 && val >= min && val <= max)
        {
            return val;
        }
        printf("Invalid choice. Enter a number between %d and %d.\n", min, max);
    }
}

int ui_prompt_battle_action_for(const char *label, int turn,
                                const Pokemon *self, const Pokemon *foe)
{
    if (label)
    {
        printf("\n%s--- %s ---%s\n", color_code(ANSI_CYAN), label, color_code(ANSI_RESET));
        printf("%s : ", label);
        display_pokemon(self);
        printf("Opponent : ");
        display_pokemon(foe);
    }
    else
    {
        printf("\n%s--- Turn %d ---%s\n", color_code(ANSI_CYAN), turn, color_code(ANSI_RESET));
        printf("You      : ");
        display_pokemon(self);
        printf("Opponent : ");
        display_pokemon(foe);
    }

    printf("Action: [%s1%s] Attack  [%s2%s] Switch  [%s3%s] Forfeit\n", color_code(ANSI_GREEN),
           color_code(ANSI_RESET), color_code(ANSI_CYAN), color_code(ANSI_RESET),
           color_code(ANSI_RED), color_code(ANSI_RESET));
    return ui_read_int_choice("Choice: ", 1, 3);
}

int ui_prompt_battle_action(int turn, const Pokemon *player, const Pokemon *opponent)
{
    return ui_prompt_battle_action_for(NULL, turn, player, opponent);
}

int ui_prompt_move(const Pokemon *pokemon)
{
    int usable = 0;

    if (!pokemon || pokemon->move_count <= 0)
    {
        return -1;
    }
    for (int i = 0; i < pokemon->move_count; i++)
    {
        if (pokemon->moves[i].pp > 0)
        {
            usable = 1;
            break;
        }
    }
    if (!usable)
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
        int choice = ui_read_int_choice("Move: ", 1, pokemon->move_count);
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

int ui_prompt_switch_for(const char *label, const Team *team, int active_idx)
{
    int alive_others = 0;

    for (int i = 0; i < team->count; i++)
    {
        if (i != active_idx && team->pokemons[i].hp > 0)
        {
            alive_others++;
        }
    }
    if (alive_others == 0)
    {
        printf("No other healthy Pokemon to switch to!\n");
        return -1;
    }

    if (label)
    {
        printf("%s - Choose a Pokemon to send out:\n", label);
    }
    else
    {
        printf("Choose a Pokemon to send out:\n");
    }
    for (int i = 0; i < team->count; i++)
    {
        if (team->pokemons[i].hp > 0)
        {
            printf("  [%d] ", i + 1);
            display_pokemon(&team->pokemons[i]);
        }
    }

    while (1)
    {
        int slot = ui_read_int_choice("Switch to (1-6): ", 1, team->count);
        if (slot < 0)
        {
            return -1;
        }
        int idx = slot - 1;
        if (idx == active_idx)
        {
            printf("%s is already in battle!\n", team->pokemons[idx].name);
            continue;
        }
        if (team->pokemons[idx].hp <= 0)
        {
            printf("%s has fainted!\n", team->pokemons[idx].name);
            continue;
        }
        printf("Go, %s!\n", team->pokemons[idx].name);
        return idx;
    }
}

int ui_prompt_switch(const Team *team, int active_idx)
{
    return ui_prompt_switch_for(NULL, team, active_idx);
}
