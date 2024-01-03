#pragma once

#include "../module_base.h"

#include <optional>

#include "proton/Variant.h"
#include "enet/include/enet/enet.h"

#include "../../../packet/packet.h"
#include "../../../utils/text_parse.h"
#include "../../../world/world_info.h"
#include "../../../peer/peer.h"

namespace modules {

class WorldHandlerModule : public module::ModuleBase {
public:
    explicit WorldHandlerModule(server::Server* proxy_server)
        : module::ModuleBase("WorldHandler_Module")
    {}

    void on_enable() override;
    void on_disable() override;



private:
    void on_incoming_raw_packet_hook(packet::GameUpdatePacket* tank_packet, std::shared_ptr<peer::Peer>, bool* fw_packet);

    void on_receive_world_data(std::vector<uint8_t> data);

private:
    world::WorldInfo m_current_world;
};
}