#pragma once

#include <vector>

#include <enet/enet.h>

#include "Player.h"

struct Client {
    ENetAddress address;
    enet_uint8 id;
    Player player;
};

class ClientsHandler {
public:
    ClientsHandler() {}

    inline enet_uint8 add(const ENetAddress& address, const Player& player) {
        Client client = {
            address,
            size(),
            player
        };

        clients.push_back(client);
        
        return client.id;
    }

    inline void remove(const enet_uint8& id) {
        enet_uint8 clientsSize = clients.size();
        for (enet_uint8 i = 0; i < clientsSize;) {
            if (clients[i].id > id)
                --clients[i].id;

            else if (clients[i].id == id) {
                clients.erase(clients.begin()+i);
                --clientsSize;
                continue;
            }

            ++i;
        }
    }

    inline enet_uint8 size() const {
        return clients.size();
    }

    Client* getById(const enet_uint8& id) {
        for (auto& client : clients)
            if (client.id == id)
                return &client;
        
        return nullptr;
    }

    Client* getByAddress(const ENetAddress& address) {
        for (auto& client : clients)
            if (client.address.host == address.host && client.address.port == address.port)
                return &client;
        
        return nullptr;
    }

private:
    std::vector<Client> clients;
};