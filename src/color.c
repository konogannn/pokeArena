#define _POSIX_C_SOURCE 200809L

#include "color.h"
#include "type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int color_is_supported(void)
{
    const char *no_color = getenv("NO_COLOR");
    const char *term = getenv("TERM");

    if (no_color && no_color[0] != '\0')
    {
        return 0;
    }
    if (!isatty(fileno(stdout)))
    {
        return 0;
    }
    if (!term || strcmp(term, "dumb") == 0)
    {
        return 0;
    }
    return 1;
}

const char *color_code(const char *ansi)
{
    if (!ansi || !color_is_supported())
    {
        return "";
    }
    return ansi;
}

const char *color_for_type(int type)
{
    switch (type)
    {
    case TYPE_FIRE:
    case TYPE_FIGHTING:
        return ANSI_RED;
    case TYPE_WATER:
    case TYPE_DRAGON:
        return ANSI_BLUE;
    case TYPE_GRASS:
    case TYPE_BUG:
        return ANSI_GREEN;
    case TYPE_ELECTRIC:
    case TYPE_GROUND:
    case TYPE_ROCK:
        return ANSI_YELLOW;
    case TYPE_POISON:
    case TYPE_PSYCHIC:
    case TYPE_GHOST:
        return ANSI_MAGENTA;
    case TYPE_ICE:
    case TYPE_FLYING:
    case TYPE_STEEL:
        return ANSI_CYAN;
    default:
        return ANSI_RESET;
    }
}

void print_colored(const char *ansi, const char *text)
{
    if (!text)
    {
        return;
    }
    printf("%s%s%s", color_code(ansi), text, color_code(ANSI_RESET));
}

void sleep_ms(int ms)
{
    if (ms <= 0 || !color_is_supported())
    {
        return;
    }

    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000);
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

void print_slow(const char *ansi, const char *text, int delay_ms)
{
    if (!text)
    {
        return;
    }
    if (!color_is_supported() || delay_ms <= 0)
    {
        print_colored(ansi, text);
        return;
    }

    printf("%s", color_code(ansi));
    for (const char *p = text; *p; p++)
    {
        putchar(*p);
        fflush(stdout);
        sleep_ms(delay_ms);
    }
    printf("%s", color_code(ANSI_RESET));
    fflush(stdout);
}
