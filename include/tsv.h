#pragma once

#define TSV_MAX_FIELDS 16
#define TSV_MAX_LINE   1024

typedef int (*TsvRowCallback)(int field_count, char **fields, void *userdata);

/* Technical parsing helpers */
char *tsv_trim(char *str);
int tsv_parse_int(const char *str, int *out_val);

/* Parse a TSV file line by line, skipping blank lines and '#' comments.
 * Leading and trailing whitespace is trimmed from each tab-separated field.
 * Returns the number of rows parsed, or -1 on error. */
int tsv_parse_file(const char *path, TsvRowCallback callback, void *userdata);
