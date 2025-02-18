#include "Network.h"

Packet::~Packet() {
	deleteData();
}

void Packet::pushData(const void* _data, const size_t& size) {
	if (dataSize != 0) {
		char* newData = new char[dataSize+size];
		for (size_t i = 0; i < dataSize; ++i)
			newData[i] = *(data+i);

		delete[] data;
		data = newData;
	} else
		data = new char[size];

	memcpy(data+dataSize, _data, size);
	dataSize += size;
}

void Packet::deleteData() {
	if (dataSize == 0) return;
	
	delete[] data;
	data = nullptr;
	dataSize = 0;
}

const char* Packet::getData() {
	return data;
}

const size_t& Packet::getDataSize() {
	return dataSize;
}

void sendPacket(ENetPeer* peer, Packet& packet, const bool& reliable, const int& channel) {
	enet_uint32 flag = reliable? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;

	ENetPacket* enetPacket = enet_packet_create(packet.getData(), packet.getDataSize(), flag);
	enet_peer_send(peer, channel, enetPacket);
}

void broadcastPacket(ENetHost* host, Packet& packet, const bool& reliable, const int& channel) {
	enet_uint32 flag = reliable? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;

	ENetPacket* enetPacket = enet_packet_create(packet.getData(), packet.getDataSize(), flag);
	enet_host_broadcast(host, channel, enetPacket);
}