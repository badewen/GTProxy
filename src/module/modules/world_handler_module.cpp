#include "world_handler_module.h"

#include "../../client/client.h"

using namespace modules;

void WorldHandlerModule::on_enable() {
    m_client->get_ctx()->OnIncomingTankPacket.Register(
            "WorldHandler_Module",
            &WorldHandlerModule::on_incoming_raw_packet_hook,
            this
    );
}

void WorldHandlerModule::on_disable() {
    m_client->get_ctx()->OnIncomingTankPacket.Remove("WorldHandler_Module");
}

void WorldHandlerModule::on_incoming_raw_packet_hook(packet::GameUpdatePacket *tank_packet, bool *fw_packet) {
    switch (tank_packet->type) {
        case packet::eTankPacketType::PACKET_SEND_MAP_DATA: {
            uint8_t* ext_data = get_extended_data(tank_packet);
            on_receive_world_data({ext_data, ext_data + tank_packet->data_size + 1});
            break;
        }
        default: {
            break;
        }
    }
}

void WorldHandlerModule::on_receive_world_data(std::vector<uint8_t> data) {

    if (!m_client->get_ctx()->CurrentWorldInfo.serialize(std::move(data))) {
        m_client->log_to_client("`4WORLD PARSING FAILED. DUMPING CURRENT WORLD DATA...");
        m_client->execute_command("save", {m_client->get_ctx()->CurrentWorldInfo.Name});
    } else {
        m_client->log_to_client("`2WORLD PARSING SUCCESS");
    }
}
