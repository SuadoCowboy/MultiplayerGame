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

ENetHost* host = nullptr;

TickHandler tickHandler;

void update() {
    enet_uint16 timesToTick = tickHandler.shouldTick();
    for (auto& client : clients.get()) {
        client->player.update(timesToTick);

        if (client->player.dir == 0) {
            if (client->player.movementStopped)
                continue;
            else
                client->player.movementStopped = true;
        } else
            client->player.movementStopped = false;
            

        Packet packet;
        packet << (enet_uint8)PLAYER_INPUT
                << (enet_uint8)client->peer->incomingPeerID
                << client->player.dir
                << client->player.rect.x
                << client->player.rect.y;

        broadcastPacket(host, packet, false, 1);
        packet.deleteData();
    }
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

    // 1 (second) / 66.66... (tickRate) = 0.015s = 15ms
    tickHandler.tickInterval = 16;
    tickHandler.start();

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
                        
                        clients.add(event.peer, clientPlayer);
                        pClient = clients.getById(event.peer->incomingPeerID);
                    }

                    std::cout << "CLIENT CONNECTED => ID: " << pClient->peer->incomingPeerID << " | IP: " << ip << ":" << event.peer->address.port << "\n";

                    Packet packet;
                    packet << (enet_uint8)CLIENT_CONNECT
                           << (enet_uint8)pClient->peer->incomingPeerID
                           << pClient->player.color.r
                           << pClient->player.color.g
                           << pClient->player.color.b
                           << pClient->player.rect;
                    
                    broadcastPacket(host, packet, true, 0);
                    packet.deleteData();
                    
                    packet << (enet_uint8)SERVER_DATA
                           << 1000/tickHandler.tickInterval;
                    
                    for (auto& client : clients.get()) {
                        if (client->peer == pClient->peer)
                            continue;

                        packet << (enet_uint8)client->peer->incomingPeerID
                               << client->player.color.r
                               << client->player.color.g
                               << client->player.color.b
                               << client->player.rect;
                    }

                    sendPacket(event.peer, packet, true, 0);
                    packet.deleteData();
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    PacketUnwrapper packetUnwrapper{event.packet->data};
                    
                    enet_uint8 packetType;
                    packetUnwrapper >> packetType;

                    if (packetType == PLAYER_INPUT) {
                        Client* pClient = clients.getById(event.peer->incomingPeerID);
                        if (!pClient) {
                            enet_packet_destroy(event.packet);
                            break;
                        }

                        enet_uint8 playerDir;
                        packetUnwrapper >> playerDir;
                                        //>> pClient->timestamp;

                        // 15 = 1 | 2 | 4 | 8 = all keys pressed. Above that it's unknown data.
                        if (playerDir > 15) {
                            enet_packet_destroy(event.packet);
                            break;
                        }

                        pClient->player.dir = playerDir;
                    }

                    enet_packet_destroy(event.packet);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    Client* pClient = clients.getById(event.peer->incomingPeerID);
                    
                    if (!pClient) {
                        std::cout << "UNKNOWN CLIENT TRIED TO DISCONNECT\n";
                        break;
                    }

                    char ip[16];
                    enet_address_get_host_ip(&pClient->peer->address, ip, sizeof(ip));

                    std::cout << "CLIENT DISCONNECTED => ID: " << pClient->peer->incomingPeerID << " | IP: " << ip << ":" << pClient->peer->address.port << "\n";
                    
                    Packet packet;
                    packet << (enet_uint8)CLIENT_DISCONNECT
                           << (enet_uint8)pClient->peer->incomingPeerID;

                    broadcastPacket(host, packet, true, 0);

                    packet.deleteData();

                    clients.erase(pClient->peer->incomingPeerID);

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