#include <iostream>

#include <enet/enet.h>
#include <thread>
#include <mutex>

namespace rl {
    #include <raylib.h>
}

#include "Player.h"
#include "Packet.h"
#include "ClientsHandler.h"
#include "TickHandler.h"
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

    // 1 (second) / 66.66... (tickRate) = 15ms
    const enet_uint8 tickInterval = 15;
    std::thread clientsUpdateThread(&ClientsHandler::updateClients, &clients, (float)tickInterval);

    bool running = true;
    while (running) {
        ENetEvent event;
        
        while (enet_host_service(host, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    char ip[16];
                    enet_address_get_host_ip(&host->address, ip, sizeof(ip));

                    Client* pClient;
                    {
                        Player clientPlayer;
                        clientPlayer.rect = {0,0, 20,20};
                        clientPlayer.color = {100, 75, 31, 255};
                        
                        clients.lock();
                        pClient = clients.getById(clients.add(event.peer->address, clientPlayer));
                    }

                    std::cout << "NEW CLIENT => ID: " << (enet_uint16)pClient->id << " | IP: " << ip << ":" << event.peer->address.port << "\n";

                    Packet packet;
                    packet << (enet_uint8)CLIENT_CONNECT
                           << pClient->id
                           << pClient->player.color.r
                           << pClient->player.color.g
                           << pClient->player.color.b
                           << pClient->player.rect;
                    clients.unlock();

                    broadcastPacket(host, packet, true, 0);
                    packet.deleteData();
                    
                    packet << (enet_uint8)SERVER_DATA
                           << tickInterval;
                
                    clients.lock();
                    packet << clients.size()-1;
                    clients.unlock();

                    sendPacket(event.peer, packet, true, 0);
                    packet.deleteData();
                    
                    clients.lock();
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
                    clients.unlock();

                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    PacketUnwrapper packetUnwrapper{event.packet->data};

                    PacketType packetType = NONE;
                    {
                        enet_uint8 packetTypeTemp;
                        packetUnwrapper >> packetTypeTemp;
                        packetType = (PacketType)packetTypeTemp;
                    }

                    if (packetType == PLAYER_INPUT) {
                        Vector2uc playerDir;
                        packetUnwrapper >> playerDir;

                        if (playerDir.x > 2 || playerDir.y > 2) {
                            enet_packet_destroy(event.packet);
                            break;
                        }
                        
                        clients.lock();
                        Client* pClient = clients.getByAddress(event.peer->address);

                        if (!pClient) {
                            enet_packet_destroy(event.packet);
                            break;
                        }

                        Packet packet;
                        packet << (enet_uint8)PLAYER_INPUT
                               << (enet_uint16)pClient->id
                               << playerDir
                               << pClient->player.rect.x
                               << pClient->player.rect.y;
                        
                        broadcastPacket(host, packet, false, 1);
                        packet.deleteData();

                        pClient->player.dir = playerDir;
                        clients.unlock();
                    }

                    enet_packet_destroy(event.packet);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    clients.lock();
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
                    clients.unlock();

                    event.peer->data = NULL;
                    break;
                }

                case ENET_EVENT_TYPE_NONE:
                    break;
            }
        }
    }

    clients.stopUpdateClients();
    enet_host_destroy(host);
    enet_deinitialize();
}