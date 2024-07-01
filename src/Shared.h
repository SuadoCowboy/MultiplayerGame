#pragma once

enum PacketType {
    NONE = 0,
    CLIENT_CONNECT,
    CLIENT_DISCONNECT, // Also sends the id disconnected, so all the ids above that id should be itself-1
    PLAYER_INPUT,
};