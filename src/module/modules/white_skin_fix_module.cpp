#include "white_skin_fix_module.h"

#include "../../client/client.h"
#include "../../utils/hash.h"

using namespace modules;

void WhiteSkinFixModule::on_enable() {
    m_client->get_ctx()->OnIncomingTankPacket.Register(
            "WhiteSkinFix_Module",
            &WhiteSkinFixModule::on_incoming_raw_packet_hook,
            this
    );
    m_client->get_ctx()->OnIncomingVarlist.Register(
            "WhiteSkinFix_Module",
            &WhiteSkinFixModule::on_varlist_hook,
            this
    );
}

void WhiteSkinFixModule::on_disable() {
    m_client->get_ctx()->OnIncomingTankPacket.Remove("WhiteSkinFix_Module");
    m_client->get_ctx()->OnIncomingVarlist.Remove("WhiteSkinFix_Module");
}

void WhiteSkinFixModule::on_incoming_raw_packet_hook(packet::GameUpdatePacket *tank_packet, bool *fw_packet) {
    if (tank_packet->type == packet::PACKET_SEND_MAP_DATA) {
        m_execute_once_on_world_enter = true;
    }
    if (!m_execute_once_on_world_enter && tank_packet->net_id != m_client->get_ctx()->PlayerInfo.NetID) {
        return;
    }

    if (tank_packet->type == packet::PACKET_SET_CHARACTER_STATE) {
        m_client->send_to_gt_client_delayed(player::Peer::build_raw_packet(tank_packet), 350);
        *fw_packet = false;
    }

}

void WhiteSkinFixModule::on_varlist_hook(VariantList *varlist, int32_t netid, bool *fw_packet) {
    if (!m_execute_once_on_world_enter && netid != m_client->get_ctx()->PlayerInfo.NetID) {
        return;
    }

    switch (utils::hash::fnv1a(varlist->Get(0).GetString())) {
        case "OnFlagMay2019"_fh: {
            m_client->send_to_gt_client_delayed(
                    player::Peer::build_variant_packet(
                        *varlist,
                        netid,
                        ENET_PACKET_FLAG_RELIABLE
                    ), 300
            );
            *fw_packet = false;
            break;
        }
        case "OnSetRoleSkinsAndIcons"_fh: {
            m_client->send_to_gt_client_delayed(
                    player::Peer::build_variant_packet(
                            *varlist,
                            netid,
                            ENET_PACKET_FLAG_RELIABLE
                    ), 400
            );
            *fw_packet = false;
            break;
        }
        case "OnSetClothing"_fh: {
            m_client->send_to_gt_client_delayed(
                    player::Peer::build_variant_packet(
                            *varlist,
                            netid,
                            ENET_PACKET_FLAG_RELIABLE
                    ), 451
            );
            *fw_packet = false;
            m_execute_once_on_world_enter = false;
            break;
        }
    }
}