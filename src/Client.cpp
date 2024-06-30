#include <iostream>
#include <unordered_map>
#include <cstdint>

#include <enet/enet.h>

namespace rl {
    #include <raylib.h>
}

#include "Player.h"
#include "Packet.h"
#include "Constants.h"
#include "ClientsHandler.h"

#define PORT 5055

enet_uint8 clientId;
std::unordered_map<enet_uint8, Player> players;

void disconnect(ENetHost* client, ENetEvent& event, ENetPeer* peer) {
    enet_peer_disconnect(peer, 0);

    enet_uint8 disconnected = false;
    /* Allow up to 3 seconds for the disconnect to succeed
    * and drop any received packets.
    */
    while (enet_host_service(client, &event, 3000) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            puts("Disconnection succeeded.");
            disconnected = true;
            break;
        default:
            break;
        }
    }

    // Drop connection, since disconnection didn't successed
    if (!disconnected)
        enet_peer_reset(peer);
}

int main() {
    if (enet_initialize() != 0) {
        std::cerr << "An error occurred while initializing ENet." << std::endl;
        exit(EXIT_FAILURE);
    }
    atexit(enet_deinitialize);

    ENetHost* client = enet_host_create(NULL, 1, 2, 0, 0);
    if (client == NULL) {
        std::cerr << "An error occurred while trying to create an ENet client host." << std::endl;
        exit(EXIT_FAILURE);
    }

    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = PORT;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL) {
        std::cerr << "No available peers for initiating an ENet connection." << std::endl;
        exit(EXIT_FAILURE);
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) <= 0 || event.type != ENET_EVENT_TYPE_CONNECT) {
        enet_peer_reset(peer);
        puts("Connection to server failed.");
        exit(EXIT_FAILURE);
    }

    while (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
        size_t dataSize = 0;
        
        PacketType packetType;
        char* data;
        {
            enet_uint8 tempPacketType = NONE;
            data = parsePacket(event.packet, tempPacketType, dataSize);
            packetType = (PacketType)tempPacketType;
        }
        
        if (packetType == NONE) {
            enet_peer_reset(peer);
            std::cerr << "ERROR WHILE PARSING PACKET => TYPE IS NONE" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (packetType == CLIENT_CONNECT && dataSize != 0) {
            clientId = *(enet_uint8*)data;
            size_t offset = sizeOfEnet_uint8;

            players[clientId] = {};

            players[clientId].color.r = *(enet_uint8*)(data+offset); 
            offset += sizeOfEnet_uint8;

            players[clientId].color.g = *(enet_uint8*)(data+offset); 
            offset += sizeOfEnet_uint8;

            players[clientId].color.b = *(enet_uint8*)(data+offset); 
            offset += sizeOfEnet_uint8;

            players[clientId].color.a = 255;

            players[clientId].rect = *(rl::Rectangle*)(data+offset);
            //offset += sizeOfFloat*4;
            
            std::cout << "DATA SIZE: " << dataSize << "\nCONNECTED => ID: " << (enet_uint16)clientId << " | COLOR: "
                      << (enet_uint16)players[clientId].color.r << ", "
                      << (enet_uint16)players[clientId].color.g << ", "
                      << (enet_uint16)players[clientId].color.b << ", "
                      << (enet_uint16)players[clientId].color.a
                      << " | POSITION: "
                      << players[clientId].rect.x << ", "
                      << players[clientId].rect.y
                      << " | SIZE: "
                      << players[clientId].rect.width << ", "
                      << players[clientId].rect.height
                      << std::endl;
        } else {
            enet_peer_reset(peer);
            std::cerr << "ERROR WHILE PARSING PACKET DATA => TYPE: " << packetType << " | SIZE: " << dataSize << std::endl;
            exit(EXIT_FAILURE);
        }

        enet_packet_destroy(event.packet);
    }

    disconnect(client, event, peer);
    enet_host_destroy(client);
    enet_deinitialize();
}