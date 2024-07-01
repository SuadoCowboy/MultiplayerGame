#include <iostream>
#include <unordered_map>
#include <cstdint>

#include <enet/enet.h>

namespace rl {
    #include <raylib.h>
}

#include "Player.h"
#include "Packet.h"
#include "ClientsHandler.h"
#include "Shared.h"

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
    if (!disconnected) {
        puts("Dropped connection because server did not respond on time.");
        enet_peer_reset(peer);
    }
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
        if (event.packet->dataLength < sizeof(enet_uint8)) {
            std::cout << "ERROR WHILE RECEIVING PACKET => DATA LENGTH: " << event.packet->dataLength << std::endl;
            enet_peer_reset(peer);
            enet_packet_destroy(event.packet);
            exit(EXIT_FAILURE);
        }

        PacketUnwrapper packetUnwrapper((const char*)event.packet->data);

        PacketType packetType;
        {
            enet_uint8 tempPacketType;
            packetUnwrapper >> tempPacketType;
            packetType = (PacketType)tempPacketType;
        }

        if (packetType == NONE) {
            std::cerr << "ERROR WHILE PARSING PACKET => TYPE IS NONE" << std::endl;
            enet_peer_reset(peer);
            enet_packet_destroy(event.packet);
            exit(EXIT_FAILURE);
        }

        if (packetType == CLIENT_CONNECT) {
            packetUnwrapper >> clientId;

            players[clientId] = {};
            packetUnwrapper >> players[clientId].color.r
                            >> players[clientId].color.g
                            >> players[clientId].color.b;
            
            players[clientId].color.a = 255;

            packetUnwrapper >> players[clientId].rect;
            
            std::cout << "DATA SIZE: " << event.packet->dataLength << "\nCONNECTED => ID: " << (enet_uint16)clientId << " | COLOR: "
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
            std::cerr << "ERROR WHILE PARSING PACKET DATA => TYPE: " << packetType << " | SIZE: " << event.packet->dataLength << std::endl;
            enet_peer_reset(peer);
            enet_packet_destroy(event.packet);
            exit(EXIT_FAILURE);
        }

        enet_packet_destroy(event.packet);
    }

    disconnect(client, event, peer);
    enet_host_destroy(client);
    enet_deinitialize();
}