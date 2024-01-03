#include "world_handler_module.h"

#include "../../../network/server/server.h"

using namespace modules;

void WorldHandlerModule::on_enable() {
    m_proxy_server->get_client()->add_on_incoming_tank_packet_callback(
            "WorldHandler_Module",
            &WorldHandlerModule::on_incoming_raw_packet_hook,
            this
    );
}

void WorldHandlerModule::on_disable() {
    m_proxy_server->get_client()->remove_on_incoming_tank_packet_callback("WorldHandler_Module");
}

void WorldHandlerModule::on_incoming_raw_packet_hook(
        packet::GameUpdatePacket *tank_packet,
        std::shared_ptr<peer::Peer> gt_server_peer,
        bool *fw_packet
) {
    switch (tank_packet->type) {
        case packet::eTankPacketType::PACKET_SEND_MAP_DATA: {
            std::vector<uint8_t> ext_data = packet::get_extended_data(tank_packet);
            on_receive_world_data(ext_data);
            break;
        }
        default: {
            break;
        }
    }
}

void WorldHandlerModule::on_receive_world_data(std::vector<uint8_t> data) {

//    if (!m_client->get_ctx()->CurrentWorldInfo.serialize(std::move(data))) {
//        m_client->log_to_client("`4WORLD PARSING FAILED. DUMPING CURRENT WORLD DATA...");
//        m_client->execute_command("save", {m_client->get_ctx()->CurrentWorldInfo.Name});
//    } else {
//        m_client->log_to_client("`2WORLD PARSING SUCCESS");
//    }
}
