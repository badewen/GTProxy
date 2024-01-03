#pragma once

#include "../module_base.h"

#include <memory>

#include "proton/Variant.h"
#include "enet/include/enet/enet.h"

#include "../../../utils/text_parse.h"
#include "../../../dialog/dialog_response_builder.h"

namespace peer {
class Peer;
}

namespace modules {

/*
 * this class is to skip vending machine dialog, hence the name FastVend.
 * if all of the fast_* is enabled, then on every vending machine dialog it does the following sequence
 *
 * - respond to a buy dialog if it is a buy dialog
 * - confirm the buy dialog if it is a buy confirmation dialog
 * - add items to the vend if it is owned vending
 * - empty the vending stock if it is owned vending
 * - withdraw the vending wl if it is owned vending
*/
class FastVendModule : public module::ModuleBase {
public:
    explicit FastVendModule(server::Server* proxy_server)
        : module::ModuleBase("FastVend_Module")
        , m_buy_amount { 0 }
    {}

    void on_enable() override;
    void on_disable() override;

    void set_buy_amount(uint16_t amt) { m_buy_amount = amt; };

    void enable_fast_buy(bool enable) { m_enable_fast_buy = enable; }
    void enable_fast_stock(bool enable) { m_enable_fast_stock = enable; }
    void enable_fast_empty(bool enable) { m_enable_fast_empty = enable; }
    void enable_fast_withdraw(bool enable) { m_enable_fast_withdraw = enable; }

private:
    void on_dialog_hook(
            VariantList* var_list,
            int32_t net_id,
            std::shared_ptr<peer::Peer> gt_server_peer,
            bool* forward_packet
    );

    void on_vending_buy_dialog(dialog::DialogResponseBuilder& dialog_response) const;
    void on_vending_confirm_buy(const dialog::DialogResponseBuilder& dialog_response) const;

    void on_vending_add_stock(dialog::DialogResponseBuilder& dialog_response) const;
    void on_vending_empty_stock(dialog::DialogResponseBuilder& dialog_response) const;
    void on_vending_withdraw_wl(dialog::DialogResponseBuilder& dialog_response) const;

private:
    uint16_t m_buy_amount;

    bool m_enable_fast_buy{};
    bool m_enable_fast_stock{};
    bool m_enable_fast_empty{};
    bool m_enable_fast_withdraw{};
};
}
