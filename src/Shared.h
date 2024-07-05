#pragma once

enum PacketType {
    CLIENT_CONNECT = 0,
    SERVER_DATA,
    CLIENT_DISCONNECT, // Also sends the id disconnected, so all the ids above that id should be itself-1
    PLAYER_INPUT,
};