#pragma once

#include "../module_base.h"

#include <optional>

#include "proton/Variant.h"
#include "enet/include/enet/enet.h"

#include "../../../network/packet.h"
#include "../../../utils/text_parse.h"

namespace modules {

class WorldHandlerModule : public module::ModuleBase {
public:
    WorldHandlerModule(client::Client* client)
        : module::ModuleBase("WorldHandler_Module")
    {}

    void on_enable() override;
    void on_disable() override;

private:
    void on_incoming_raw_packet_hook(packet::GameUpdatePacket* tank_packet, bool* fw_packet);

    void on_receive_world_data(std::vector<uint8_t> data);

private:
};
}