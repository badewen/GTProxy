#pragma once

#include "../module_base.h"

#include <optional>
#include <memory>

#include "proton/Variant.h"
#include "enet/include/enet/enet.h"

#include "../../../packet/packet.h"
#include "../../../utils/text_parse.h"

namespace peer {
class Peer;
}

namespace modules {

class AutoFishModule : public module::ModuleBase {
public:
    explicit AutoFishModule(server::Server* proxy_server)
        : module::ModuleBase("AutoFish_Module", {"NetAvatarHandler_Module"})
    {}

    void on_enable() override;
    void on_disable() override;

private:
    void on_outgoing_tile_hook(
            const packet::GameUpdatePacket* tank_packet,
            std::shared_ptr<peer::Peer> gt_peer,
            bool* forward_packet
    );
    void on_gone_fishing_hook(
            const packet::GameUpdatePacket* tank_packet,
            std::shared_ptr<peer::Peer> gt_server_peer,
            bool* forward_packet
    );
    void on_splash_audio_hook(
            VariantList* varlist,
            int32_t net_id,
            std::shared_ptr<peer::Peer> gt_server_peer,
            bool* forward_packet
    );

private:
    std::optional<packet::GameUpdatePacket> m_fishing_packet;
    std::optional<packet::GameUpdatePacket> m_tile_packet;
    std::optional<packet::GameUpdatePacket> m_fish_out_packet;
};
}
