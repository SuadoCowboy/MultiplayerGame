#include <iostream>
#include <string>
#include <vector>

#include <enet/enet.h>

#undef DrawText
#undef DrawTextEx

namespace rl {
    #include <raylib.h>
}

#define CLIENT
#include "Player.h"
#include "Network.h"
#include "Shared.h"
#include "Prediction.h"

#define PORT 5055

ENetPeer* serverPeer = nullptr;
double tickRate = 0.0;

bool receivePacket(ENetHost* host, ENetEvent& event, PacketUnwrapper& output) {
    int result = enet_host_service(host, &event, 0);
    if (event.type != ENET_EVENT_TYPE_RECEIVE || result <= 0)
        return false;
    
    if (event.packet->dataLength < sizeof(enet_uint8)) {
        std::cout << "ERROR WHILE RECEIVING PACKET => DATA LENGTH: " << event.packet->dataLength << std::endl;
        enet_peer_reset(serverPeer);
        enet_packet_destroy(event.packet);
        exit(EXIT_FAILURE);
    }
    
    output = PacketUnwrapper(event.packet->data);

    return true;
}

struct Client {
    enet_uint8 id;
    Player player;
};

Client* pClient = nullptr;
std::vector<Client*> clients;

rl::Vector2 serverPosition = {0.0f, 0.0f}; // Our player position in the server

void eraseClient(const enet_uint8 id) {
    enet_uint8 clientsSize = clients.size();
    for (enet_uint8 i = 0; i < clientsSize; ++i)
        if (clients[i]->id == id) {
            delete clients[i];
            clients.erase(clients.begin()+i);
            break;
        }
}

enet_uint8 addClient(const enet_uint8& id, Player& player) {
    player.setIdText(std::to_string(id).c_str(), 10);
    
    Client* pClient = new Client();
    pClient->id = id;
    pClient->player = player;

    clients.push_back(pClient);
    
    return pClient->id;
}

/// @warning clientsMutex is not locked since it doesn't know for how long it needs to be locked
Client* getClientById(const enet_uint8& id) {
    for (auto& client : clients)
        if (client->id == id) return client;
    
    return nullptr;
}

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

enet_uint8 handleClientConnect(PacketUnwrapper& packetUnwrapper) {
    enet_uint8 id;
    packetUnwrapper >> id;

    Player player;
    packetUnwrapper >> player.color.r
                    >> player.color.g
                    >> player.color.b;
    
    player.color.a = 255;

    packetUnwrapper >> player.rect;

    std::cout << "CONNECTED => ID: " << id << "\n";
    addClient(id, player);
    return id;
}

void getInitialData(ENetHost* client, ENetEvent& event, ENetPeer* serverPeer) {
    enet_uint8 clientsListSize = 0;
    while (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
        if (event.packet->dataLength < sizeof(enet_uint8)) {
            std::cout << "ERROR WHILE RECEIVING PACKET => DATA LENGTH: " << event.packet->dataLength << std::endl;
            enet_peer_reset(serverPeer);
            enet_packet_destroy(event.packet);
            exit(EXIT_FAILURE);
        }

        PacketUnwrapper packetUnwrapper(event.packet->data);

        enet_uint8 packetType;
        packetUnwrapper >> packetType;

        if (packetType == CLIENT_CONNECT) {
            enet_uint8 id = handleClientConnect(packetUnwrapper);
            if (!pClient) {
                pClient = getClientById(id);
                serverPosition = {pClient->player.rect.x, pClient->player.rect.y};
            }
        }
            
        else if (packetType == SERVER_DATA) {
            packetUnwrapper >> tickRate;
            std::cout << "SERVER TICK RATE => " << tickRate << "\n";

            while (event.packet->dataLength > packetUnwrapper.offset) {
                handleClientConnect(packetUnwrapper);
                ++clientsListSize;
            }

            enet_packet_destroy(event.packet);
            break;
        }

        enet_packet_destroy(event.packet);
    }
    
    std::cout << "PLAYERS CONNECTED: " << clientsListSize+1 << "\n";
}

int main() {
    rl::InitWindow(1590, 200, "Multiplayer Game");

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

    {
        ENetAddress address;
        enet_address_set_host(&address, "127.0.0.1");
        address.port = PORT;

        serverPeer = enet_host_connect(client, &address, 2, 0);
        if (serverPeer == NULL) {
            std::cerr << "No available peers for initiating an ENet connection." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) <= 0 || event.type != ENET_EVENT_TYPE_CONNECT) {
        enet_peer_reset(serverPeer);
        serverPeer = nullptr;

        puts("Connection to server failed.");
        exit(EXIT_FAILURE);
    }
    
    getInitialData(client, event, serverPeer);

    while (!rl::WindowShouldClose()) {
        float dt = rl::GetFrameTime();

        rl::BeginDrawing();
        rl::ClearBackground(rl::BLACK);

        for (auto& client : clients) {
            if (client->id == pClient->id) {
                client->player.update(serverPeer, dt, tickRate);

                if (client->player.dir == 0) {
                    client->player.rect.x = serverPosition.x;
                    client->player.rect.y = serverPosition.y;
                }
                if (predictedData[nextPredictionId-1].dir != client->player.dir)
                    recordPrediction(client->player.dir, client->player.rect.x, client->player.rect.y);

                rl::DrawRectangle(serverPosition.x, serverPosition.y, client->player.rect.width, client->player.rect.height, rl::RED);
            } else
                client->player.update(nullptr, dt, tickRate);

            rl::DrawRectangle(client->player.rect.x, client->player.rect.y, client->player.rect.width, client->player.rect.height, client->player.color);
            rl::DrawText(
                client->player.idText.c_str(),
                client->player.rect.x+client->player.rect.width*0.5f-client->player.idTextWidthHalf,
                client->player.rect.y-client->player.idTextFontSize-2,
                client->player.idTextFontSize,
                rl::WHITE
            );
        }

        rl::DrawFPS(0.0f,0.0f);

        rl::EndDrawing();

        PacketUnwrapper packetUnwrapper;
        while (receivePacket(client, event, packetUnwrapper)) {
            enet_uint8 type;
            packetUnwrapper >> type;

            switch (type) {
            case CLIENT_DISCONNECT: {
                enet_uint8 id;
                packetUnwrapper >> id;

                eraseClient(id);
                std::cout << "DISCONNECTED => ID: " << id << "\n";
                
                break;
            }

            case CLIENT_CONNECT:
                handleClientConnect(packetUnwrapper);
                break;
            
            case PLAYER_INPUT: {
                enet_uint8 id;
                packetUnwrapper >> id;
                
                Client* pSomeClient = getClientById(id);
                if (!pSomeClient) {
                    std::cout << "ERROR => received input from a client that doesn't seem to exist\n";
                    enet_packet_destroy(event.packet);
                    break;
                }

                if (id == pClient->id) {
                    packetUnwrapper.offset += sizeof(enet_uint8);
                    packetUnwrapper >> serverPosition;
                } else {
                    packetUnwrapper >> pSomeClient->player.dir
                                    >> pSomeClient->player.rect.x
                                    >> pSomeClient->player.rect.y;
                }
                
                break;
            }
            }

            enet_packet_destroy(event.packet);
        }
    }

    rl::CloseWindow();
    disconnect(client, event, serverPeer);
    enet_host_destroy(client);
    enet_deinitialize();
}