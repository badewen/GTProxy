#include "auto_fish_module.h"

#include "../../client/client.h"

using namespace modules;

void AutoFishModule::on_enable() {
    m_client->get_ctx()->OnOutgoingTankPacket.Register("AutoFish_Module", &AutoFishModule::on_outgoing_tile_hook, this);
    m_client->get_ctx()->OnIncomingTankPacket.Register("AutoFish_Module", &AutoFishModule::on_gone_fishing_hook, this);
    m_client->get_ctx()->OnIncomingVarlist.Register("AutoFish_Module", &AutoFishModule::on_splash_audio_hook, this);
}

void AutoFishModule::on_disable() {
    m_client->get_ctx()->OnOutgoingTankPacket.Remove("AutoFish_Module");
    m_client->get_ctx()->OnIncomingTankPacket.Remove("AutoFish_Module");
    m_client->get_ctx()->OnIncomingVarlist.Remove("AutoFish_Module");

    m_fishing_packet.reset();
    m_tile_packet.reset();
    m_fishing_packet.reset();
}

void AutoFishModule::on_outgoing_tile_hook(const packet::GameUpdatePacket *tank_packet, bool *forward_packet) {
    if (tank_packet->type == packet::PACKET_TILE_CHANGE_REQUEST) {
        m_tile_packet = *tank_packet;
    }
}

void AutoFishModule::on_gone_fishing_hook(const packet::GameUpdatePacket* tank_packet, bool* forward_packet) {
    if (tank_packet->type == packet::PACKET_GONE_FISHIN &&
        m_tile_packet.has_value() &&
        tank_packet->net_id == m_client->get_ctx()->PlayerInfo.NetID)
    {
        m_fishing_packet = m_tile_packet;
        m_fish_out_packet = m_fishing_packet;

        // fist
        m_fish_out_packet.value().value = 18;
    }
}

void AutoFishModule::on_splash_audio_hook(VariantList* varlist, int32_t net_id, bool* forward_packet) {
    if (net_id != m_client->get_ctx()->PlayerInfo.NetID &&
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

    if (m_client->get_ctx()->PlayerInfo.NetID != net_id) {
        m_client->log_to_client("Not out splash packet");
        return;
    }

    m_client->queue_packet(player::Peer::build_raw_packet(&m_fishing_packet.value()), true);
    m_client->queue_packet_delayed(player::Peer::build_raw_packet(&m_fishing_packet.value()), true, 250);
}
