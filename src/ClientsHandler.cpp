#include "ClientsHandler.h"

#include <iostream>

enet_uint8 ClientsHandler::add(const ENetAddress& address, const Player& player) {
    Client client = {
        address,
        size(),
        player
    };

    clients.push_back(client);
    
    return client.id;
}

void ClientsHandler::erase(const enet_uint8 id) {
    enet_uint8 clientsSize = clients.size();
    for (enet_uint8 i = 0; i < clientsSize;) {
        if (clients[i].id > id)
            --(clients[i].id);

        else if (clients[i].id == id) {
            clients.erase(clients.begin()+i);
            --clientsSize;
            continue;
        }

        ++i;
    }
}

enet_uint8 ClientsHandler::size() const {
    return clients.size();
}

std::vector<Client>& ClientsHandler::get() {
    return clients;
}

Client* ClientsHandler::getById(const enet_uint8& id) {
    for (auto& client : clients)
        if (client.id == id)
            return &client;
    
    return nullptr;
}

Client* ClientsHandler::getByAddress(const ENetAddress& address) {
    for (auto& client : clients)
        if (client.address.host == address.host && client.address.port == address.port)
            return &client;
    
    return nullptr;
}