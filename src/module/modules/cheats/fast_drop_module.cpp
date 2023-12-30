#include "fast_drop_module.h"

#include <chrono>

#include "../../../client/client.h"
#include "../../../dialog/dialog_response_builder.h"

using namespace modules;

void FastDropModule::drop_dialog_blocker(VariantList* varlist, int32_t net_id, bool* forward_packet) {
    if (varlist->Get(0).GetString() == "OnDialogRequest") {
        auto resp = dialog::DialogResponseBuilder(varlist->Get(1).GetString());
        if (!m_drop_all) {
            resp.input_text("count", std::to_string(m_amount));
        }

        m_client->send_to_server_delayed(player::Peer::build_packet(
                packet::ePacketType::NET_MESSAGE_GENERIC_TEXT,
                resp.build()
        ), 200);

        *forward_packet = false;
    }
}

void FastDropModule::on_enable() {
    this->m_proxy_server->get_ctx()->OnIncomingVarlist.Register("FastDrop_Module", &FastDropModule::drop_dialog_blocker, this);
}

void FastDropModule::on_disable() {
    this->m_proxy_server->get_ctx()->OnIncomingVarlist.Remove("FastDrop_Module");
}
