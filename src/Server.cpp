#include <iostream>

#include <enet/enet.h>

namespace rl {
    #include <raylib.h>
}

#include "Player.h"
#include "Network.h"
#include "ClientsHandler.h"
#include "TimeSystem.h"
#include "Shared.h"

#define MAX_CLIENTS 32
#define PORT 5055

ClientsHandler clients;

double tickInterval = 0.0;
enet_uint8 currentTick = 0;
enet_uint8 maxTick = 0; // = 1/tickInterval

ENetHost* host = nullptr;

void update() {
    timerSleep(tickInterval);

    for (auto& client : clients.get()) {
        client->player.update();

        if (client->player.oldDir == client->player.dir)
            continue;
        
        client->player.oldDir = client->player.dir;

        Packet packet;
        packet << (enet_uint8)PLAYER_INPUT
                << client->id
                << client->player.dir
                << client->player.rect.x
                << client->player.rect.y;

        broadcastPacket(host, packet, false, 1);
        packet.deleteData();
    }

    currentTick = (currentTick + 1) % maxTick;
}

int main() {
    if (enet_initialize() != 0) {
        std::cerr << "An error occurred while initializing ENet." << std::endl;
        exit(EXIT_FAILURE);
    }
    atexit(enet_deinitialize);

    std::cout << "Initialized ENet successfully.\n";

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

    // TODO: BIG ISSUE: CPU is from 20%-30% usage WITHOUT CLIENTS CONNECTED and when a client connects
    // it gets lower, probably because of all the mutexes(which is another problem because client update
    // should not be delayed or else it will get the wrong position)

    // 1 (second) / 66.66... (tickRate) = 15ms = 0.015s
    tickInterval = 0.015;
    maxTick = 1 / tickInterval - 1;

    while (true) {
        ENetEvent event;
        
        while (enet_host_service(host, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    char ip[16];
                    enet_address_get_host_ip(&host->address, ip, sizeof(ip));

                    Client* pClient;
                    {
                        Player clientPlayer;
                        clientPlayer.rect = {100, 100, 20,20};
                        clientPlayer.color = {100, 31, 75, 255};
                        
                        pClient = clients.getById(clients.add(event.peer->address, clientPlayer));
                    }

                    std::cout << "CLIENT CONNECTED => ID: " << (enet_uint16)pClient->id << " | IP: " << ip << ":" << event.peer->address.port << "\n";

                    Packet packet;
                    packet << (enet_uint8)CLIENT_CONNECT
                           << pClient->id
                           << pClient->player.color.r
                           << pClient->player.color.g
                           << pClient->player.color.b
                           << pClient->player.rect;
                    
                    broadcastPacket(host, packet, true, 0);
                    packet.deleteData();
                    
                    packet << (enet_uint8)SERVER_DATA
                           << (enet_uint8)(tickInterval*1000)
                           << clients.size()-1;

                    sendPacket(event.peer, packet, true, 0);
                    packet.deleteData();
                    
                    for (auto& client : clients.get()) {
                        if (client->id == pClient->id)
                            continue;

                        packet << (enet_uint8)SERVER_DATA
                               << client->id
                               << client->player.color.r
                               << client->player.color.g
                               << client->player.color.b
                               << client->player.rect;
                        
                        sendPacket(event.peer, packet, true, 0);
                        packet.deleteData();
                    }

                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    PacketUnwrapper packetUnwrapper{event.packet->data};
                    
                    enet_uint8 packetType;
                    packetUnwrapper >> packetType;

                    if (packetType == PLAYER_INPUT) {
                        enet_uint8 playerDir;
                        packetUnwrapper >> playerDir;

                        // 15 = 1 | 2 | 4 | 8 = all keys pressed. Above that it's unknown data.
                        if (playerDir > 15) {
                            enet_packet_destroy(event.packet);
                            break;
                        }
                        
                        Client* pClient = clients.getByAddress(event.peer->address);
                        if (!pClient) {
                            enet_packet_destroy(event.packet);
                            break;
                        }

                        pClient->player.dir = playerDir;
                    }

                    enet_packet_destroy(event.packet);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    Client* pClient = clients.getByAddress(event.peer->address);
                    
                    if (!pClient) {
                        std::cout << "UNKNOWN CLIENT TRIED TO DISCONNECT\n";
                        break;
                    }

                    char ip[16];
                    enet_address_get_host_ip(&pClient->address, ip, sizeof(ip));

                    std::cout << "CLIENT DISCONNECTED => ID: " << (enet_uint16)pClient->id << " | IP: " << ip << ":" << pClient->address.port << "\n";
                    
                    Packet packet;
                    packet << (enet_uint8)CLIENT_DISCONNECT
                           << pClient->id;

                    broadcastPacket(host, packet, true, 0);

                    packet.deleteData();

                    clients.erase(pClient->id);

                    event.peer->data = NULL;
                    break;
                }

                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }

        update();
    }

    enet_host_destroy(host);
    enet_deinitialize();
}