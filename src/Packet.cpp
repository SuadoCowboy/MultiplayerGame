#include "Packet.h"

#include "Constants.h"

inline char* preparePacketData(const enet_uint8& packetType, const char* data, const size_t& size) {
	char* dataPool = new char[sizeOfEnet_uint8 + size];
	memcpy(dataPool, &packetType, sizeOfEnet_uint8);
	
    if (data && size)
		memcpy(dataPool + sizeOfEnet_uint8, data, size);

	return dataPool;
}

void sendPacket(ENetPeer* peer, const enet_uint8& packetType, const char* data, const size_t& size, const bool& reliable, const int& channel) {
	char* dataPool = preparePacketData(packetType, data, size);
	enet_uint32 flag = reliable? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;

	ENetPacket* packet = enet_packet_create(dataPool, size + sizeOfEnet_uint8, flag);
	enet_peer_send(peer, channel, packet);

    enet_packet_destroy(packet);
	delete[] dataPool;

}

void broadcastPacket(ENetHost* host, const enet_uint8& packetType, const char* data, const size_t& size, const bool& reliable, const int& channel) {
	char* dataPool = preparePacketData(packetType, data, size);
	enet_uint32 flag = reliable? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;

	ENetPacket* packet = enet_packet_create(dataPool, size + sizeOfEnet_uint8, flag);
	enet_host_broadcast(host, channel, packet);

    enet_packet_destroy(packet);
	delete[] dataPool;

}

char* parsePacket(const ENetPacket* packet, enet_uint8& packetType, size_t& dataSize) {
	void *data = packet->data;
	if (packet->dataLength < sizeOfEnet_uint8) {
		dataSize = 0;
		packetType = NONE;
		return nullptr;
	}

	memcpy(&packetType, data, sizeOfEnet_uint8);
	if (packet->dataLength > sizeOfEnet_uint8) {
		dataSize = packet->dataLength-sizeOfEnet_uint8;
		return (char*)data + sizeOfEnet_uint8;
	}

	return nullptr;
}