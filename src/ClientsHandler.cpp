#include "ClientsHandler.h"

ClientsHandler::~ClientsHandler() {
    for (auto& client : clients) {
        delete client;
    }

    clients.clear();
}

void ClientsHandler::add(ENetPeer* peer, const Player& player) {
    Client* pClient = new Client();
    
    pClient->peer = peer;
    pClient->player = player;

    clients.push_back(pClient);
}

void ClientsHandler::erase(const enet_uint16 id) {
    enet_uint8 clientsSize = size();
    for (enet_uint8 i = 0; i < clientsSize; ++i) {
        if (clients[i]->peer->incomingPeerID == id) {
            delete clients[i];
            clients.erase(clients.begin()+i);
        }
    }
}

enet_uint8 ClientsHandler::size() const {
    return clients.size();
}

std::vector<Client*>& ClientsHandler::get() {
    return clients;
}

Client* ClientsHandler::getById(const enet_uint16& id) {
    for (auto& client : clients)
        if (client->peer->incomingPeerID == id)
            return client;
    
    return nullptr;
}