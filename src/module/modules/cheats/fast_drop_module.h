#pragma once

#include "../module_base.h"

#include "proton/Variant.h"
#include "enet/include/enet/enet.h"

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

    void set_drop_all(bool drop_all) {
        m_drop_all = drop_all;
    }

private:
    void drop_dialog_blocker(VariantList* varlist, int32_t net_id, bool* forward_packet);

private:
    uint32_t m_amount;
    bool m_drop_all;
};
}
