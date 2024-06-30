#pragma once

#include <enet/enet.h>

enum PacketType {
    NONE = 0,
    CLIENT_CONNECT,
    CLIENT_DISCONNECT, // Also sends the id disconnected, so all the ids above that id should be itself-1
    PLAYER_INPUT,
};

void sendPacket(ENetPeer* peer, const enet_uint8& packetType, const char* data, const size_t& size, const bool& reliable, const int& channel);
void broadcastPacket(ENetHost* host, const enet_uint8& packetType, const char* data, const size_t& size, const bool& reliable, const int& channel);

/// @param packetType (out) PacketType as enet_uint8
/// @param dataSize (out) data size
/// @return data
char* parsePacket(const ENetPacket* packet, enet_uint8& packetType, size_t& dataSize);