#include "event_manager.h"

using namespace event_manager;

// send help, i am addicted to using macro

#define GTPROXY_HANDLE_LIST_DECLARE(event_name) std::unordered_map<std::string, \
                                                event_name##CbHandle> \
                                                event_name##HandleList {}

// im lazy
#define GTPROXY_OUT_PARAM_TO_RETURN(code)   \
        user_data out {};                   \
        out.block_packet = false;          \
        code                                \
        return out.block_packet;

eventpp::CallbackList<event_manager::OnWorldEnter> OnWorldEnterList {};
GTPROXY_HANDLE_LIST_DECLARE(OnWorldEnter);

std::function<event_manager::OnReceivePacket> OnReceivePacketCb = nullptr;

eventpp::CallbackList<event_manager::OnReceiveTankPacket> OnReceiveTankPacketList {};
GTPROXY_HANDLE_LIST_DECLARE(OnReceiveTankPacket);

eventpp::CallbackList<event_manager::OnReceiveRawPacket> OnReceiveRawPacketList {};
GTPROXY_HANDLE_LIST_DECLARE(OnReceiveRawPacket);

eventpp::CallbackList<event_manager::OnReceiveVariantlist> OnReceiveVariantlistList {};
GTPROXY_HANDLE_LIST_DECLARE(OnReceiveVariantlist);


void event_manager::AddOnWorldEnter(std::function<event_manager::OnWorldEnter> cb, const std::string& id) {
    auto h = OnWorldEnterList.append(cb);
    OnWorldEnterHandleList.insert_or_assign(id, h);
}

void event_manager::InvokeOnWorldEnter(const World& world) {
    OnWorldEnterList(world);
}


void event_manager::SetOnReceivePacket(std::function<event_manager::OnReceivePacket> cb) {
    OnReceivePacketCb = cb;
}

bool event_manager::InvokeOnReceivePacket(ENetPeer* peer, ENetPacket* packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
        OnReceivePacketCb(peer, packet, &out);
    });
}


void event_manager::AddOnReceiveTankPacket(std::function<event_manager::OnReceiveTankPacket> cb, const std::string &id) {
    auto h = OnReceiveTankPacketList.append(cb);
    OnReceiveTankPacketHandleList.insert_or_assign(id, h);
}

bool
event_manager::InvokeOnReceiveTankPacket(ENetPeer *peer, player::GameUpdatePacket *packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
        OnReceiveTankPacketList(peer, packet, &out);
    });
}


void event_manager::AddOnReceiveRawPacket(std::function<OnReceiveRawPacket> cb, const std::string &id) {
    auto h = OnReceiveRawPacketList.append(cb);
    OnReceiveRawPacketHandleList.insert_or_assign(id, h);
}

bool event_manager::InvokeOnReceiveRawPacket(ENetPeer *peer, ENetPacket *packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
        OnReceiveRawPacketList(peer, packet, &out);
    });
}


void event_manager::AddOnReceiveVariantlist(std::function<OnReceiveVariantlist> cb, const std::string &id) {
    auto h = OnReceiveVariantlistList.append(cb);
    OnReceiveVariantlistHandleList.insert_or_assign(id, h);
}

bool event_manager::InvokeOnReceiveVariantlist(ENetPeer *peer, VariantList* packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
       OnReceiveVariantlistList(peer, packet, &out);
    });
}
