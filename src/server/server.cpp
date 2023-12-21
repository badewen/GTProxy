#include "server.h"

#include <utility>

#include <enet/enet.h>

using namespace server;

bool Server::start(Config conf) {
    m_config = std::move(conf);


    m_thread_pool->push_task(&Server::server_thread, this);
    m_thread_pool->push_task(&client::Client::start, m_client);
}

void Server::on_connect(ENetPeer *peer) {
    m_gt_peer = std::make_shared<peer::Peer>(peer);

    m_on_connect_callbacks.Invoke(m_gt_peer);
}

void Server::on_receive(ENetPeer *peer, ENetPacket *packet) {
    packet::ePacketType packet_type = packet::get_packet_type(packet);

    bool forward_packet = true;

    switch (packet_type) {
        case packet::ePacketType::NET_MESSAGE_GAME_PACKET: {
            m_on_outgoing_tank_packet.Invoke(packet::get_tank_packet(packet), m_gt_peer, &forward_packet);
        }
        default: {
            m_on_outgoing_packet.Invoke(packet, m_gt_peer, &forward_packet);
        }
    }

    if (forward_packet) {
        m_client->get_server_peer()->send_enet_packet(packet);
    }
}

void Server::on_disconnect(ENetPeer *peer) {
    m_on_disconnect_callbacks.Invoke(m_gt_peer);

    m_gt_peer.reset();
}

void Server::server_thread() {
    while (m_running) {
        ENetEvent event{};
        while (enet_host_service(m_enet_host, &event, 1)) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    on_connect(event.peer);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    on_disconnect(event.peer);
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    on_receive(event.peer, event.packet);
                    break;
                }
            }
            enet_packet_destroy(event.packet);
        }
    }
}

void Server::create_host(bool use_new_packet) {
    ENetAddress addr {};
    addr.host = ENET_HOST_ANY;
    addr.port = m_config.Host.port;

    m_enet_host = enet_host_create(&addr, 1, 1, 0, 0);

    enet_host_compress_with_range_coder(m_enet_host);

    m_enet_host->checksum = enet_crc32;
    m_enet_host->usingNewPacketForServer = use_new_packet;
}