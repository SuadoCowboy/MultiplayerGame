#pragma once

namespace rl {
    #include <raylib.h>
}

struct Player {
    rl::Rectangle rect = {0,0,0,0};
    rl::Color color = {0,0,0,0};
};