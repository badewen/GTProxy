#pragma once

#include "module_base.h"

#include <optional>

#include <proton/shared/util/Variant.h>
#include <enet/enet.h>

#include "../../network/packet.h"
#include "../../utils/text_parse.h"

namespace modules {

class WhiteSkinFixModule : public module::ModuleBase {
public:
    WhiteSkinFixModule(client::Client* client)
        : module::ModuleBase("WhiteSkinFix_Module")
    {}

    void on_enable() override;
    void on_disable() override;

private:
    void on_incoming_raw_packet_hook(packet::GameUpdatePacket* tank_packet, bool* fw_packet);
    void on_varlist_hook(VariantList* varlist, int32_t netid, bool* fw_packet);

private:
    bool m_execute_once_on_world_enter;
};
}