#include "save.h"
#include "tsv.h"

#include <stdio.h>
#include <string.h>

typedef struct
{
    Team team;
    const Pokemon *pokedex;
    int pokedex_count;
} TeamLoadCtx;

static const Pokemon *find_in_pokedex(const Pokemon *pokedex, int count, const char *name)
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

static int add_saved_pokemon(int field_count, char **fields, void *userdata)
{
    TeamLoadCtx *ctx = userdata;

    if (field_count < 1 || fields[0][0] == '\0')
    {
        fprintf(stderr, "save: missing pokemon name\n");
        return -1;
    }
    if (ctx->team.count >= 6)
    {
        fprintf(stderr, "save: team cannot exceed 6 pokemon\n");
        return -1;
    }

    const Pokemon *base = find_in_pokedex(ctx->pokedex, ctx->pokedex_count, fields[0]);
    if (!base)
    {
        fprintf(stderr, "save: unknown pokemon '%s'\n", fields[0]);
        return -1;
    }

    Pokemon p = *base;
    if (field_count >= 2)
    {
        int hp = 0;
        if (tsv_parse_int(fields[1], &hp) != 0 || hp < 0 || hp > p.max_hp)
        {
            fprintf(stderr, "save: invalid hp '%s' for '%s'\n", fields[1], p.name);
            return -1;
        }
        p.hp = hp;
    }
    if (field_count >= 3)
    {
        int st = 0;
        if (tsv_parse_int(fields[2], &st) != 0 || st < 0 || st >= STATUS_COUNT)
        {
            fprintf(stderr, "save: invalid status '%s' for '%s'\n", fields[2], p.name);
            return -1;
        }
        p.status = (enum Status)st;
    }

    add_pokemon(&ctx->team, p);
    return 0;
}

int save_team(const char *path, const Team *team)
{
    if (!path || !team)
    {
        fprintf(stderr, "save: invalid arguments to save_team\n");
        return -1;
    }

    FILE *fp = fopen(path, "w");
    if (!fp)
    {
        fprintf(stderr, "save: cannot open '%s' for writing\n", path);
        return -1;
    }

    fprintf(fp, "# name\thp\tstatus\n");
    for (int i = 0; i < team->count; i++)
    {
        const Pokemon *p = &team->pokemons[i];
        fprintf(fp, "%s\t%d\t%d\n", p->name, p->hp, (int)p->status);
    }

    if (fclose(fp) != 0)
    {
        fprintf(stderr, "save: failed closing '%s'\n", path);
        return -1;
    }
    return 0;
}

int load_team(const char *path, const Pokemon *pokedex, int pokedex_count, Team *out_team)
{
    if (!path || !pokedex || pokedex_count <= 0 || !out_team)
    {
        fprintf(stderr, "save: invalid arguments to load_team\n");
        return -1;
    }

    TeamLoadCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.pokedex = pokedex;
    ctx.pokedex_count = pokedex_count;

    if (tsv_parse_file(path, add_saved_pokemon, &ctx) < 0)
    {
        return -1;
    }

    *out_team = ctx.team;
    return 0;
}
