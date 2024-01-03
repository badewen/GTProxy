#pragma once

#include "../module_base.h"

#include <optional>

#include "proton/Variant.h"
#include "enet/include/enet/enet.h"

#include "../../../packet/packet.h"
#include "../../../utils/text_parse.h"
#include "../../../peer/peer.h"

namespace modules {

class WhiteSkinFixModule : public module::ModuleBase {
public:
    WhiteSkinFixModule(server::Server* proxy_server)
        : module::ModuleBase("WhiteSkinFix_Module", {"NetAvatarHandler_Module"})
    {}

    void on_enable() override;
    void on_disable() override;

private:
    void on_incoming_raw_packet_hook(
            packet::GameUpdatePacket* tank_packet,
            std::shared_ptr<peer::Peer> gt_server_peer,
            bool* fw_packet
    );
    void on_varlist_hook(
            VariantList* varlist,
            int32_t netid,
            std::shared_ptr<peer::Peer> gt_server_peer,
            bool* fw_packet
    );

private:
    bool m_execute_once_on_world_enter;
};
}