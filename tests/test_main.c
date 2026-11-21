#include "ai.h"
#include "battle.h"
#include "gym.h"
#include "moves.h"
#include "pokemon.h"
#include "save.h"
#include "team.h"
#include "type.h"
#include "utils.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;

#define RUN_TEST(fn)               \
    do                             \
    {                              \
        fn();                      \
        tests_run++;               \
        printf("  [PASS] %s\n", #fn); \
    } while (0)

static void test_utils(void)
{
    char buf1[] = "   hello world \t\n";
    assert(strcmp(str_trim(buf1), "hello world") == 0);

    char buf2[] = "   ";
    assert(strcmp(str_trim(buf2), "") == 0);

    int val = 0;
    assert(str_parse_int("42", &val) == 0 && val == 42);
    assert(str_parse_int("-7", &val) == 0 && val == -7);
    assert(str_parse_int("12abc", &val) == -1);
    assert(str_parse_int("", &val) == -1);

    assert(str_equals_icase("Pikachu", "pIkAcHu") == 1);
    assert(str_equals_icase("Pikachu", "Raichu") == 0);
}

static void test_types_gen4(void)
{
    enum Type t = TYPE_NORMAL;
    assert(type_from_string("Fire", &t) == 0 && t == TYPE_FIRE);
    assert(type_from_string("Steel", &t) == 0 && t == TYPE_STEEL);
    assert(type_from_string("Invalid", &t) == -1);
    assert(strcmp(type_to_string(TYPE_ELECTRIC), "Electric") == 0);

    /* Gen 4 specifics: Steel resists Ghost and Dark */
    assert(fabs(type_effectiveness(TYPE_GHOST, TYPE_STEEL) - 0.5) < 1e-9);
    assert(fabs(type_effectiveness(TYPE_DARK, TYPE_STEEL) - 0.5) < 1e-9);
    /* Immunities and weaknesses */
    assert(fabs(type_effectiveness(TYPE_ELECTRIC, TYPE_GROUND) - 0.0) < 1e-9);
    assert(fabs(type_effectiveness(TYPE_WATER, TYPE_FIRE) - 2.0) < 1e-9);
    assert(fabs(type_effectiveness(TYPE_GRASS, TYPE_FIRE) - 0.5) < 1e-9);
}

static void test_loaders(void)
{
    int move_count = 0;
    Move *moves = load_moves("data/moves.tsv", &move_count);
    assert(moves != NULL && move_count == 76);

    int poke_count = 0;
    Pokemon *pokedex = load_pokemons("data/pokemon.tsv", moves, move_count, &poke_count);
    assert(pokedex != NULL && poke_count == 151);

    assert(load_moves("nonexistent.tsv", &move_count) == NULL);
    assert(load_pokemons("nonexistent.tsv", moves, move_count, &poke_count) == NULL);

    free_pokemons(pokedex);
    free_moves(moves);
}

static void test_team_management(void)
{
    int move_count = 0;
    Move *moves = load_moves("data/moves.tsv", &move_count);
    int poke_count = 0;
    Pokemon *pokedex = load_pokemons("data/pokemon.tsv", moves, move_count, &poke_count);
    assert(moves != NULL && pokedex != NULL);

    Team team;
    memset(&team, 0, sizeof(team));
    assert(is_team_valid(team) == 0);

    add_pokemon(&team, pokedex[0]); /* Bulbasaur */
    add_pokemon(&team, pokedex[3]); /* Charmander */
    assert(team.count == 2);
    assert(is_team_valid(team) == 1);

    swap_pokemon(&team, 0, 1);
    assert(strcmp(team.pokemons[0].name, "Charmander") == 0);
    assert(strcmp(team.pokemons[1].name, "Bulbasaur") == 0);

    team.pokemons[0].hp = 5;
    team.pokemons[0].status = STATUS_BURN;
    team.pokemons[0].moves[0].pp = 0;
    heal_team(&team);
    assert(team.pokemons[0].hp == team.pokemons[0].max_hp);
    assert(team.pokemons[0].status == STATUS_NONE);
    assert(team.pokemons[0].moves[0].pp == team.pokemons[0].moves[0].max_pp);

    remove_pokemon(&team, 0);
    assert(team.count == 1 && strcmp(team.pokemons[0].name, "Bulbasaur") == 0);

    free_pokemons(pokedex);
    free_moves(moves);
}

static void test_battle_damage_and_status(void)
{
    Move fire_move = {"Flamethrower", 95, 100, TYPE_FIRE, EFFECT_BURN, 10, 15, 15};
    Move electric_move = {"Thunderbolt", 95, 100, TYPE_ELECTRIC, EFFECT_PARALYSIS, 10, 15, 15};

    Pokemon charizard = {"Charizard", 78, 78, 84, 78, 100, TYPE_FIRE, STATUS_NONE, {fire_move}, 1};
    Pokemon venusaur = {"Venusaur", 80, 80, 82, 83, 80, TYPE_GRASS, STATUS_NONE, {fire_move}, 1};
    Pokemon onix = {"Onix", 35, 35, 45, 160, 70, TYPE_GROUND, STATUS_NONE, {fire_move}, 1};

    /* STAB (1.5x) + Super Effective (2.0x) */
    int stab_se = calculate_damage(charizard, venusaur, fire_move);
    /* Non-STAB (1.0x) + Super Effective (2.0x) */
    int nostab_se = calculate_damage(venusaur, venusaur, fire_move);
    assert(stab_se > nostab_se);

    /* Immunity returns 0 */
    assert(calculate_damage(charizard, onix, electric_move) == 0);

    assert(is_pokemon_alive(charizard) == 1);
    charizard.hp = 0;
    assert(is_pokemon_alive(charizard) == 0);
}

static void test_ai_modes(void)
{
    Move immune_move = {"Thunderbolt", 95, 100, TYPE_ELECTRIC, EFFECT_NONE, 0, 15, 15};
    Move good_move = {"Surf", 95, 100, TYPE_WATER, EFFECT_NONE, 0, 15, 15};
    Move weak_move = {"Tackle", 35, 100, TYPE_NORMAL, EFFECT_NONE, 0, 15, 15};

    Pokemon opp = {"Starmie", 60, 60, 75, 85, 115, TYPE_WATER, STATUS_NONE,
                   {immune_move, weak_move, good_move}, 3};
    Pokemon player = {"Onix", 35, 35, 45, 160, 70, TYPE_GROUND, STATUS_NONE, {weak_move}, 1};

    Move chosen = ai_choose_move(&opp, &player, AI_HARD);
    assert(strcmp(chosen.name, "Surf") == 0);

    /* Hard AI switches out when low HP and healthy counter is on bench */
    Pokemon bench = {"Venusaur", 80, 80, 82, 83, 80, TYPE_GRASS, STATUS_NONE, {weak_move}, 1};
    Team ai_team;
    memset(&ai_team, 0, sizeof(ai_team));
    opp.hp = 10; /* <= 25% HP */
    add_pokemon(&ai_team, opp);
    add_pokemon(&ai_team, bench);

    assert(ai_should_switch(&ai_team, 0, &player, AI_HARD) == 1);
    assert(ai_should_switch(&ai_team, 0, &player, AI_EASY) == -1);
}

static void test_save_and_load(void)
{
    int move_count = 0;
    Move *moves = load_moves("data/moves.tsv", &move_count);
    int poke_count = 0;
    Pokemon *pokedex = load_pokemons("data/pokemon.tsv", moves, move_count, &poke_count);

    Team saved;
    memset(&saved, 0, sizeof(saved));
    add_pokemon(&saved, pokedex[24]); /* Pikachu */
    saved.pokemons[0].hp = 17;
    saved.pokemons[0].status = STATUS_POISON;
    saved.pokemons[0].moves[0].pp = 12;

    const char *tmp_path = "/tmp/pokearena_unit_test_save.tsv";
    assert(save_team(tmp_path, &saved, 0x05) == 0);

    Team loaded;
    memset(&loaded, 0, sizeof(loaded));
    int badges = 0;
    assert(load_team(tmp_path, pokedex, poke_count, &loaded, &badges) == 0);
    assert(badges == 0x05);
    assert(loaded.count == 1);
    assert(strcmp(loaded.pokemons[0].name, "Pikachu") == 0);
    assert(loaded.pokemons[0].hp == 17);
    assert(loaded.pokemons[0].status == STATUS_POISON);
    assert(loaded.pokemons[0].moves[0].pp == 12);

    remove(tmp_path);
    free_pokemons(pokedex);
    free_moves(moves);
}

int main(void)
{
    srand(42);
    printf("Running PokeArena unit tests:\n");
    RUN_TEST(test_utils);
    RUN_TEST(test_types_gen4);
    RUN_TEST(test_loaders);
    RUN_TEST(test_team_management);
    RUN_TEST(test_battle_damage_and_status);
    RUN_TEST(test_ai_modes);
    RUN_TEST(test_save_and_load);
    printf("All %d test suites passed!\n", tests_run);
    return 0;
}
