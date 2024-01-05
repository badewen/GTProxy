#include "auto_fish_module.h"

#include "../../../network/server/server.h"
#include "../necessary/net_avatar_handler_module.h"

using namespace modules;

void AutoFishModule::on_enable() {
    m_proxy_server->add_on_outgoing_tank_packet_callback("AutoFish_Module", &AutoFishModule::on_outgoing_tile_hook, this);
    m_proxy_server->get_client()->add_on_incoming_tank_packet_callback("AutoFish_Module", &AutoFishModule::on_gone_fishing_hook, this);
    m_proxy_server->get_client()->add_on_incoming_varlist_packet_callback("AutoFish_Module", &AutoFishModule::on_splash_audio_hook, this);
}

void AutoFishModule::on_disable() {
    m_proxy_server->remove_on_outgoing_tank_packet_callback("AutoFish_Module");
    m_proxy_server->get_client()->remove_on_incoming_tank_packet_callback("AutoFish_Module");
    m_proxy_server->get_client()->remove_on_incoming_varlist_packet_callback("AutoFish_Module");

    m_fishing_packet.reset();
    m_tile_packet.reset();
    m_fishing_packet.reset();
}

void AutoFishModule::on_outgoing_tile_hook(
        const packet::GameUpdatePacket *tank_packet,
        std::shared_ptr<peer::Peer> gt_peer,
        bool *forward_packet
) {
    if (tank_packet->type == packet::PACKET_TILE_CHANGE_REQUEST) {
        m_tile_packet = *tank_packet;
    }
}

void AutoFishModule::on_gone_fishing_hook(
        const packet::GameUpdatePacket* tank_packet,
        std::shared_ptr<peer::Peer> gt_server_peer,
        bool* forward_packet
) {
    std::shared_ptr<NetAvatarHandlerModule> net_avatar_module =
            m_proxy_server->get_module_manager()->get_module_by_name<NetAvatarHandlerModule>("NetAvatarHandler_Module");

    if (tank_packet->type == packet::PACKET_GONE_FISHIN &&
        m_tile_packet.has_value() &&
        tank_packet->net_id == net_avatar_module->get_current_net_avatar()->NetID)
    {
        m_fishing_packet = m_tile_packet;
        m_fish_out_packet = m_fishing_packet;

        // fist
        m_fish_out_packet.value().value = 18;
    }
}

void AutoFishModule::on_splash_audio_hook(
        VariantList* varlist,
        int32_t net_id,
        std::shared_ptr<peer::Peer> gt_server_peer,
        bool* forward_packet
) {
    std::shared_ptr<NetAvatarHandlerModule> net_avatar_module =
            m_proxy_server->get_module_manager()->get_module_by_name<NetAvatarHandlerModule>("NetAvatarHandler_Module");

    if (net_id != net_avatar_module->get_current_net_avatar()->NetID &&
        !m_fishing_packet.has_value())
    {
        return;
    }

    if (varlist->Get(0).GetString() != "OnPlayPositioned") {
        return;
    }

    if (varlist->Get(1).GetString() != "audio/splash.wav") {
        return;
    }

    if (net_avatar_module->get_current_net_avatar()->NetID != net_id) {
        return;
    }

    m_proxy_server->get_client()->send_to_gt_server(packet::create_raw_packet(&m_fishing_packet.value()), true);
    m_proxy_server->get_client()->send_to_gt_server_delayed(packet::create_raw_packet(&m_fishing_packet.value()), true, 250);
}
