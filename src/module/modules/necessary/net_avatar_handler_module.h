#include "../module_base.h"

#include "../../../player/player.h"

namespace peer {
class Peer;
}

namespace modules {
class NetAvatarHandlerModule : public module::ModuleBase {
public:
    explicit NetAvatarHandlerModule(server::Server* proxy_server)
        : module::ModuleBase("NetAvatarHandler_Module")
    {}

    void on_enable() override;
    void on_disable() override;

    std::shared_ptr<NetAvatar> get_current_net_avatar() { return m_current_net_avatar; }

private:
    void on_varlist_hook(VariantList* varlist, int32_t netid, std::shared_ptr<peer::Peer> gt_server_peer, bool* fw_packet);

    void on_spawn(VariantList* varlist);
    void on_remove(VariantList* varlist);

private:
    std::shared_ptr<NetAvatar> m_current_net_avatar;
    std::vector<NetAvatar> m_net_avatar_list;
};
}
