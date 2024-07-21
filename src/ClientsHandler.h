#pragma once

#include <vector>
#include <chrono>

#include <enet/enet.h>

#include "Player.h"

struct Client {
    ENetPeer* peer;
    Player player;
    enet_uint16 lastPredictionId;
    std::chrono::system_clock::time_point timeSinceLastInput;
};

class ClientsHandler {
public:
    ClientsHandler() {}
    ~ClientsHandler();

    void add(ENetPeer* peer, const Player& player);
    void erase(const enet_uint8 id);

    enet_uint8 size() const;
    
    std::vector<Client*>& get();
    Client* getById(const enet_uint8& id);

private:
    std::vector<Client*> clients;
};