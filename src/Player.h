#pragma once

#include <iostream>
#include <cstring>

namespace rl {
    #include <raylib.h>
}

#include "Shared.h"
#ifdef CLIENT
#include "Packet.h"
#endif

#define PLAYER_VELOCITY 20.0f

struct Player {
    rl::Rectangle rect = {0.0f,0.0f,0.0f,0.0f};
    rl::Color color = {0,0,0,0};

    Vector2uc dir = {1,1};

#ifdef CLIENT
    std::string idText;
    rl::Vector2 idTextPosition = {0,0};
    float idTextWidthHalf = 0;
    float idTextFontSize = 0;

    void setIdText(const std::string& _idText, const float& _idTextFontSize) {
        idText = _idText;
        idTextFontSize = _idTextFontSize;

        idTextWidthHalf = rl::MeasureText(idText.c_str(), idTextFontSize)*0.5f;
        update(nullptr);
    }
#endif

    void update(
#ifdef CLIENT
        ENetPeer* serverPeer
#endif
        ) {
#ifdef CLIENT
        if (serverPeer != nullptr) {
            Vector2uc newDir = {1,1};

            if (rl::IsKeyDown(rl::KEY_W))
                newDir.y--;
            if (rl::IsKeyDown(rl::KEY_S))
                newDir.y++;
            
            if (rl::IsKeyDown(rl::KEY_A))
                newDir.x--;
            if (rl::IsKeyDown(rl::KEY_D))
                newDir.x++;
            
            if (newDir.x != dir.x || newDir.y != dir.y) {
                dir.x = newDir.x;
                dir.y = newDir.y;

                Packet packet;
                packet << (enet_uint8)PLAYER_INPUT
                    << dir;
                
                sendPacket(serverPeer, packet, false, 1);
                packet.deleteData();
            }
        }
        
        idTextPosition = {rect.x+rect.width*0.5f-idTextWidthHalf, rect.y-idTextFontSize-2};
#endif
        rect.x += PLAYER_VELOCITY * ((float)dir.x - 1.0f);
        rect.y += PLAYER_VELOCITY * ((float)dir.y - 1.0f);
    }
};