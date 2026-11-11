#include "ai.h"
#include "battle.h"

#include <stdlib.h>
#include <string.h>

const char *ai_difficulty_to_string(enum AiDifficulty difficulty)
{
    switch (difficulty)
    {
        case AI_EASY:
            return "Easy";
        case AI_MEDIUM:
            return "Medium";
        case AI_HARD:
            return "Hard";
        default:
            return "Medium";
    }
}

Team ai_generate_team(const Pokemon *pokedex, int pokedex_count, int count)
{
    Team team;

    memset(&team, 0, sizeof(team));
    if (!pokedex || pokedex_count <= 0 || count <= 0)
    {
        return team;
    }

    int limit = count > 6 ? 6 : count;
    for (int i = 0; i < limit; i++)
    {
        add_pokemon(&team, pokedex[rand() % pokedex_count]);
    }
    return team;
}

static int evaluate_move(const Pokemon *opponent, const Pokemon *player, Move move,
                         enum AiDifficulty difficulty)
{
    double eff = type_effectiveness(move.type, player->type);

    if (move.power == 0 && move.effect == EFFECT_HEAL)
    {
        if (difficulty == AI_HARD && opponent->hp * 2 <= opponent->max_hp)
        {
            return 120;
        }
        return 5;
    }
    if (eff == 0.0)
    {
        return -1;
    }

    int dmg = calculate_damage(*opponent, *player, move);
    int score = (dmg * move.accuracy) / 100;

    if (eff > 1.0)
    {
        score += 20;
    }
    else if (eff < 1.0)
    {
        score -= 10;
    }

    if (dmg >= player->hp)
    {
        score += 1000 + move.accuracy;
    }
    else if (difficulty == AI_HARD && player->hp * 3 <= player->max_hp && dmg * 2 >= player->hp)
    {
        score += 200;
    }

    if (difficulty == AI_HARD && player->status == STATUS_NONE &&
        (move.effect == EFFECT_POISON || move.effect == EFFECT_BURN ||
         move.effect == EFFECT_PARALYSIS))
    {
        score += move.effect_chance / 3;
    }
    return score < 0 ? 0 : score;
}

static Move pick_random_move(const Pokemon *opponent)
{
    int idx = opponent->move_count > 1 ? rand() % opponent->move_count : 0;
    return opponent->moves[idx];
}

Move ai_choose_move(const Pokemon *opponent, const Pokemon *player, enum AiDifficulty difficulty)
{
    if (!opponent || opponent->move_count <= 0)
    {
        Move empty;
        memset(&empty, 0, sizeof(empty));
        return empty;
    }
    if (!player || difficulty == AI_EASY)
    {
        return pick_random_move(opponent);
    }

    int best_idx = 0;
    int best_score = -2;
    for (int i = 0; i < opponent->move_count; i++)
    {
        int s = evaluate_move(opponent, player, opponent->moves[i], difficulty);
        if (s > best_score)
        {
            best_score = s;
            best_idx = i;
        }
    }

    if (difficulty == AI_MEDIUM && opponent->move_count > 1 && rand() % 4 == 0)
    {
        int alt = rand() % opponent->move_count;
        if (type_effectiveness(opponent->moves[alt].type, player->type) > 0.0)
        {
            return opponent->moves[alt];
        }
    }
    return opponent->moves[best_idx];
}

static int score_matchup(const Pokemon *candidate, const Pokemon *player)
{
    if (!is_pokemon_alive(*candidate) || candidate->max_hp <= 0)
    {
        return -1000;
    }

    int hp_pct = (candidate->hp * 100) / candidate->max_hp;
    int score = hp_pct;

    if (player)
    {
        double opp_off = type_effectiveness(candidate->type, player->type);
        double player_off = type_effectiveness(player->type, candidate->type);

        if (opp_off > 1.0)
        {
            score += 40;
        }
        else if (opp_off < 1.0)
        {
            score -= 20;
        }
        if (player_off > 1.0)
        {
            score -= 40;
        }
        else if (player_off < 1.0)
        {
            score += 25;
        }
    }
    return score;
}

int ai_choose_switch(const Team *opponent_team, const Pokemon *player, enum AiDifficulty difficulty)
{
    if (!opponent_team)
    {
        return -1;
    }
    if (difficulty != AI_HARD || !player)
    {
        for (int i = 0; i < opponent_team->count; i++)
        {
            if (is_pokemon_alive(opponent_team->pokemons[i]))
            {
                return i;
            }
        }
        return -1;
    }

    int best_idx = -1;
    int best_score = -10000;
    for (int i = 0; i < opponent_team->count; i++)
    {
        if (!is_pokemon_alive(opponent_team->pokemons[i]))
        {
            continue;
        }
        int s = score_matchup(&opponent_team->pokemons[i], player);
        if (s > best_score)
        {
            best_score = s;
            best_idx = i;
        }
    }
    return best_idx;
}

int ai_should_switch(const Team *opponent_team, int active_idx,
                     const Pokemon *player, enum AiDifficulty difficulty)
{
    if (difficulty != AI_HARD || !opponent_team || !player ||
        active_idx < 0 || active_idx >= opponent_team->count)
    {
        return -1;
    }

    const Pokemon *active = &opponent_team->pokemons[active_idx];
    if (!is_pokemon_alive(*active))
    {
        return -1;
    }

    int low_hp = active->hp * 4 <= active->max_hp;
    int bad_type = type_effectiveness(player->type, active->type) > 1.0;
    if (!low_hp && !bad_type)
    {
        return -1;
    }

    int active_score = score_matchup(active, player);
    int best_idx = -1;
    int best_score = active_score + 25;

    for (int i = 0; i < opponent_team->count; i++)
    {
        if (i == active_idx || !is_pokemon_alive(opponent_team->pokemons[i]))
        {
            continue;
        }
        const Pokemon *cand = &opponent_team->pokemons[i];
        if (cand->hp * 2 <= cand->max_hp)
        {
            continue;
        }
        int s = score_matchup(cand, player);
        if (s > best_score)
        {
            best_score = s;
            best_idx = i;
        }
    }
    return best_idx;
}
