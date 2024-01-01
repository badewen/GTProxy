#include "client.h"

#include <utility>

#include "../server/server.h"
#include <enet/enet.h>
#include <magic_enum.hpp>

using namespace client;

bool Client::init(server::Server* proxy_server, std::shared_ptr<BS::thread_pool> thread_pool) {
    m_proxy_server = proxy_server;
    m_thread_pool = thread_pool;

    return true;
}

bool Client::start() {
    create_host();

    m_running = true;

    m_thread_pool->push_task(&client::Client::client_thread, this);
    m_thread_pool->push_task(&client::Client::process_delayed_packet_thread, this);

    return true;
}

bool Client::connect(ENetAddress addr, bool use_new_packet) {
    m_enet_host->usingNewPacket = use_new_packet;

    return enet_host_connect(m_enet_host, &addr, 1, NULL, m_proxy_server->get_gt_peer()->get_connect_id());
}

void Client::stop() {
    m_running = false;

    m_gt_server_peer->disconnect_now();

    m_proxy_server = nullptr;
    m_thread_pool.reset();
    m_gt_server_peer.reset();

    m_on_connect_callbacks.RemoveAll();
    m_on_disconnect_callbacks.RemoveAll();
    m_on_incoming_varlist.RemoveAll();
    m_on_incoming_packet.RemoveAll();
    m_on_incoming_tank_packet.RemoveAll();

    enet_host_destroy(m_enet_host);
}

void Client::on_connect(ENetPeer *peer) {
    m_gt_server_peer = std::make_shared<peer::Peer>(peer);

    m_on_connect_callbacks.Invoke(m_gt_server_peer);
}

void Client::on_receive(ENetPeer *peer, ENetPacket *packet) {
    packet::ePacketType packet_type = packet::get_packet_type(packet);

    bool forward_packet = true;

    switch (packet_type) {
        case packet::ePacketType::NET_MESSAGE_GAME_PACKET: {
            if (packet::get_tank_packet(packet)->type == packet::eTankPacketType::PACKET_CALL_FUNCTION) {
                std::vector<uint8_t> ext_data = packet::get_extended_data(packet::get_tank_packet(packet));
                VariantList varlist {};

                varlist.SerializeFromMem(ext_data.data(), ext_data.size());

                m_on_incoming_varlist.Invoke(&varlist, packet::get_tank_packet(packet)->net_id, m_gt_server_peer, &forward_packet);
            }

            m_on_incoming_tank_packet.Invoke(packet::get_tank_packet(packet), m_gt_server_peer, &forward_packet);
        }
        default: {
            m_on_incoming_packet.Invoke(packet, m_gt_server_peer, &forward_packet);
        }
    }

    if (forward_packet) {
        m_proxy_server->send_to_gt_client(packet, true);
    }
}

void Client::on_disconnect(ENetPeer *peer) {
    m_on_disconnect_callbacks.Invoke(m_gt_server_peer);

    m_gt_server_peer.reset();
}

void Client::client_thread() {
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
        }
    }
}

void Client::create_host() {
    m_enet_host = enet_host_create(NULL, 1, 1, 0, 0);

    enet_host_compress_with_range_coder(m_enet_host);

    m_enet_host->checksum = enet_crc32;
    m_enet_host->usingNewPacket = true;
}

void Client::send_to_gt_server(ENetPacket *packet, bool invoke_event) {

    bool forward_packet = true;

    if (invoke_event) {
        m_proxy_server->outgoing_packet_events_invoke(packet, &forward_packet);
    }

    if (forward_packet) {
        m_proxy_server->print_packet_info_outgoing(packet);
        m_gt_server_peer->send_enet_packet(packet);
    }
}

void Client::send_to_gt_server_delayed(ENetPacket *packet, float delay_ms, bool invoke_event) {
    m_delayed_packet_secondary_queue.enqueue({
        .Delay = delay_ms,
        .Packet = packet,
        .InvokeEvents = invoke_event
    });
}

void Client::incoming_packet_events_invoke(ENetPacket *packet, bool *forward_packet) {
    packet::ePacketType packet_type = packet::get_packet_type(packet);

    switch (packet_type) {
        case packet::ePacketType::NET_MESSAGE_GAME_PACKET: {
            packet::GameUpdatePacket* tank_pkt = packet::get_tank_packet(packet);

            if (tank_pkt->type == packet::eTankPacketType::PACKET_CALL_FUNCTION) {
                VariantList varlist {};
                varlist.SerializeFromMem(packet::get_extended_data(tank_pkt).data(), tank_pkt->extended_data_length);

                m_on_incoming_varlist.Invoke(&varlist, tank_pkt->net_id, m_gt_server_peer, forward_packet);
            }

            m_on_incoming_tank_packet.Invoke(packet::get_tank_packet(packet), m_gt_server_peer, forward_packet);
        }
        default: {
            m_on_incoming_packet.Invoke(packet, m_gt_server_peer, forward_packet);
        }
    }

}

void Client::print_packet_info_incoming(ENetPacket *packet) {
    packet::ePacketType packet_type = packet::get_packet_type(packet);

    switch (packet_type) {
        case packet::ePacketType::NET_MESSAGE_GAME_PACKET: {
            packet::GameUpdatePacket* tank_pkt = packet::get_tank_packet(packet);

            if (tank_pkt->type == packet::eTankPacketType::PACKET_CALL_FUNCTION) {
                VariantList varlist {};
                varlist.SerializeFromMem(packet::get_extended_data(tank_pkt).data(), tank_pkt->extended_data_length);

                spdlog::info("Incoming VariantList with netid {} \n {}",
                             tank_pkt->net_id,
                             varlist.GetContentsAsDebugString()
                             );
            }

            spdlog::info("Incoming {}[{}] TankPacket with netid {}",
                         magic_enum::enum_name(tank_pkt->type),
                         static_cast<int32_t>(tank_pkt->type),
                         tank_pkt->net_id
            );
        }
        default: {
            spdlog::info("Incoming {}[{}] packet \n{}",
                         magic_enum::enum_name(packet_type),
                         static_cast<int32_t>(packet_type),
                         packet::get_text(packet)
            );
        }
    }
}

void Client::process_delayed_packet_thread() {
    while (m_running) {
        std::array<packetInfoStruct, 16> packets;
        while (m_delayed_packet_primary_queue.try_dequeue_bulk(packets.begin(), packets.size())) {
            for (auto pkt : packets) {
                if (!pkt.Delay.IsDone()) {
                    m_delayed_packet_secondary_queue.enqueue(pkt);
                }

                send_to_gt_server(pkt.Packet, pkt.InvokeEvents);
            }
        }
        std::swap(m_delayed_packet_primary_queue, m_delayed_packet_secondary_queue);
    }
}
