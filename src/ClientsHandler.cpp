#include "ClientsHandler.h"

#include <iostream>
#include <mutex>
#include <chrono>

#include "TickHandler.h"

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
    enet_uint8 clientsSize = clients.size();
    for (enet_uint8 i = 0; i < clientsSize;) {
        if (clients[i]->id > id)
            --(clients[i]->id);

        else if (clients[i]->id == id) {
            delete clients[i];
            clients.erase(clients.begin()+i);
            --clientsSize;
            continue;
        }

        ++i;
    }
}

enet_uint16 ClientsHandler::size() const {
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

void ClientsHandler::updateClients(const float tickInterval) {
    TickHandler tickHandler(tickInterval);
    auto dtStartTime = std::chrono::steady_clock::now();
    
    while (true) {
        tickHandler.update(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - dtStartTime).count());
        
        dtStartTime = std::chrono::steady_clock::now();

        if (!tickHandler.shouldTick())
            continue;
        
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto& client : clients)
            client->player.update();

        if (!keepUpdatingClients)
            break;
    }
}

void ClientsHandler::lock() {
    clientsMutex.lock();
}

void ClientsHandler::unlock() {
    clientsMutex.unlock();
}

void ClientsHandler::stopUpdateClients() {
    std::lock_guard<std::mutex> lock(clientsMutex);
    keepUpdatingClients = false;
}