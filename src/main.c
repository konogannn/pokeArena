#include "moves.h"
#include "pokemon.h"
#include "report.h"
#include "team.h"
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(void)
{
    srand((unsigned int)time(NULL));

    int move_count = 0;
    Move *moves = load_moves("data/moves.tsv", &move_count);
    if (!moves)
    {
        fprintf(stderr, "Failed to load data/moves.tsv\n");
        return 1;
    }

    int pokemon_count = 0;
    Pokemon *pokemons = load_pokemons("data/pokemon.tsv", moves, move_count, &pokemon_count);
    if (!pokemons)
    {
        fprintf(stderr, "Failed to load data/pokemon.tsv\n");
        free_moves(moves);
        return 1;
    }

    Team team;
    memset(&team, 0, sizeof(team));
    BattleReport report;
    report_init(&report);

    run_game_loop(&pokemons, &pokemon_count, &moves, &move_count, &team, &report);

    if (report_write_file("battle_report.txt", &report) == 0)
    {
        printf("Battle report saved to 'battle_report.txt'.\n");
    }

    free_pokemons(pokemons);
    free_moves(moves);
    return 0;
}
