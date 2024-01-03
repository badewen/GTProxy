#include "net_avatar_handler_module.h"

#include "../../../network/server/server.h"

using namespace modules;

void NetAvatarHandlerModule::on_enable() {
    m_proxy_server->get_client()->add_on_incoming_varlist_packet_callback(
            "NetAvatarHandler_Module",
            &NetAvatarHandlerModule::on_varlist_hook,
            this
    );
}

void NetAvatarHandlerModule::on_disable() {
    m_proxy_server->get_client()->remove_on_incoming_varlist_packet_callback("NetAvatarHandler_Module");
}

void NetAvatarHandlerModule::on_varlist_hook(
        VariantList *varlist,
        int32_t netid,
        std::shared_ptr<peer::Peer> gt_server_peer,
        bool *fw_packet
) {
    if (varlist->Get(0).GetString() == "OnSpawn") {
        on_spawn(varlist);
    }
    else if (varlist->Get(0).GetString() == "OnRemove") {
        on_remove(varlist);
    }
}

void NetAvatarHandlerModule::on_spawn(VariantList *varlist) {
    utils::TextParse parsed {varlist->Get(1).GetString()};
    if (parsed.get("type", 1) == "local") {
        m_current_net_avatar->NetID = parsed.get<int32_t>("netID", 1);
        m_current_net_avatar->PlayerName = parsed.get("name", 1);
        m_current_net_avatar->UserID = parsed.get<int32_t>("userID", 1);
    }
}

void NetAvatarHandlerModule::on_remove(VariantList *varlist) {
    utils::TextParse parsed {varlist->Get(1).GetString()};
    if (parsed.get<int32_t>("netID", 1) == m_current_net_avatar->NetID) {
        m_current_net_avatar = {};
    }
}