#include "connection_handler_module.h"

#include "../../../network/client/client.h"
#include "../../../network/server/server.h"

#include "../../../utils/klv.h"

using namespace modules;

void ConnectionHandlerModule::on_enable() {
    m_proxy_server->add_on_connect_callback(
            "ConnectionHandler_Module",
            &ConnectionHandlerModule::on_gt_client_connect,
            this
    );
    m_proxy_server->add_on_disconnect_callback(
            "ConnectionHandler_Module",
            &ConnectionHandlerModule::on_gt_client_disconnect,
            this
    );

    m_proxy_server->get_client()->add_on_connect_callback(
            "ConnectionHandler_Module",
            &ConnectionHandlerModule::on_proxy_client_connect,
            this
    );
    m_proxy_server->get_client()->add_on_disconnect_callback(
            "ConnectionHandler_Module",
            &ConnectionHandlerModule::on_proxy_client_disconnect,
            this
    );

    m_proxy_server->get_client()->add_on_incoming_packet_callback(
            "ConnectionHandler_Module",
            &ConnectionHandlerModule::on_receive_hello_packet_hook,
            this
    );
    m_proxy_server->get_client()->add_on_incoming_varlist_packet_callback(
            "ConnectionHandler_Module",
            &ConnectionHandlerModule::on_receive_redirect_packet_hook,
            this
    );

    m_proxy_server->add_on_outgoing_packet_callback(
            "ConnectionHandler_Module",
            &ConnectionHandlerModule::on_receive_login_packet_hook,
            this
    );
}

void ConnectionHandlerModule::on_disable() {
    m_proxy_server->remove_on_connect_callback("ConnectionHandler_Module");
    m_proxy_server->remove_on_disconnect_callback("ConnectionHandler_Module");
    m_proxy_server->get_client()->remove_on_connect_callback("ConnectionHandler_Module");
    m_proxy_server->get_client()->remove_on_disconnect_callback("ConnectionHandler_Module");
    m_proxy_server->get_client()->remove_on_incoming_packet_callback("ConnectionHandler_Module");
    m_proxy_server->get_client()->remove_on_incoming_varlist_packet_callback("ConnectionHandler_Module");
    m_proxy_server->remove_on_outgoing_packet_callback("ConnectionHandler_Module");
}

void ConnectionHandlerModule::on_gt_client_connect(std::shared_ptr<peer::Peer> gt_peer) {
    spdlog::info("A new client is connected to the proxy");
    gt_peer->send_packet(packet::ePacketType::NET_MESSAGE_SERVER_HELLO, std::vector<uint8_t>{0} );
}

void ConnectionHandlerModule::on_gt_client_disconnect(std::shared_ptr<peer::Peer> gt_peer) {
    spdlog::info("The client is disconnected from the proxy.");
    m_proxy_server->get_client()->get_server_peer()->disconnect();
}

void ConnectionHandlerModule::on_proxy_client_connect(std::shared_ptr<peer::Peer> gt_server_peer) {
    spdlog::info("Connected to Growtopia server. ({}:{})",
                 m_gt_server_ip,
                 m_gt_server_port
    );
}

void ConnectionHandlerModule::on_proxy_client_disconnect(std::shared_ptr<peer::Peer> gt_server_peer) {
    spdlog::info("Disconnected from Growtopia server.");

    if (m_proxy_server->get_gt_peer()->is_connected()) {
        m_proxy_server->get_gt_peer()->disconnect();
    }

    m_login_data = {};
}

void ConnectionHandlerModule::on_receive_hello_packet_hook(
        ENetPacket *pkt,
        std::shared_ptr<peer::Peer> gt_server_peer,
        bool *fw_packet
) {
    if (packet::get_packet_type(pkt) != packet::ePacketType::NET_MESSAGE_SERVER_HELLO) {
        return;
    }

    gt_server_peer->send_packet(packet::ePacketType::NET_MESSAGE_GENERIC_TEXT, m_login_data.get_all_raw());

    *fw_packet = false;
}

void ConnectionHandlerModule::on_receive_login_packet_hook(
        ENetPacket *pkt,
        std::shared_ptr<peer::Peer> gt_peer,
        bool *fw_packet
) {
    if (packet::get_packet_type(pkt) != packet::ePacketType::NET_MESSAGE_GENERIC_TEXT) {
        return;
    }

    utils::TextParse login_parse {packet::get_text(pkt)};

    if (!login_parse.get("meta", 1).empty()) {
        m_login_data = login_parse;

        if (m_proxy_server->get_config()->Misc.spoof_login &&
            login_parse.get("meta", 1) != m_current_gt_client_meta)
        {
            utils::LoginData generated_login_data = utils::LoginData::Generate();

            m_login_data.set("mac", generated_login_data.Spoofed_mac);
            m_login_data.set("rid", generated_login_data.Spoofed_rid);
            m_login_data.set("wk", generated_login_data.Spoofed_wk);
            m_login_data.set("hash", generated_login_data.Spoofed_device_id_hash);
            m_login_data.set("hash2", generated_login_data.Spoofed_mac_hash);
            m_login_data.set(
                    "klv",
                    utils::generate_klv(
                            m_login_data.get<std::uint16_t>("protocol", 1),
                            m_login_data.get("game_version", 1),
                            m_login_data.get("rid", 1)
                    )
            );
            m_current_gt_client_meta = login_parse.get("meta", 1);
        }

        if (m_proxy_server->get_config()->Misc.force_update_game_version) {
            m_login_data.set("game_version", m_proxy_server->get_config()->Server.game_version);
        }

        if (m_proxy_server->get_config()->Misc.force_update_protocol) {
            m_login_data.set("protocol", m_proxy_server->get_config()->Server.protocol);
        }

        m_login_data = login_parse;

        auto http_data = server::Http::ServerDataCache.find(m_current_gt_client_meta);

        // first time connected to the proxy
        if (http_data != server::Http::ServerDataCache.end()) {
            m_gt_server_ip = http_data->second.get("server", 1);
            m_gt_server_port = http_data->second.get("port", 1);
            m_use_new_packet = http_data->second.get<bool>("type", 1);

            server::Http::ServerDataCache.erase(m_current_gt_client_meta);
        }

        ENetAddress addr {};
        enet_address_set_host_ip(&addr, m_gt_server_ip.c_str());
        addr.port = std::stoi(m_gt_server_port);

        m_proxy_server->get_client()->connect(addr, m_use_new_packet);

        *fw_packet = false;
    }
}

void ConnectionHandlerModule::on_receive_redirect_packet_hook(
        VariantList *varlist,
        int32_t net_id,
        std::shared_ptr<peer::Peer> gt_server_peer,
        bool *fw_packet
) {
    if (varlist->Get(0).GetString() == "OnSendToServer") {
        std::vector<std::string> tokenized_data = utils::TextParse::string_tokenize(varlist->Get(4).GetString());

        m_gt_server_ip = tokenized_data.at(0);
        m_gt_server_port = varlist->Get(1).GetString();

        varlist->Get(1).Set(m_proxy_server->get_config()->Host.port);
        varlist->Get(4).Set(
                fmt::format(
                        "{}|{}|{}",
                        "127.0.0.1",
                        tokenized_data.at(1),
                        tokenized_data.at(2)
                )
        );

        m_proxy_server->send_to_gt_client(
            packet::create_varlist_packet(
                *varlist,
                net_id
            ),
            false
        );

        *fw_packet = false;
    }
}
