#include <iostream>
#include <vector>

#include <enet/enet.h>

namespace rl {
    #include <raylib.h>
}

#include "Player.h"
#include "Packet.h"
#include "Shared.h"

#define PORT 5055

struct Client {
    enet_uint8 id;
    Player player;
};

enet_uint8 clientId;

std::vector<Client> clients;
void eraseClient(const enet_uint8 id) {
    enet_uint8 clientsSize = clients.size();
    for (enet_uint8 i = 0; i < clientsSize;) {
        if (clients[i].id > id)
            --(clients[i].id);
        
        else if (clients[i].id == id) {
            --clientsSize;
            clients.erase(clients.begin()+i);
            continue;
        }

        ++i;
    }

    if (clientId > id) --clientId;
}

enet_uint8 addClient(const enet_uint8& id, const Player& player) {
    Client client = {
        id,
        player
    };

    clients.push_back(client);
    
    return client.id;
}

Client* getClientById(const enet_uint8& id) {
    for (auto& client : clients)
        if (client.id == id) return &client;
    
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

enet_uint8 handleClientConnect(const ENetEvent& event, PacketUnwrapper& packetUnwrapper) {
    enet_uint8 id;
    packetUnwrapper >> id;

    Player player;
    packetUnwrapper >> player.color.r
                    >> player.color.g
                    >> player.color.b;
    
    player.color.a = 255;

    packetUnwrapper >> player.rect;
    
    std::cout << "DATA SIZE: " << event.packet->dataLength << "\nCONNECTED => ID: " << (enet_uint16)id << " | COLOR: "
            << (enet_uint16)player.color.r << ", "
            << (enet_uint16)player.color.g << ", "
            << (enet_uint16)player.color.b << ", "
            << (enet_uint16)player.color.a
            << " | POSITION: "
            << player.rect.x << ", "
            << player.rect.y
            << " | SIZE: "
            << player.rect.width << ", "
            << player.rect.height
            << std::endl;
    
    addClient(id, player);
    return id;
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
    } else if (event.type == ENET_EVENT_TYPE_RECEIVE)
        enet_packet_destroy(event.packet);
    
    // Get initial data
    bool handlingClientsList = false;
    bool definedOwnClient = false;
    enet_uint8 clientsListSize = 0;
    enet_uint8 clientsListIndex = 0;
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
            enet_uint8 id = handleClientConnect(event, packetUnwrapper);
            if (!definedOwnClient)
                clientId = id;
        }
            
        else if (packetType == CLIENTS_LIST) {
            if (!handlingClientsList) {
                handlingClientsList = true;

                packetUnwrapper >> clientsListSize;
                if (clientsListSize == 0)
                    break;
                
                continue;
            }

            handleClientConnect(event, packetUnwrapper);
            ++clientsListIndex;
            if (clientsListIndex >= clientsListSize)
                break;
        }

        enet_packet_destroy(event.packet);
    }

    std::cout << "PLAYERS: " << clientsListSize+1 << "\n";

    while (true) {
        if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
            if (event.packet->dataLength < sizeof(enet_uint8)) {
                std::cout << "ERROR WHILE RECEIVING PACKET => DATA LENGTH: " << event.packet->dataLength << std::endl;
                enet_peer_reset(peer);
                enet_packet_destroy(event.packet);
                exit(EXIT_FAILURE);
            }
            
            PacketUnwrapper packetUnwrapper((const char*)event.packet->data);

            enet_uint8 type;
            packetUnwrapper >> type;

            if (type == CLIENT_DISCONNECT) {
                enet_uint8 id = 0;
                packetUnwrapper >> id;

                eraseClient(id);
                std::cout << "DISCONNECTED => ID: " << (enet_uint16)id
                          << "\nCURRENT CLIENT ID: " << (enet_uint16)clientId << "\n";
            }

            if (type == CLIENT_CONNECT)
                handleClientConnect(event, packetUnwrapper);

            enet_packet_destroy(event.packet);
        }
    }

    disconnect(client, event, peer);
    enet_host_destroy(client);
    enet_deinitialize();
}