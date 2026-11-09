#include "tsv.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *tsv_trim(char *str) {
    while (*str && isspace((unsigned char)*str)) {
        str++;
    }
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\0';
    }
    return str;
}

int tsv_parse_int(const char *str, int *out_val) {
    if (!str || *str == '\0' || !out_val) {
        return -1;
    }

    char *end = NULL;
    errno = 0;
    long val = strtol(str, &end, 10);
    if (errno != 0 || *end != '\0' || val < INT_MIN || val > INT_MAX) {
        return -1;
    }

    *out_val = (int)val;
    return 0;
}

static int is_blank_or_comment(const char *line) {
    while (*line && isspace((unsigned char)*line)) {
        line++;
    }
    return (*line == '\0' || *line == '#');
}

static int split_fields(char *line, char **fields, int max_fields) {
    int count = 0;
    char *cursor = line;

    while (count < max_fields) {
        char *tab = strchr(cursor, '\t');
        if (tab) {
            *tab = '\0';
        }
        fields[count++] = tsv_trim(cursor);
        if (!tab) {
            break;
        }
        cursor = tab + 1;
    }
    return count;
}

static int parse_line(char *line, TsvRowCallback callback, void *userdata) {
    if (is_blank_or_comment(line)) {
        return 0;
    }

    char *fields[TSV_MAX_FIELDS];
    int count = split_fields(line, fields, TSV_MAX_FIELDS);
    return callback(count, fields, userdata) == 0 ? 1 : -1;
}

int tsv_parse_file(const char *path, TsvRowCallback callback, void *userdata) {
    if (!path || !callback) {
        return -1;
    }

    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }

    char line[TSV_MAX_LINE];
    int rows = 0;
    while (fgets(line, sizeof(line), fp)) {
        int status = parse_line(line, callback, userdata);
        if (status < 0) {
            fclose(fp);
            return -1;
        }
        rows += status;
    }

    fclose(fp);
    return rows;
}
