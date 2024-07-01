#include <iostream>

#include <enet/enet.h>

namespace rl {
    #include <raylib.h>
}

#include "Player.h"
#include "Packet.h"
#include "ClientsHandler.h"
#include "Shared.h"

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
            Packet packet;
            char ip[16];

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

                    packet << (enet_uint8)CLIENT_CONNECT
                           << pClient->id
                           << pClient->player.color.r
                           << pClient->player.color.g
                           << pClient->player.color.b
                           << pClient->player.rect;
                    broadcastPacket(host, packet, true, 0);
                    packet.deleteData();

                    packet << (enet_uint8)CLIENTS_LIST
                           << clients.size()-1;
                    sendPacket(event.peer, packet, true, 0);
                    packet.deleteData();

                    for (auto& client : clients.get()) {
                        if (client.id == pClient->id)
                            continue;

                        packet << (enet_uint8)CLIENTS_LIST
                               << client.id
                               << client.player.color.r
                               << client.player.color.g
                               << client.player.color.b
                               << client.player.rect;
                        
                        sendPacket(event.peer, packet, true, 0);
                        packet.deleteData();
                    }
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    pClient = clients.getByAddress(event.peer->address);
                    if (!pClient) {
                        std::cout << "UNKNOWN CLIENT TRIED TO DISCONNECT\n";
                        break;
                    }

                    enet_address_get_host_ip(&pClient->address, ip, sizeof(ip));

                    std::cout << "CLIENT DISCONNECTED => ID: " << (enet_uint16)pClient->id << " | IP: " << ip << ":" << pClient->address.port << "\n";
                    packet << (enet_uint8)CLIENT_DISCONNECT
                           << pClient->id;

                    broadcastPacket(host, packet, true, 0);
                    packet.deleteData();

                    clients.erase(pClient->id);
                    event.peer->data = NULL;
                    break;

                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }
    }

    enet_host_destroy(host);
    enet_deinitialize();
}