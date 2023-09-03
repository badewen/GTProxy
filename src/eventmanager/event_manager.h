#pragma once

#include "eventpp/callbacklist.h"

#include "../world/world.h"
#include "../network/packet.h"
#include "enet/enet.h"
#include "../proton/shared/util/Variant.h"

#define GTPROXY_CB_HANDLE_DECLARE(cb_type) using cb_type##CbHandle = eventpp::internal_::CallbackListBase<cb_type,\
                                            eventpp::DefaultPolicies>::Handle

namespace event_manager
{

    struct user_data {
        // this is used by OnReceivePacket* and OnSendPacket* functions
        // if set, will block packet from going to gt client or sent to the server
        bool block_packet;

        // who knows what you want to put inside here.
        void* data;
    };

    using OnWorldEnter = void(const World& world);
    GTPROXY_CB_HANDLE_DECLARE(OnWorldEnter);

    void AddOnWorldEnter(std::function<OnWorldEnter> cb, const std::string& id);
    void InvokeOnWorldEnter(const World& world);


    using OnReceivePacket = void(ENetPeer* peer, ENetPacket* packet, user_data* data);

    void SetOnReceivePacket(std::function<OnReceivePacket> cb);
    bool InvokeOnReceivePacket(ENetPeer* peer, ENetPacket* packet);


    //these one is an actual event.
    using OnReceiveTankPacket = void(ENetPeer* peer, player::GameUpdatePacket* packet, user_data* data);
    GTPROXY_CB_HANDLE_DECLARE(OnReceiveTankPacket);

    void AddOnReceiveTankPacket(std::function<OnReceiveTankPacket> cb, const std::string& id);

    bool InvokeOnReceiveTankPacket(ENetPeer* peer, player::GameUpdatePacket* packet);


    using OnReceiveRawPacket = void(ENetPeer* peer, ENetPacket* packet, user_data* data);
    GTPROXY_CB_HANDLE_DECLARE(OnReceiveRawPacket);

    void AddOnReceiveRawPacket(std::function<OnReceiveRawPacket> cb, const std::string& id);

    bool InvokeOnReceiveRawPacket(ENetPeer* peer, ENetPacket* packet);


    using OnReceiveVariantlist = void(ENetPeer* peer, VariantList* packet, user_data* data);
    GTPROXY_CB_HANDLE_DECLARE(OnReceiveVariantlist);

    void AddOnReceiveVariantlist(std::function<OnReceiveVariantlist> cb, const std::string& id);

    bool InvokeOnReceiveVariantlist(ENetPeer* peer, VariantList* packet);

};
