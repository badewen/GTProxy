#include "fast_drop_module.h"

#include <chrono>

#include "../../client/client.h"

using namespace modules;

void FastDropModule::drop_dialog_blocker(VariantList* varlist, int32_t net_id, bool* forward_packet) {
    if (varlist->Get(0).GetString() == "OnDialogRequest" &&
        varlist->Get(1).GetString().find("drop_item") != std::string::npos) {
        *forward_packet = false;
    }
}

void FastDropModule::drop_packet_handler(ENetPacket* outg_packet, bool* forward_packet) {
    utils::TextParse text_parse { packet::get_text(outg_packet) };
    std::string item_id;

    if (packet::get_message_type(outg_packet) == packet::NET_MESSAGE_GENERIC_TEXT &&
        !text_parse.get("action", 0, "|").empty()
    ) {
        item_id = text_parse.get("itemID", 1, "|");

        m_client->send_to_server_delayed( player::Peer::build_packet(
                packet::NET_MESSAGE_GENERIC_TEXT,
                fmt::format(
                        "action|dialog_return\ndialog_name|drop_item\nitemID|{}|\ncount|{}",
                        item_id,
                        std::to_string(m_amount))
                ),
                500
        );

    }
}

void FastDropModule::on_enable() {
    this->m_client->get_ctx()->OnIncomingVarlist.Register("FastDrop_Module", &FastDropModule::drop_dialog_blocker, this);
    this->m_client->get_ctx()->OnOutgoingPacket.Register("FastDrop_Module", &FastDropModule::drop_packet_handler, this);
}

void FastDropModule::on_disable() {
    this->m_client->get_ctx()->OnIncomingVarlist.Remove("FastDrop_Module");
    this->m_client->get_ctx()->OnOutgoingPacket.Remove("FastDrop_Module");
}
