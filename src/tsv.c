#include "tsv.h"
#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int is_blank_or_comment(const char *line)
{
    while (*line && isspace((unsigned char)*line))
    {
        line++;
    }
    return (*line == '\0' || *line == '#');
}

static int split_fields(char *line, char **fields, int max_fields)
{
    int count = 0;
    char *cursor = line;

    while (count < max_fields)
    {
        char *tab = strchr(cursor, '\t');
        if (tab)
        {
            *tab = '\0';
        }
        fields[count++] = str_trim(cursor);
        if (!tab)
        {
            break;
        }
        cursor = tab + 1;
    }
    return count;
}

static int parse_line(char *line, TsvRowCallback callback, void *userdata)
{
    if (is_blank_or_comment(line))
    {
        return 0;
    }

    char *fields[TSV_MAX_FIELDS];
    int count = split_fields(line, fields, TSV_MAX_FIELDS);
    return callback(count, fields, userdata) == 0 ? 1 : -1;
}

int tsv_parse_file(const char *path, TsvRowCallback callback, void *userdata)
{
    if (!path || !callback)
    {
        return -1;
    }

    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        fprintf(stderr, "tsv: cannot open file '%s'\n", path);
        return -1;
    }

    char line[TSV_MAX_LINE];
    int rows = 0;
    int line_num = 0;
    while (fgets(line, sizeof(line), fp))
    {
        line_num++;
        int status = parse_line(line, callback, userdata);
        if (status < 0)
        {
            fprintf(stderr, "tsv: failed parsing '%s' at line %d\n", path, line_num);
            fclose(fp);
            return -1;
        }
        rows += status;
    }

    fclose(fp);
    return rows;
}
