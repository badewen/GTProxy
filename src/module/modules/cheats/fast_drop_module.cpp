#include "fast_drop_module.h"

#include <chrono>

#include "../../../network/server/server.h"
#include "../../../dialog/dialog_response_builder.h"

using namespace modules;

void FastDropModule::drop_dialog_blocker(
        VariantList* varlist,
        int32_t net_id,
        std::shared_ptr<peer::Peer> gt_server_peer,
        bool* forward_packet
) {
    if (varlist->Get(0).GetString() == "OnDialogRequest") {
        auto resp = dialog::DialogResponseBuilder(varlist->Get(1).GetString());
        if (!m_drop_all) {
            resp.input_text("count", std::to_string(m_amount));
        }

        m_proxy_server->get_client()->send_to_gt_server_delayed(
            packet::create_packet(
                packet::ePacketType::NET_MESSAGE_GENERIC_TEXT,
                resp.build()
            ),
            200
        );

        *forward_packet = false;
    }
}

void FastDropModule::on_enable() {
   m_proxy_server->get_client()->add_on_incoming_varlist_packet_callback(
           "FastDrop_Module",
           &FastDropModule::drop_dialog_blocker,
           this
   );
}

void FastDropModule::on_disable() {
    m_proxy_server->get_client()->remove_on_incoming_varlist_packet_callback("FastDrop_Module");
}
