#pragma once

#include <vector>

#include <enet/enet.h>

#include "Player.h"

struct Client {
    ENetPeer* peer;
    Player player;
};

class ClientsHandler {
public:
    ClientsHandler() {}
    ~ClientsHandler();

    void add(ENetPeer* peer, const Player& player);
    void erase(const enet_uint16 id);

    enet_uint8 size() const;
    
    std::vector<Client*>& get();
    Client* getById(const enet_uint16& id);

private:
    std::vector<Client*> clients;
};