#include "main.h"
#include "sd_manager.h"
#include <stdint.h>

Recipe_t activeRecipe = {
    .count = 3,
    .features = {
        {333.0f,  0, 1, 0},
        {762.0f,  0, 1, 0},
        {1095.0f, 0, 0, 1}
    }
};

uint16_t recipeSize = 3;