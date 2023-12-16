#pragma once

#include "module_base.h"

#include <optional>

#include "proton/Variant.h"
#include <enet/enet.h>

#include "../../network/packet.h"
#include "../../utils/text_parse.h"

namespace modules {

class AutoFishModule : public module::ModuleBase {
public:
    AutoFishModule(client::Client* client)
        : module::ModuleBase("AutoFish_Module")
    {}

    void on_enable() override;
    void on_disable() override;

private:
    void on_outgoing_tile_hook(const packet::GameUpdatePacket* tank_packet, bool* forward_packet);
    void on_gone_fishing_hook(const packet::GameUpdatePacket* tank_packet, bool* forward_packet);
    void on_splash_audio_hook(VariantList* varlist, int32_t net_id, bool* forward_packet);

private:
    std::optional<packet::GameUpdatePacket> m_fishing_packet;
    std::optional<packet::GameUpdatePacket> m_tile_packet;
    std::optional<packet::GameUpdatePacket> m_fish_out_packet;
};
}
