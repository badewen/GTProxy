#include "white_skin_fix_module.h"

#include "../../../network/client/client.h"
#include "../../../network/server/server.h"
#include "../../../utils/hash.h"
#include "../../../player/player.h"
#include "../../../module/modules/necessary/net_avatar_handler_module.h"

using namespace module;
using namespace modules;
using namespace peer;

void WhiteSkinFixModule::on_enable() {
    m_proxy_server->get_client()->add_on_incoming_tank_packet_callback(
            "WhiteSkinFix_Module",
            &WhiteSkinFixModule::on_incoming_raw_packet_hook,
            this
    );
    m_proxy_server->get_client()->add_on_incoming_varlist_packet_callback(
            "WhiteSkinFix_Module",
            &WhiteSkinFixModule::on_varlist_hook,
            this
    );
}

void WhiteSkinFixModule::on_disable() {
    m_proxy_server->get_client()->remove_on_incoming_tank_packet_callback("WhiteSkinFix_Module");
    m_proxy_server->get_client()->remove_on_incoming_varlist_packet_callback("WhiteSkinFix_Module");
}

void WhiteSkinFixModule::on_incoming_raw_packet_hook(
        packet::GameUpdatePacket *tank_packet,
        std::shared_ptr<Peer> gt_server_peer,
        bool *fw_packet
) {
    if (tank_packet->type == packet::PACKET_SEND_MAP_DATA) {
        m_execute_once_on_world_enter = true;
        return;
    }

    std::shared_ptr<NetAvatarHandlerModule> net_avatar_module =
            m_proxy_server->get_module_manager()->get_module_by_name<NetAvatarHandlerModule>("NetAvatarHandler_Module");

    if (!m_execute_once_on_world_enter &&
        tank_packet->net_id != net_avatar_module->get_current_net_avatar()->NetID)
    {
        return;
    }

    if (tank_packet->type == packet::PACKET_SET_CHARACTER_STATE) {
        m_proxy_server->send_to_gt_client_delayed(packet::create_raw_packet(tank_packet), 300);
        *fw_packet = false;
    }

}

void WhiteSkinFixModule::on_varlist_hook(
        VariantList *varlist,
        int32_t netid,
        std::shared_ptr<Peer> gt_server_peer,
        bool *fw_packet
) {
    std::shared_ptr<NetAvatarHandlerModule> net_avatar_module =
            m_proxy_server->get_module_manager()->get_module_by_name<NetAvatarHandlerModule>("NetAvatarHandler_Module");

    if (!m_execute_once_on_world_enter && netid != net_avatar_module->get_current_net_avatar()->NetID) {
        return;
    }

    switch (utils::hash::fnv1a(varlist->Get(0).GetString())) {
        case "OnFlagMay2019"_fh: {
            m_proxy_server->send_to_gt_client_delayed(
                    packet::create_varlist_packet(
                        *varlist,
                        netid,
                        ENET_PACKET_FLAG_RELIABLE
                    ), 250
            );
            *fw_packet = false;
            break;
        }
        case "OnSetRoleSkinsAndIcons"_fh: {
            m_proxy_server->send_to_gt_client_delayed(
                    packet::create_varlist_packet(
                            *varlist,
                            netid,
                            ENET_PACKET_FLAG_RELIABLE
                    ), 320
            );
            *fw_packet = false;
            break;
        }
        case "OnSetClothing"_fh: {
            m_proxy_server->send_to_gt_client_delayed(
                    packet::create_varlist_packet(
                            *varlist,
                            netid,
                            ENET_PACKET_FLAG_RELIABLE
                    ), 321
            );
            *fw_packet = false;
            m_execute_once_on_world_enter = false;
            break;
        }
    }
}