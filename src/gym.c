#include "gym.h"
#include "battle.h"
#include "color.h"
#include "tsv.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static const GymLeader KANTO_GYMS[GYM_COUNT] = {
    {"Brock", "Boulder Badge", "Rock", AI_EASY, {"Geodude", "Onix", NULL, NULL}, 2},
    {"Misty", "Cascade Badge", "Water", AI_EASY, {"Staryu", "Starmie", NULL, NULL}, 2},
    {"Lt. Surge", "Thunder Badge", "Electric", AI_MEDIUM, {"Voltorb", "Pikachu", "Raichu", NULL}, 3},
    {"Erika", "Rainbow Badge", "Grass", AI_MEDIUM, {"Victreebel", "Tangela", "Vileplume", NULL}, 3},
    {"Koga", "Soul Badge", "Poison", AI_MEDIUM, {"Koffing", "Muk", "Weezing", NULL}, 3},
    {"Sabrina", "Marsh Badge", "Psychic", AI_HARD, {"Kadabra", "Mr. Mime", "Alakazam", NULL}, 3},
    {"Blaine", "Volcano Badge", "Fire", AI_HARD, {"Growlithe", "Ponyta", "Rapidash", "Arcanine"}, 4},
    {"Giovanni", "Earth Badge", "Ground", AI_HARD, {"Rhyhorn", "Dugtrio", "Nidoking", "Rhydon"}, 4},
};

static int equals_icase(const char *a, const char *b)
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

static const Pokemon *find_pokedex_by_name(const Pokemon *pokedex, int count, const char *name)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(pokedex[i].name, name) == 0)
        {
            return &pokedex[i];
        }
    }
    return NULL;
}

static int count_badges(int badges_mask)
{
    int count = 0;
    for (int i = 0; i < GYM_COUNT; i++)
    {
        if (badges_mask & (1 << i))
        {
            count++;
        }
    }
    return count;
}

const GymLeader *get_gym_leader(int index)
{
    if (index < 0 || index >= GYM_COUNT)
    {
        return NULL;
    }
    return &KANTO_GYMS[index];
}

int find_gym_index(const char *arg)
{
    int num = 0;

    if (!arg || *arg == '\0')
    {
        return -1;
    }
    if (tsv_parse_int(arg, &num) == 0)
    {
        if (num >= 1 && num <= GYM_COUNT)
        {
            return num - 1;
        }
        return -1;
    }
    for (int i = 0; i < GYM_COUNT; i++)
    {
        if (equals_icase(KANTO_GYMS[i].leader, arg))
        {
            return i;
        }
    }
    return -1;
}

void display_gyms(int badges_mask)
{
    printf("%sKanto Gym Leaders (%d/8 badges):%s\n", color_code(ANSI_BOLD),
           count_badges(badges_mask), color_code(ANSI_RESET));
    for (int i = 0; i < GYM_COUNT; i++)
    {
        const GymLeader *g = &KANTO_GYMS[i];
        int won = (badges_mask & (1 << i)) != 0;
        printf("  [%d] %s%-3s%s  %-10s | %-13s | %-8s | AI: %s\n", i + 1,
               color_code(won ? ANSI_GREEN : ANSI_DIM), won ? "WON" : "---", color_code(ANSI_RESET),
               g->leader, g->badge, g->specialty, ai_difficulty_to_string(g->difficulty));
    }
}

