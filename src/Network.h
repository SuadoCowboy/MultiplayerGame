#pragma once

#include <enet/enet.h>

/// @brief only used to send data. This class allocates memory dynamically in a char* buffer called data
class Packet {
public:
    Packet() {};
    ~Packet();
    
    /// @warning no size checking is being made. Use knowing the size of data!
    void pushData(const void* data, const size_t& size);

    const size_t& getDataSize();
    const char* getData();

    /// @warning this function SHOULD be called to deallocate memory
    void deleteData();

private:
    char* data = nullptr;
    size_t dataSize = 0;
};

template<typename T>
Packet& operator<<(Packet& packet, const T& data) {
    packet.pushData(&data, sizeof(T));

    return packet;
}

struct PacketUnwrapper {
    PacketUnwrapper() {}
    PacketUnwrapper(const void* data) : data((const char*)data) {};

    template<typename T>
    void get(T& out) {
        out = *(T*)(data+offset);
        offset += sizeof(T);
    }

    size_t offset = 0;
    const char* data = nullptr;
};

template<typename T>
PacketUnwrapper& operator>>(PacketUnwrapper& packetUnwrapper, T& out) {
    packetUnwrapper.get(out);
    return packetUnwrapper;
}

void sendPacket(ENetPeer* peer, Packet& packet, const bool& reliable, const int& channel);
void broadcastPacket(ENetHost* host, Packet& packet, const bool& reliable, const int& channel);