#include "fast_vend_module.h"

#include "../../client/client.h"

using namespace modules;

void FastVendModule::on_enable() {
    m_client->get_ctx()->OnIncomingVarlist.Register("FastVend_Module", &FastVendModule::on_dialog_hook, this);
}

void FastVendModule::on_disable() {
    m_client->get_ctx()->OnIncomingVarlist.Remove("FastVend_Module");
}

void FastVendModule::on_dialog_hook(VariantList* var_list, int32_t net_id, bool* forward_packet) {
    if (var_list->Get(0).GetString() != "OnDialogRequest") return;

    dialog::DialogResponseBuilder dialog_response {var_list->Get(1).GetString()};

    if (dialog_response.get_dialog_name() != "vending") return;

    // fast buy section
    // handle the initial buy dialog where it asks for the amount of item
    if (dialog_response.has_text_input("buycount") && m_enable_fast_buy) {
        on_vending_buy_dialog(dialog_response);
    }
    // confirms the buy dialog
    else if (dialog_response.has_data_embed("verify")) {
        on_vending_confirm_buy(dialog_response);
    }

    // fast stock
    else if (dialog_response.has_button("addstock") && m_enable_fast_stock) {
        on_vending_add_stock(dialog_response);
    }

    // fast empty vend
    else if (dialog_response.has_button("pullstock") && m_enable_fast_empty) {
        on_vending_empty_stock(dialog_response);
    }

    // fast withdraw wl
    else if (dialog_response.has_button("withdraw") && m_enable_fast_withdraw) {
        on_vending_withdraw_wl(dialog_response);
    }

    else {
        return;
    }

    *forward_packet = false;

    m_client->send_to_server_delayed(
            player::Peer::build_packet(
                packet::NET_MESSAGE_GENERIC_TEXT,
                dialog_response.build()
            ),
            50
    );
}

void FastVendModule::on_vending_buy_dialog(dialog::DialogResponseBuilder& dialog_response) const {
    dialog_response.input_text("buycount", std::to_string(m_buy_amount));
}

void FastVendModule::on_vending_confirm_buy(const dialog::DialogResponseBuilder& dialog_response) const {
    // do nothing. it only need embed data and dialog return.
}

void FastVendModule::on_vending_add_stock(dialog::DialogResponseBuilder& dialog_response) const {
    dialog_response.click_button("addstock");
}

void FastVendModule::on_vending_empty_stock(dialog::DialogResponseBuilder& dialog_response) const {
    dialog_response.click_button("pullstock");
}

void FastVendModule::on_vending_withdraw_wl(dialog::DialogResponseBuilder& dialog_response) const {
    dialog_response.click_button("withdraw");
}

