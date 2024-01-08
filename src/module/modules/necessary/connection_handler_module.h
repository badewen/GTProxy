// handles the connect and disconnect event from the client and the server.
// handles the login, sets up and update any necessary things, like restore module state, context state, spoofing login data,etc.

#include "../module_base.h"

#include <memory>

#include <enet/enet.h>
#include "../../../utils/login_data_spoof.h"
#include "../../../packet/packet.h"
#include "../../../utils/text_parse.h"

namespace server {
class Server;
}

namespace peer {
class Peer;
}

namespace modules {

class ConnectionHandlerModule : public module::ModuleBase {
public:
    ConnectionHandlerModule(server::Server* server) :
        module::ModuleBase("ConnectionHandler_Module")
    {}

    void on_enable() override;
    void on_disable() override;

private:
    void on_gt_client_connect(std::shared_ptr<peer::Peer> gt_peer);
    void on_gt_client_disconnect(std::shared_ptr<peer::Peer> gt_peer);

    void on_proxy_client_connect(std::shared_ptr<peer::Peer> gt_server_peer);
    void on_proxy_client_disconnect(std::shared_ptr<peer::Peer> gt_server_peer);

    // from the gt client
    void on_outgoing_text_packet(ENetPacket* packet, std::shared_ptr<peer::Peer> gt_peer, bool* fw_packet);
    void on_outgoing_tank_packet(packet::GameUpdatePacket* tank_packet, std::shared_ptr<peer::Peer> gt_peer, bool* fw_pkt);

    void on_receive_redirect_packet_hook(
            VariantList* varlist,
            int32_t net_id,
            std::shared_ptr<peer::Peer> gt_server_peer,
            bool* fw_packet
    );
    void on_receive_hello_packet_hook(ENetPacket* packet, std::shared_ptr<peer::Peer> gt_server_peer, bool* fw_packet);

private:
    std::string m_current_gt_client_meta;
    std::string m_gt_server_ip, m_gt_server_port;
    bool m_use_new_packet {};
    utils::TextParse m_login_data;
    utils::LoginData m_current_spoofed_login_data;
};
}