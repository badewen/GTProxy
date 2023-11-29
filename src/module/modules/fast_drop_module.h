#pragma once

#include "module_base.h"

#include <proton/shared/util/Variant.h>
#include <enet/enet.h>

namespace modules {

class FastDropModule : public module::ModuleBase {
public:
    FastDropModule(client::Client* client) :
        module::ModuleBase("FastDrop_Module")
        , m_amount(0)
    {}

    void on_enable() override;
    void on_disable() override;

    void set_drop_amount(uint32_t amount) {
        m_amount = amount;
    }

private:
    void drop_dialog_blocker(VariantList* varlist, int32_t net_id, bool* forward_packet);
    void drop_packet_handler(ENetPacket* outg_packet, bool* forward_packet);

private:
    uint32_t m_amount;
};
}
