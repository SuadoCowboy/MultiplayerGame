#pragma once

#include <vector>
#include <mutex>

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
    ~ClientsHandler();

    enet_uint8 add(const ENetAddress& address, const Player& player);
    void erase(const enet_uint8 id);
    /// @return enet_uint16 size because size could be 256
    enet_uint16 size() const;
    
    std::vector<Client*>& get();
    Client* getById(const enet_uint8& id);
    Client* getByAddress(const ENetAddress& address);

    /// @brief should be threaded
    /// @param tickInterval 1 second dividided by tickRate. Example: 1/66.666... = 0.015ms interval 
    void updateClients(const float tickInterval);

    void stopUpdateClients();

    /// @brief locks the clientsMutex
    void lock();
    /// @brief unlocks the clientsMutex
    void unlock();

private:
    bool keepUpdatingClients = true;
    std::mutex clientsMutex;
    std::vector<Client*> clients;
};