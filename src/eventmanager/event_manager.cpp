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

#define GTPROXY_ADD_EVENT(cb_type, cb, id) \
        auto h = cb_type##List.append(cb); \
        cb_type##HandleList.insert_or_assign(id, h);

eventpp::CallbackList<OnWorldEnter> OnWorldEnterList {};
GTPROXY_HANDLE_LIST_DECLARE(OnWorldEnter);

std::function<OnReceivePacket> OnReceivePacketCb = nullptr;

eventpp::CallbackList<OnReceiveTankPacket> OnReceiveTankPacketList {};
GTPROXY_HANDLE_LIST_DECLARE(OnReceiveTankPacket);

eventpp::CallbackList<OnReceiveRawPacket> OnReceiveRawPacketList {};
GTPROXY_HANDLE_LIST_DECLARE(OnReceiveRawPacket);

eventpp::CallbackList<OnReceiveVariantlist> OnReceiveVariantlistList {};
GTPROXY_HANDLE_LIST_DECLARE(OnReceiveVariantlist);


std::function<event_manager::OnSendPacket> OnSendPacketCb = nullptr;

eventpp::CallbackList<OnSendRawPacket> OnSendRawPacketList {};
GTPROXY_HANDLE_LIST_DECLARE(OnSendRawPacket);

eventpp::CallbackList<OnSendTankPacket> OnSendTankPacketList {};
GTPROXY_HANDLE_LIST_DECLARE(OnSendTankPacket);

eventpp::CallbackList<OnSendVariantlist> OnSendVariantlistList {};
GTPROXY_HANDLE_LIST_DECLARE(OnSendVariantlist);

// resisting the urge to macro-ify these function. (failed)
// post-reflection thought:
// "die as a hero or live long enough to see yourself become a villain"

void event_manager::AddOnWorldEnter (std::function<event_manager::OnWorldEnter> cb, const std::string& id) {
    GTPROXY_ADD_EVENT(OnWorldEnter, cb, id);
}

void event_manager::InvokeOnWorldEnter (const World& world) {
    OnWorldEnterList(world);
}

void event_manager::RemoveOnWorldEnter(const std::string& id) {
    OnWorldEnterList.remove(OnWorldEnterHandleList[id]);
}

void event_manager::SetOnReceivePacket(std::function<OnReceivePacket> cb) {
    OnReceivePacketCb = std::move(cb);
}

bool event_manager::InvokeOnReceivePacket(ENetPeer* peer, ENetPacket* packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
        OnReceivePacketCb(peer, packet, &out);
    });
}


void event_manager::AddOnReceiveTankPacket(std::function<event_manager::OnReceiveTankPacket> cb, const std::string &id) {
    GTPROXY_ADD_EVENT(OnReceiveTankPacket, cb, id);
}

void event_manager::RemoveOnReceiveTankPacket(const std::string &id) {
    OnReceiveTankPacketList.remove(OnReceiveTankPacketHandleList[id]);
}

bool
event_manager::InvokeOnReceiveTankPacket(ENetPeer *peer, player::GameUpdatePacket *packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
        OnReceiveTankPacketList(peer, packet, &out);
    });
}


void event_manager::AddOnReceiveRawPacket(std::function<OnReceiveRawPacket> cb, const std::string &id) {
    GTPROXY_ADD_EVENT(OnReceiveRawPacket, cb, id);
}

void event_manager::RemoveOnReceiveRawPacket(const std::string &id) {
    OnReceiveRawPacketList.remove(OnReceiveRawPacketHandleList[id]);
}

bool event_manager::InvokeOnReceiveRawPacket(ENetPeer *peer, ENetPacket *packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
        OnReceiveRawPacketList(peer, packet, &out);
    });
}


void event_manager::AddOnReceiveVariantlist(std::function<OnReceiveVariantlist> cb, const std::string &id) {
    GTPROXY_ADD_EVENT(OnReceiveVariantlist, cb, id);
}

void event_manager::RemoveOnReceiveVariantlist(const std::string &id) {
    OnReceiveVariantlistList.remove(OnReceiveVariantlistHandleList[id]);
}

bool event_manager::InvokeOnReceiveVariantlist(ENetPeer *peer, VariantList* packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
       OnReceiveVariantlistList(peer, packet, &out);
    });
}


void event_manager::SetOnSendPacket(std::function<OnSendPacket> cb) {
    OnSendPacketCb = std::move(cb);
}

bool event_manager::InvokeOnSendPacket(ENetPeer *peer, ENetPacket *packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
       OnSendPacketCb(peer, packet, &out);
    });
}


void event_manager::AddOnSendRawPacket(std::function<OnSendRawPacket> cb, const std::string &id) {
    GTPROXY_ADD_EVENT(OnSendRawPacket, cb, id);
}

void event_manager::RemoveOnSendRawPacket(const std::string &id) {
    OnSendRawPacketList.remove(OnSendRawPacketHandleList[id]);
}

bool event_manager::InvokeOnSendRawPacket(ENetPeer *peer, ENetPacket *packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
       OnSendRawPacketList(peer, packet, &out);
    });
}

void event_manager::AddOnSendTankPacket(std::function<OnSendTankPacket> cb, const std::string &id) {
    GTPROXY_ADD_EVENT(OnSendTankPacket, cb, id);
}

void event_manager::RemoveOnSendTankPacket(const std::string &id) {
    OnSendTankPacketList.remove(OnSendTankPacketHandleList[id]);
}

bool event_manager::InvokeOnSendTankPacket(ENetPeer *peer, player::GameUpdatePacket *packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
       OnSendTankPacketList(peer, packet, &out);
    });
}

void event_manager::AddOnSendVariantlist(std::function<OnSendVariantlist> cb, const std::string &id) {
    GTPROXY_ADD_EVENT(OnSendVariantlist, cb, id);
}

void event_manager::RemoveOnSendVariantlist(const std::string &id) {
    OnSendVariantlistList.remove(OnSendVariantlistHandleList[id]);
}

bool event_manager::InvokeOnSendVariantlist(ENetPeer *peer, VariantList *packet) {
    GTPROXY_OUT_PARAM_TO_RETURN({
       OnSendVariantlistList(peer, packet, &out);
    });
}
