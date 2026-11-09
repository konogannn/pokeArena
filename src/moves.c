#include "moves.h"
#include "tsv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Move *items;
    int count;
    int capacity;
} MoveBuffer;

/* --- Technical buffer helper --- */

static int append_move(MoveBuffer *buf, const Move *move) {
    int new_cap = 0;
    Move *tmp = NULL;

    if (buf->count == buf->capacity) {
        new_cap = buf->capacity == 0 ? 16 : buf->capacity * 2;
        tmp = realloc(buf->items, (size_t)new_cap * sizeof(Move));
        if (!tmp) {
            return -1;
        }
        buf->items = tmp;
        buf->capacity = new_cap;
    }
    buf->items[buf->count++] = *move;
    return 0;
}

/* --- Business logic --- */

static int has_move_name(const MoveBuffer *buf, const char *name) {
    for (int i = 0; i < buf->count; i++) {
        if (strcmp(buf->items[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_valid_move(const Move *move) {
    if (move->name[0] == '\0') {
        return 0;
    }
    if (move->power <= 0) {
        return 0;
    }
    if (move->accuracy < 0 || move->accuracy > 100) {
        return 0;
    }
    return 1;
}

static int parse_move(char **fields, Move *out) {
    snprintf(out->name, sizeof(out->name), "%s", fields[0]);
    if (tsv_parse_int(fields[1], &out->power) != 0) {
        return -1;
    }
    if (tsv_parse_int(fields[2], &out->accuracy) != 0) {
        return -1;
    }
    return type_from_string(fields[3], &out->type);
}

static int add_move_row(int field_count, char **fields, void *userdata) {
    MoveBuffer *buf = userdata;
    Move move;

    if (field_count < 4) {
        return -1;
    }
    if (parse_move(fields, &move) != 0 || !is_valid_move(&move)) {
        return -1;
    }
    if (has_move_name(buf, move.name)) {
        return -1;
    }
    return append_move(buf, &move);
}

Move *load_moves(const char *path, int *out_count) {
    MoveBuffer buf = {NULL, 0, 0};

    if (out_count) {
        *out_count = 0;
    }
    if (!path) {
        return NULL;
    }
    if (tsv_parse_file(path, add_move_row, &buf) < 0) {
        free(buf.items);
        return NULL;
    }
    if (out_count) {
        *out_count = buf.count;
    }
    return buf.items;
}

void free_moves(Move *moves) {
    free(moves);
}
