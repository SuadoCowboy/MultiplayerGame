#pragma once

namespace rl {
    #include <raylib.h>
}

#include "Shared.h"
#ifdef CLIENT
#include <string>
#include "Network.h"
#endif

#define PLAYER_VELOCITY 8.0f

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
#endif

    void update(
#ifdef CLIENT
        ENetPeer* serverPeer,
        const float& dt,
        const float& tickRate
#else
        const enet_uint16& timesToTick
#endif
        ) {
#ifdef CLIENT
        if (serverPeer != nullptr) {
            enet_uint8 newDir = 0;

            if (rl::IsKeyDown(rl::KEY_W))
                newDir |= 1;
            if (rl::IsKeyDown(rl::KEY_S))
                newDir |= 2;
            
            if (rl::IsKeyDown(rl::KEY_A))
                newDir |= 4;
            if (rl::IsKeyDown(rl::KEY_D))
               newDir |= 8;
            
            if (newDir != dir) {
                dir = newDir;

                Packet packet;
                packet << (enet_uint8)PLAYER_INPUT
                       << dir
                       << rl::GetTime();

                sendPacket(serverPeer, packet, false, 1);
                packet.deleteData();
            }
        }
#endif
        rect.x += PLAYER_VELOCITY * ((dir & 8) * 0.125 - (dir & 4) * 0.25)
#ifdef CLIENT
        * dt * tickRate
#else
        * timesToTick
#endif
        ;

        rect.y += PLAYER_VELOCITY * ((dir & 2) * 0.5 - (dir & 1))
#ifdef CLIENT
        * dt * tickRate
#else
        * timesToTick
#endif
        ;
    }
};