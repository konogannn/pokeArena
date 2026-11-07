#pragma once

#include "type.h"

typedef struct {
    char name[32];
    int power;
    int accuracy;
    Type type;
} Move;
