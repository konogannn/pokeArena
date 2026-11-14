#include "ui.h"
#include "ai.h"
#include "battle.h"
#include "color.h"
#include "gym.h"
#include "save.h"
#include "tsv.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UI_INPUT_MAX 128

static int equals_ignore_case(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
        {
            return 0;
        }
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static const Pokemon *find_pokedex_pokemon(const Pokemon *pokedex, int count, const char *arg)
{
    int index = 0;

    if (!arg || *arg == '\0')
    {
        return NULL;
    }
    if (tsv_parse_int(arg, &index) == 0)
    {
        if (index >= 0 && index < count)
        {
            return &pokedex[index];
        }
        return NULL;
    }
    for (int i = 0; i < count; i++)
    {
        if (equals_ignore_case(pokedex[i].name, arg))
        {
            return &pokedex[i];
        }
    }
    return NULL;
}

static char *split_word(char *str, char **out_rest)
{
    char *word = tsv_trim(str);
    char *cursor = word;

    while (*cursor && !isspace((unsigned char)*cursor))
    {
        cursor++;
    }
    if (*cursor)
    {
        *cursor = '\0';
        *out_rest = tsv_trim(cursor + 1);
    }
    else
    {
        *out_rest = NULL;
    }
    return word;
}

static void cmd_help(void)
{
    printf("%sAvailable commands:%s\n", color_code(ANSI_BOLD), color_code(ANSI_RESET));
    printf("  %spokedex%s (p)          - List all available Pokemon\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %smoves%s (m)            - List all available moves\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %sinfo%s <name|index>    - Show detailed stats and moves\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %steam%s (t)             - Show your current team\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %sadd%s <name|index>     - Add a Pokemon to your team\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %sremove%s <1-6>         - Remove a Pokemon from your team\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %sswap%s <1-6> <1-6>     - Swap two Pokemon in your team\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %sautofill%s             - Fill remaining team slots randomly\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %sbattle%s (b)           - Start a battle with your current team\n",
           color_code(ANSI_GREEN), color_code(ANSI_RESET));
    printf("  %sgyms%s                 - List all 8 Kanto Gym Leaders & badges\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %sgym%s <1-8|name>       - Challenge a Kanto Gym Leader\n",
           color_code(ANSI_YELLOW), color_code(ANSI_RESET));
    printf("  %sheal%s                 - Restore all team Pokemon to full HP\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %ssave%s [path]          - Save your team to a TSV file\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %sload%s [path]          - Load a team from a TSV file\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %sclear%s                - Clear your team\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %shelp%s (h)             - Show this help message\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("  %squit%s (q)             - Exit the game\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
}

static void cmd_pokedex(const Pokemon *pokedex, int count)
{
    printf("%sPokedex (%d Pokemon):%s\n", color_code(ANSI_BOLD), count, color_code(ANSI_RESET));
    for (int i = 0; i < count; i++)
    {
        printf("  [%2d] ", i);
        display_pokemon(&pokedex[i]);
    }
}

static void cmd_moves(const Move *moves, int count)
{
    printf("%sMoves (%d):%s\n", color_code(ANSI_BOLD), count, color_code(ANSI_RESET));
    for (int i = 0; i < count; i++)
    {
        printf("  [%2d] ", i);
        display_move(&moves[i]);
    }
}

static void cmd_info(const Pokemon *pokedex, int count, const char *arg)
{
    const Pokemon *p = find_pokedex_pokemon(pokedex, count, arg);

    if (!p)
    {
        fprintf(stderr, "ui: unknown Pokemon '%s'\n", arg ? arg : "");
        return;
    }
    display_pokemon_stats(p);
}

static void cmd_add(const Pokemon *pokedex, int count, Team *team, const char *arg)
{
    const Pokemon *p = find_pokedex_pokemon(pokedex, count, arg);

    if (!p)
    {
        fprintf(stderr, "ui: unknown Pokemon '%s'\n", arg ? arg : "");
        return;
    }
    add_pokemon(team, *p);
    printf("%sAdded %s to your team (%d/6).%s\n",
           color_code(ANSI_GREEN), p->name, team->count, color_code(ANSI_RESET));
}

static void cmd_remove(Team *team, const char *arg)
{
    int index = 0;

    if (!arg || tsv_parse_int(arg, &index) != 0)
    {
        fprintf(stderr, "ui: usage: remove <1-6>\n");
        return;
    }
    remove_pokemon(team, index - 1);
}

static void cmd_swap(Team *team, char *arg)
{
    char *second = NULL;
    const char *first = arg ? split_word(arg, &second) : NULL;
    int i = 0;
    int j = 0;

    if (!first || !second || tsv_parse_int(first, &i) != 0 || tsv_parse_int(second, &j) != 0)
    {
        fprintf(stderr, "ui: usage: swap <1-6> <1-6>\n");
        return;
    }
    swap_pokemon(team, i - 1, j - 1);
}

static void cmd_autofill(const Pokemon *pokedex, int pokedex_count, Team *team)
{
    if (pokedex_count <= 0)
    {
        return;
    }
    if (team->count >= 6)
    {
        printf("Team is already full (6/6).\n");
        return;
    }
    while (team->count < 6)
    {
        int pick = rand() % pokedex_count;
        add_pokemon(team, pokedex[pick]);
    }
    printf("%sTeam autofilled (%d/6).%s\n",
           color_code(ANSI_GREEN), team->count, color_code(ANSI_RESET));
}

static enum AiDifficulty parse_difficulty(const char *arg)
{
    if (arg)
    {
        if (equals_ignore_case(arg, "easy") || strcmp(arg, "1") == 0)
        {
            return AI_EASY;
        }
        if (equals_ignore_case(arg, "medium") || strcmp(arg, "2") == 0)
        {
            return AI_MEDIUM;
        }
        if (equals_ignore_case(arg, "hard") || strcmp(arg, "3") == 0)
        {
            return AI_HARD;
        }
    }

    char line[UI_INPUT_MAX];
    int choice = 2;
    printf("Select AI difficulty: [1] Easy  [2] Medium  [3] Hard (default 2): ");
    if (fgets(line, sizeof(line), stdin) && tsv_parse_int(tsv_trim(line), &choice) == 0 &&
        choice >= 1 && choice <= 3)
    {
        return (enum AiDifficulty)choice;
    }
    return AI_MEDIUM;
}

static void cmd_battle(const Pokemon *pokedex, int pokedex_count, Team *team, const char *arg)
{
    if (!is_team_valid(*team) || is_team_defeated(*team))
    {
        fprintf(stderr, "ui: your team must have 1-6 healthy Pokemon to battle\n");
        return;
    }

    Battle battle;
    memset(&battle, 0, sizeof(battle));
    battle.difficulty = parse_difficulty(arg);
    battle.player_team = *team;
    battle.opponent_team = ai_generate_team(pokedex, pokedex_count, team->count);

    start_battle(&battle);
    *team = battle.player_team;
}

static void cmd_gym(const Pokemon *pokedex, int pokedex_count, Team *team,
                    int *badges_mask, const char *arg)
{
    int idx = find_gym_index(arg);

    if (idx < 0)
    {
        fprintf(stderr, "ui: usage: gym <1-8|leader name>\n");
        return;
    }
    challenge_gym(idx, pokedex, pokedex_count, team, badges_mask);
}

static void cmd_save(const Team *team, int badges_mask, const char *arg)
{
    const char *path = (arg && *arg != '\0') ? arg : "data/team.tsv";

    if (save_team(path, team, badges_mask) == 0)
    {
        printf("%sSaved to '%s' (%d/6 Pokemon).%s\n",
               color_code(ANSI_GREEN), path, team->count, color_code(ANSI_RESET));
    }
}

static void cmd_load(const Pokemon *pokedex, int pokedex_count,
                     Team *team, int *badges_mask, const char *arg)
{
    const char *path = (arg && *arg != '\0') ? arg : "data/team.tsv";

    if (load_team(path, pokedex, pokedex_count, team, badges_mask) == 0)
    {
        printf("%sLoaded from '%s' (%d/6 Pokemon).%s\n",
               color_code(ANSI_GREEN), path, team->count, color_code(ANSI_RESET));
        display_team(*team);
    }
}

static int handle_command(const char *cmd, char *arg,
                          const Pokemon *pokedex, int pokedex_count,
                          const Move *moves, int move_count, Team *team, int *badges_mask)
{
    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0 || strcmp(cmd, "q") == 0)
    {
        return 1;
    }
    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0)
    {
        cmd_help();
    }
    else if (strcmp(cmd, "pokedex") == 0 || strcmp(cmd, "p") == 0)
    {
        cmd_pokedex(pokedex, pokedex_count);
    }
    else if (strcmp(cmd, "moves") == 0 || strcmp(cmd, "m") == 0)
    {
        cmd_moves(moves, move_count);
    }
    else if (strcmp(cmd, "info") == 0)
    {
        cmd_info(pokedex, pokedex_count, arg);
    }
    else if (strcmp(cmd, "team") == 0 || strcmp(cmd, "t") == 0)
    {
        display_team(*team);
    }
    else if (strcmp(cmd, "add") == 0)
    {
        cmd_add(pokedex, pokedex_count, team, arg);
    }
    else if (strcmp(cmd, "remove") == 0)
    {
        cmd_remove(team, arg);
    }
    else if (strcmp(cmd, "swap") == 0)
    {
        cmd_swap(team, arg);
    }
    else if (strcmp(cmd, "autofill") == 0)
    {
        cmd_autofill(pokedex, pokedex_count, team);
    }
    else if (strcmp(cmd, "battle") == 0 || strcmp(cmd, "b") == 0)
    {
        cmd_battle(pokedex, pokedex_count, team, arg);
    }
    else if (strcmp(cmd, "gyms") == 0)
    {
        display_gyms(*badges_mask);
    }
    else if (strcmp(cmd, "gym") == 0)
    {
        cmd_gym(pokedex, pokedex_count, team, badges_mask, arg);
    }
    else if (strcmp(cmd, "heal") == 0)
    {
        heal_team(team);
        printf("%sTeam healed to full HP.%s\n", color_code(ANSI_GREEN), color_code(ANSI_RESET));
    }
    else if (strcmp(cmd, "save") == 0)
    {
        cmd_save(team, *badges_mask, arg);
    }
    else if (strcmp(cmd, "load") == 0)
    {
        cmd_load(pokedex, pokedex_count, team, badges_mask, arg);
    }
    else if (strcmp(cmd, "clear") == 0)
    {
        team->count = 0;
        printf("Team cleared.\n");
    }
    else
    {
        printf("Unknown command '%s'. Type 'help' for a list of commands.\n", cmd);
    }
    return 0;
}

void run_game_loop(const Pokemon *pokedex, int pokedex_count,
                   const Move *moves, int move_count, Team *team)
{
    char line[UI_INPUT_MAX];
    int badges_mask = 0;

    if (!pokedex || !moves || !team)
    {
        return;
    }

    printf("%s+--------------------------------------+%s\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("%s|         Welcome to PokeArena!        |%s\n",
           color_code(ANSI_BOLD), color_code(ANSI_RESET));
    printf("%s+--------------------------------------+%s\n",
           color_code(ANSI_CYAN), color_code(ANSI_RESET));
    printf("Loaded %d Pokemon and %d moves. Type '%shelp%s' to begin.\n",
           pokedex_count, move_count, color_code(ANSI_CYAN), color_code(ANSI_RESET));

    while (1)
    {
        printf("%sPokeArena [%d/6]> %s",
               color_code(ANSI_BOLD), team->count, color_code(ANSI_RESET));
        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        char *arg = NULL;
        char *cmd = split_word(line, &arg);
        if (*cmd == '\0')
        {
            continue;
        }
        if (handle_command(cmd, arg, pokedex, pokedex_count, moves, move_count, team, &badges_mask) != 0)
        {
            break;
        }
    }
}
