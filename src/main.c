#include "moves.h"
#include "pokemon.h"

#include <stdio.h>

int main(void)
{
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

    printf("Loaded %d moves and %d pokemon:\n", move_count, pokemon_count);
    for (int i = 0; i < pokemon_count; i++)
    {
        printf("  %-12s type=%-13s hp=%3d atk=%2d def=%2d spd=%3d moves=%d\n", pokemons[i].name,
               type_to_string(pokemons[i].type), pokemons[i].hp, pokemons[i].attack,
               pokemons[i].defense, pokemons[i].speed, pokemons[i].move_count);
    }

    free_pokemons(pokemons);
    free_moves(moves);
    return 0;
}
