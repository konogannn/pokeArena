#include "utils.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

char *str_trim(char *str)
{
    while (*str && isspace((unsigned char)*str))
    {
        str++;
    }
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1]))
    {
        str[--len] = '\0';
    }
    return str;
}

int str_parse_int(const char *str, int *out_val)
{
    if (!str || *str == '\0' || !out_val)
    {
        return -1;
    }

    char *end = NULL;
    errno = 0;
    long val = strtol(str, &end, 10);
    if (errno != 0 || *end != '\0' || val < INT_MIN || val > INT_MAX)
    {
        return -1;
    }

    *out_val = (int)val;
    return 0;
}

int str_equals_icase(const char *a, const char *b)
{
    if (!a || !b)
    {
        return 0;
    }
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
