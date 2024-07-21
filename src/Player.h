#pragma once

namespace rl {
    #include <raylib.h>
}

#define PLAYER_VELOCITY 8.0f

#include "Shared.h"
#ifdef CLIENT
#include <string>

#include <enet/enet.h>

#include "Network.h"
#endif



struct Player {
    rl::Rectangle rect = {0.0f,0.0f,0.0f,0.0f};
    rl::Color color = {0,0,0,0};

    /*
    1 = up
    2 = down
    4 = left
    8 = right
    */
    enet_uint8 dir = 0;

#ifdef CLIENT
    std::string idText;
    rl::Vector2 idTextPosition = {0,0};
    float idTextWidthHalf = 0;
    float idTextFontSize = 0;

    void setIdText(const std::string& _idText, const float& _idTextFontSize) {
        idText = _idText;
        idTextFontSize = _idTextFontSize;

        idTextWidthHalf = rl::MeasureText(idText.c_str(), idTextFontSize)*0.5f;
    }
#else
    enet_uint8 oldDir = 0;
#endif

    void update(
#ifdef CLIENT
            const float& dt
#endif
    ) {
        rect.x += PLAYER_VELOCITY * ((dir & 8) * 0.125 - (dir & 4) * 0.25)
#ifdef CLIENT
        * dt
#endif
        ;

        rect.y += PLAYER_VELOCITY * ((dir & 2) * 0.5 - (dir & 1))
#ifdef CLIENT
        * dt
#endif
        ;
    }
};