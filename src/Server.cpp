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
#include "Constants.h"

#define MAX_CLIENTS 32
#define PORT 5055

ClientsHandler clients;

int main() {
    if (enet_initialize() != 0) {
        std::cerr << "An error occurred while initializing ENet." << std::endl;
        exit(EXIT_FAILURE);
    }
    atexit(enet_deinitialize);

    std::cout << "Initialized ENet successfully.\n";

    ENetHost* host;
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = PORT;

        host = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0);

        if (!host) {
            std::cerr << "An error occurred while trying to create an ENet client host." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    {
        char ip[16];
        enet_address_get_host_ip(&host->address, ip, sizeof(ip));
        std::cout << "Address: " << ip << ":" << host->address.port << "\n";
    }

    bool running = true;
    while (running) {
        ENetEvent event;
        /* Wait up to 1000 milliseconds for an event. (WARNING: blocking) */
        while (enet_host_service(host, &event, 1000) > 0) {
            Client* pClient = nullptr;
            char ip[16];
            char* dataPool = nullptr;
            size_t offset;

            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    enet_address_get_host_ip(&host->address, ip, sizeof(ip));

                    {
                        Player clientPlayer;
                        clientPlayer.rect = {0,0, 20,20};
                        clientPlayer.color = {100, 75, 31, 255};

                        pClient = clients.getById(clients.add(event.peer->address, clientPlayer));
                    }

                    std::cout << "NEW CLIENT => ID: " << (enet_uint16)pClient->id << " | IP: " << ip << ":" << event.peer->address.port << "\n";
                    
                    dataPool = new char[sizeOfEnet_uint8*4 + sizeOfFloat*4];

                    memcpy(dataPool, &pClient->id, sizeOfEnet_uint8);
                    offset = sizeOfEnet_uint8;
                    memcpy(dataPool+offset, &pClient->player.color.r, sizeOfEnet_uint8);
                    offset += sizeOfEnet_uint8;
                    memcpy(dataPool+offset, &pClient->player.color.g, sizeOfEnet_uint8);
                    offset += sizeOfEnet_uint8;
                    memcpy(dataPool+offset, &pClient->player.color.b, sizeOfEnet_uint8);
                    offset += sizeOfEnet_uint8;
                    memcpy(dataPool+offset, &pClient->player.rect.x, sizeOfFloat);
                    offset += sizeOfFloat;
                    memcpy(dataPool+offset, &pClient->player.rect.y, sizeOfFloat);
                    offset += sizeOfFloat;
                    memcpy(dataPool+offset, &pClient->player.rect.width, sizeOfFloat);
                    offset += sizeOfFloat;
                    memcpy(dataPool+offset, &pClient->player.rect.height, sizeOfFloat);
                    offset += sizeOfFloat;

                    broadcastPacket(host, CLIENT_CONNECT, dataPool, offset, true, 0);
                    
                    delete[] dataPool;
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    pClient = clients.getByAddress(event.peer->address);
                    
                    if (!pClient)
                        break;

                    enet_address_get_host_ip(&host->address, ip, sizeof(ip));

                    std::cout << "CLIENT DISCONNECTED => ID: " << (enet_uint16)pClient->id << " | IP: " << ip << "\n";
                    event.peer->data = NULL;
                    running = false;
                    break;

                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }
    }

    enet_host_destroy(host);
    enet_deinitialize();
}