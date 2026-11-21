#include "team.h"

#include <stdio.h>

void add_pokemon(Team *team, Pokemon pokemon)
{
    if (!team)
    {
        return;
    }
    if (team->count < 0 || team->count >= 6)
    {
        fprintf(stderr, "team: cannot add '%s', team is full (max 6)\n", pokemon.name);
        return;
    }
    team->pokemons[team->count++] = pokemon;
}

void remove_pokemon(Team *team, int index)
{
    if (!team)
    {
        return;
    }
    if (index < 0 || index >= team->count)
    {
        fprintf(stderr, "team: invalid index %d (count=%d)\n", index, team->count);
        return;
    }
    for (int i = index; i < team->count - 1; i++)
    {
        team->pokemons[i] = team->pokemons[i + 1];
    }
    team->count--;
}

void swap_pokemon(Team *team, int i, int j)
{
    if (!team)
    {
        return;
    }
    if (i < 0 || i >= team->count || j < 0 || j >= team->count)
    {
        fprintf(stderr, "team: invalid swap indices %d, %d (count=%d)\n", i, j, team->count);
        return;
    }
    if (i == j)
    {
        return;
    }
    Pokemon tmp = team->pokemons[i];
    team->pokemons[i] = team->pokemons[j];
    team->pokemons[j] = tmp;
}

void heal_team(Team *team)
{
    if (!team)
    {
        return;
    }
    for (int i = 0; i < team->count; i++)
    {
        team->pokemons[i].hp = team->pokemons[i].max_hp;
        team->pokemons[i].status = STATUS_NONE;
        for (int j = 0; j < team->pokemons[i].move_count; j++)
        {
            team->pokemons[i].moves[j].pp = team->pokemons[i].moves[j].max_pp;
        }
    }
}

int is_team_valid(Team team)
{
    if (team.count < 1 || team.count > 6)
    {
        return 0;
    }
    for (int i = 0; i < team.count; i++)
    {
        if (team.pokemons[i].name[0] == '\0' || team.pokemons[i].max_hp <= 0)
        {
            return 0;
        }
    }
    return 1;
}
