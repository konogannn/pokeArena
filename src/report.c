#include "report.h"

#include <stdio.h>
#include <string.h>

void report_init(BattleReport *report)
{
    if (!report)
    {
        return;
    }
    memset(report, 0, sizeof(*report));
}

static const char *battle_outcome_string(const Battle *battle)
{
    if (battle->forfeited)
    {
        return "Forfeit";
    }
    if (is_team_defeated(battle->player_team))
    {
        return "Loss";
    }
    if (is_team_defeated(battle->opponent_team))
    {
        return "Win";
    }
    return "Unfinished";
}

void report_record_battle(BattleReport *report, const char *label, const Battle *battle)
{
    if (!report || !battle)
    {
        return;
    }

    const char *outcome = battle_outcome_string(battle);
    if (strcmp(outcome, "Win") == 0)
    {
        report->wins++;
    }
    else if (strcmp(outcome, "Loss") == 0)
    {
        report->losses++;
    }
    else if (strcmp(outcome, "Forfeit") == 0)
    {
        report->forfeits++;
    }

    report->total_turns += battle->turn;
    report->total_damage_dealt += battle->damage_dealt;
    report->total_damage_taken += battle->damage_taken;
    report->total_kos_inflicted += battle->kos_inflicted;
    report->total_kos_suffered += battle->kos_suffered;

    if (report->total_battles < REPORT_MAX_BATTLES)
    {
        BattleRecord *rec = &report->records[report->total_battles];
        snprintf(rec->label, sizeof(rec->label), "%s", label ? label : "Battle");
        snprintf(rec->outcome, sizeof(rec->outcome), "%s", outcome);
        rec->turns = battle->turn;
        rec->damage_dealt = battle->damage_dealt;
        rec->damage_taken = battle->damage_taken;
        rec->kos_inflicted = battle->kos_inflicted;
        rec->kos_suffered = battle->kos_suffered;
    }
    report->total_battles++;
}

int report_write_file(const char *path, const BattleReport *report)
{
    if (!path || !report)
    {
        return -1;
    }

    FILE *fp = fopen(path, "w");
    if (!fp)
    {
        fprintf(stderr, "report: cannot open '%s' for writing\n", path);
        return -1;
    }

    fprintf(fp, "=== PokeArena Battle Report ===\n\n");
    fprintf(fp, "Summary:\n");
    fprintf(fp, "  Total Battles : %d\n", report->total_battles);
    fprintf(fp, "  Wins          : %d\n", report->wins);
    fprintf(fp, "  Losses        : %d\n", report->losses);
    fprintf(fp, "  Forfeits      : %d\n", report->forfeits);
    if (report->total_battles > 0)
    {
        double win_rate = (double)report->wins * 100.0 / (double)report->total_battles;
        fprintf(fp, "  Win Rate      : %.1f%%\n", win_rate);
    }
    fprintf(fp, "  Total Turns   : %d\n", report->total_turns);
    fprintf(fp, "  Damage Dealt  : %d\n", report->total_damage_dealt);
    fprintf(fp, "  Damage Taken  : %d\n", report->total_damage_taken);
    fprintf(fp, "  KOs Inflicted : %d\n", report->total_kos_inflicted);
    fprintf(fp, "  KOs Suffered  : %d\n", report->total_kos_suffered);

    if (report->total_battles > 0)
    {
        int shown = report->total_battles < REPORT_MAX_BATTLES
                        ? report->total_battles
                        : REPORT_MAX_BATTLES;
        fprintf(fp, "\nBattle Log:\n");
        for (int i = 0; i < shown; i++)
        {
            const BattleRecord *rec = &report->records[i];
            fprintf(fp, "  #%d %-28s | %-7s | Turns: %2d | Dmg: %4d/%-4d | KO: %d/%d\n",
                    i + 1, rec->label, rec->outcome, rec->turns,
                    rec->damage_dealt, rec->damage_taken,
                    rec->kos_inflicted, rec->kos_suffered);
        }
    }

    if (fclose(fp) != 0)
    {
        fprintf(stderr, "report: failed closing '%s'\n", path);
        return -1;
    }
    return 0;
}