int challenge_gym(int index, const Pokemon *pokedex, int pokedex_count, Team *player_team,
                  int *badges_mask)
{
    const GymLeader *gym = get_gym_leader(index);
    if (!gym || !pokedex || !player_team || !badges_mask)
    {
        return -1;
    }
    if (!is_team_valid(*player_team) || is_team_defeated(*player_team))
    {
        fprintf(stderr, "gym: your team must have 1-6 healthy Pokemon\n");
        return -1;
    }

    Battle battle;
    memset(&battle, 0, sizeof(battle));
    battle.difficulty = gym->difficulty;
    battle.player_team = *player_team;

    for (int i = 0; i < gym->team_size; i++)
    {
        const Pokemon *p = find_pokedex_by_name(pokedex, pokedex_count, gym->pokemon_names[i]);
        if (p)
        {
            add_pokemon(&battle.opponent_team, *p);
        }
    }
    if (battle.opponent_team.count == 0)
    {
        fprintf(stderr, "gym: could not build team for %s\n", gym->leader);
        return -1;
    }

    printf("\n%s=== Gym #%d: %s (%s) ===%s\n", color_code(ANSI_BOLD), index + 1, gym->leader,
           gym->badge, color_code(ANSI_RESET));
    start_battle(&battle);
    *player_team = battle.player_team;

    if (!battle.forfeited && !is_team_defeated(battle.player_team) &&
        is_team_defeated(battle.opponent_team))
    {
        *badges_mask |= (1 << index);
        printf("%sYou earned the %s! (%d/8 badges)%s\n", color_code(ANSI_GREEN), gym->badge,
               count_badges(*badges_mask), color_code(ANSI_RESET));
        if (count_badges(*badges_mask) == GYM_COUNT)
        {
            print_slow(ANSI_YELLOW,
                       "Congratulations! You collected all 8 Kanto badges! You can now enter the 'league'!\n",
                       20);
        }
        return 1;
    }
    return 0;
}

int challenge_league(const Pokemon *pokedex, int pokedex_count,
                     Team *player_team, int badges_mask)
{
    static const GymLeader league[LEAGUE_COUNT] = {
        {"Elite Four Lorelei", "Indigo #1", "Ice",      AI_HARD, {"Dewgong", "Cloyster", "Slowbro", "Lapras"},     4},
        {"Elite Four Bruno",   "Indigo #2", "Fighting", AI_HARD, {"Onix", "Hitmonchan", "Hitmonlee", "Machamp"},  4},
        {"Elite Four Agatha",  "Indigo #3", "Ghost",    AI_HARD, {"Gengar", "Golbat", "Haunter", "Arbok"},        4},
        {"Elite Four Lance",   "Indigo #4", "Dragon",   AI_HARD, {"Gyarados", "Dragonair", "Aerodactyl", "Dragonite"}, 4},
        {"Champion Blue",      "Champion",  "Mixed",    AI_HARD, {"Pidgeot", "Alakazam", "Rhydon", "Charizard"}, 4},
    };

    if (!pokedex || !player_team)
    {
        return -1;
    }
    if (count_badges(badges_mask) < GYM_COUNT)
    {
        fprintf(stderr, "league: you need all 8 Kanto badges first (%d/8 earned)\n",
                count_badges(badges_mask));
        return -1;
    }
    if (!is_team_valid(*player_team) || is_team_defeated(*player_team))
    {
        fprintf(stderr, "league: your team must have 1-6 healthy Pokemon\n");
        return -1;
    }

    print_slow(ANSI_CYAN, "\n=== Welcome to the Indigo League! ===\n", 20);
    for (int m = 0; m < LEAGUE_COUNT; m++)
    {
        const GymLeader *op = &league[m];
        Battle battle;
        memset(&battle, 0, sizeof(battle));
        battle.difficulty = op->difficulty;
        battle.player_team = *player_team;

        for (int i = 0; i < op->team_size; i++)
        {
            const Pokemon *p = find_pokedex_by_name(pokedex, pokedex_count, op->pokemon_names[i]);
            if (p)
            {
                add_pokemon(&battle.opponent_team, *p);
            }
        }

        printf("\n%s=== Match %d/%d: %s ===%s\n",
               color_code(ANSI_BOLD), m + 1, LEAGUE_COUNT, op->leader, color_code(ANSI_RESET));
        start_battle(&battle);
        *player_team = battle.player_team;

        if (battle.forfeited || is_team_defeated(battle.player_team) ||
            !is_team_defeated(battle.opponent_team))
        {
            print_slow(ANSI_RED, "You were defeated in the Indigo League!\n", 20);
            return 0;
        }
    }

    print_slow(ANSI_YELLOW,
               "\n=== HALL OF FAME ===\nCongratulations! You are the Indigo League Champion!\n",
               20);
    return 1;
}
