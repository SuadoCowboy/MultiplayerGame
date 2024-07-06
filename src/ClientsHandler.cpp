#include "ClientsHandler.h"

ClientsHandler::~ClientsHandler() {
    for (auto& client : clients) {
        delete client;
    }

    clients.clear();
}

enet_uint8 ClientsHandler::add(const ENetAddress& address, const Player& player) {
    Client* pClient = new Client();
    
    pClient->address = address;
    pClient->id = size();
    pClient->player = player;

    clients.push_back(pClient);
    
    return pClient->id;
}

void ClientsHandler::erase(const enet_uint8 id) {
    delete clients[id];
    clients.erase(clients.begin()+id);

    enet_uint8 clientsSize = clients.size();
    for (enet_uint8 i = id; i < clientsSize; ++i)
        --clients[i]->id;
}

enet_uint8 ClientsHandler::size() const {
    return clients.size();
}

std::vector<Client*>& ClientsHandler::get() {
    return clients;
}

Client* ClientsHandler::getById(const enet_uint8& id) {
    for (auto& client : clients)
        if (client->id == id)
            return client;
    
    return nullptr;
}

Client* ClientsHandler::getByAddress(const ENetAddress& address) {
    for (auto& client : clients)
        if (client->address.host == address.host && client->address.port == address.port)
            return client;
    
    return nullptr;
}