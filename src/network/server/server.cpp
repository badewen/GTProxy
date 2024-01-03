#include "server.h"

#include <utility>

#include "enet/include/enet/enet.h"
#include <magic_enum.hpp>

using namespace server;

bool Server::init(Config conf) {
    m_config = std::move(conf);

    return m_client->init(this, m_thread_pool);
}

bool Server::start() {
    create_host();

    m_running = true;

    m_module_manager.set_proxy_server_ptr(this);

    m_module_manager.enable_module("ConnectionHandler_Module");
    m_module_manager.enable_module("WhiteSkinFix_Module");
    m_module_manager.enable_module("WorldHandler_Module");

    m_thread_pool->push_task(&client::Client::start, m_client.get());
    m_thread_pool->push_task(&server::Server::server_thread, this);
    m_thread_pool->push_task(&server::Server::process_delayed_packet_thread, this);

    return true;
}

void Server::stop() {
    m_module_manager.disable_all_module();

    m_client->stop();
    m_gt_peer->disconnect_now();

    m_running = false;

    m_thread_pool->purge();

    m_client.reset();
    m_gt_peer.reset();

    m_on_connect_callbacks.RemoveAll();
    m_on_disconnect_callbacks.RemoveAll();
    m_on_outgoing_packet.RemoveAll();
    m_on_outgoing_tank_packet.RemoveAll();

    enet_host_destroy(m_enet_host);
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
        m_client->send_to_gt_server(packet, true);
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

void Server::create_host() {
    ENetAddress addr {};
    addr.host = ENET_HOST_ANY;
    addr.port = m_config.Host.port;

    m_enet_host = enet_host_create(&addr, 1, 1, 0, 0);

    enet_host_compress_with_range_coder(m_enet_host);

    m_enet_host->checksum = enet_crc32;
    m_enet_host->usingNewPacketForServer = true;
}

void Server::send_to_gt_client(ENetPacket *packet, bool invoke_event) {
    bool forward_packet = true;

    if (invoke_event) {
        // invoke incoming packet events.
        m_client->incoming_packet_events_invoke(packet, &forward_packet);
    }

    if (forward_packet) {
        m_client->print_packet_info_incoming(packet);
        m_gt_peer->send_enet_packet(packet);
    }
}

void Server::send_to_gt_client_delayed(ENetPacket *packet, float delay_ms, bool invoke_event) {
    m_delayed_packet_secondary_queue.enqueue({
        .Delay = delay_ms,
        .Packet = packet,
        .InvokeEvents = invoke_event
    });
}

void Server::outgoing_packet_events_invoke(ENetPacket *packet, bool *forward_packet) {
    packet::ePacketType packet_type = packet::get_packet_type(packet);

    switch (packet_type) {
        case packet::ePacketType::NET_MESSAGE_GAME_PACKET: {
            m_on_outgoing_tank_packet.Invoke(packet::get_tank_packet(packet), m_gt_peer, forward_packet);
        }
        default: {
            m_on_outgoing_packet.Invoke(packet, m_gt_peer, forward_packet);
        }
    }
}

void Server::print_packet_info_outgoing(ENetPacket *packet) {
    packet::ePacketType packet_type = packet::get_packet_type(packet);

    switch (packet_type) {
        case packet::ePacketType::NET_MESSAGE_GAME_PACKET: {
            packet::GameUpdatePacket* tank_pkt = packet::get_tank_packet(packet);

            spdlog::info("Outgoing {}[{}] TankPacket with netid {}",
                         magic_enum::enum_name(tank_pkt->type),
                         static_cast<int32_t>(tank_pkt->type),
                         tank_pkt->net_id
            );
        }
        default: {
            spdlog::info("Outgoing {}[{}] packet \n{}",
                         magic_enum::enum_name(packet_type),
                         static_cast<int32_t>(packet_type),
                         packet::get_text(packet)
            );
        }
    }
}

void Server::process_delayed_packet_thread() {
    while (m_running) {
        std::array<packetInfoStruct, 16> packets;
        while (m_delayed_packet_primary_queue.try_dequeue_bulk(packets.begin(), packets.size())) {
            for (auto pkt : packets) {
                if (!pkt.Delay.IsDone()) {
                    m_delayed_packet_secondary_queue.enqueue(pkt);
                }

                send_to_gt_client(pkt.Packet, pkt.InvokeEvents);
            }
        }
        std::swap(m_delayed_packet_primary_queue, m_delayed_packet_secondary_queue);
    }
}
