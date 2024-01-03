#pragma once

#include "../module_base.h"

#include <memory>

#include "proton/Variant.h"
#include "enet/include/enet/enet.h"

namespace peer {
class Peer;
}

namespace modules {

class FastDropModule : public module::ModuleBase {
public:
    FastDropModule(server::Server* proxy_server) :
        module::ModuleBase("FastDrop_Module")
        , m_amount(0)
    {}

    void on_enable() override;
    void on_disable() override;

    void set_drop_amount(uint32_t amount) {
        m_amount = amount;
    }

    void set_drop_all(bool drop_all) {
        m_drop_all = drop_all;
    }

private:
    void drop_dialog_blocker(
            VariantList* varlist,
            int32_t net_id,
            std::shared_ptr<peer::Peer> gt_server_peer,
            bool* forward_packet
    );

private:
    uint32_t m_amount;
    bool m_drop_all;
};
}
