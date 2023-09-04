#pragma once

#include "eventpp/callbacklist.h"

#include "../world/world.h"
#include "../network/packet.h"
#include "enet/enet.h"
#include "../proton/shared/util/Variant.h"

#define GTPROXY_CB_HANDLE_NAME(cb_type) cb_type##CbHandle
#define GTPROXY_CB_HANDLE_DECLARE(cb_type) using GTPROXY_CB_HANDLE_NAME(cb_type) = \
                                                eventpp::internal_::CallbackListBase<cb_type,\
                                                eventpp::DefaultPolicies>::Handle

#define GTPROXY_ADD_EVENT_NAME(cb_type) Add##cb_type
#define GTPROXY_SET_EVENT_NAME(cb_type) Set##cb_type
#define GTPROXY_INVOKE_EVENT_NAME(cb_type) Invoke##cb_type

#define GTPROXY_ADD_EVENT_DECLARE(cb_type) void GTPROXY_ADD_EVENT_NAME(cb_type)(std::function<cb_type> cb, const std::string& id)
#define GTPROXY_SET_EVENT_DECLARE(cb_type) void GTPROXY_SET_EVENT_NAME(cb_type)(std::function<cb_type> cb)

#define GTPROXY_INVOKE_PACKET_EVENT_DECLARE(cb_type, ...) bool GTPROXY_INVOKE_EVENT_NAME(cb_type)(__VA_ARGS__)

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

    GTPROXY_ADD_EVENT_DECLARE(OnWorldEnter);
    void InvokeOnWorldEnter(const World& world);


    using OnReceivePacket = void(ENetPeer* peer, ENetPacket* packet, user_data* data);

    GTPROXY_SET_EVENT_DECLARE(OnReceivePacket);
    GTPROXY_INVOKE_PACKET_EVENT_DECLARE(OnReceivePacket,
                                        ENetPeer* peer, ENetPacket* packet);


    //these one is an actual event.
    using OnReceiveTankPacket = void(ENetPeer* peer, player::GameUpdatePacket* packet, user_data* data);
    GTPROXY_CB_HANDLE_DECLARE(OnReceiveTankPacket);

    GTPROXY_ADD_EVENT_DECLARE(OnReceiveTankPacket);
    GTPROXY_INVOKE_PACKET_EVENT_DECLARE(OnReceiveTankPacket,
                                        ENetPeer* peer, player::GameUpdatePacket* packet);


    using OnReceiveRawPacket = void(ENetPeer* peer, ENetPacket* packet, user_data* data);
    GTPROXY_CB_HANDLE_DECLARE(OnReceiveRawPacket);

    GTPROXY_ADD_EVENT_DECLARE(OnReceiveRawPacket);
    GTPROXY_INVOKE_PACKET_EVENT_DECLARE(OnReceiveRawPacket,
                                        ENetPeer* peer, ENetPacket* packet);


    using OnReceiveVariantlist = void(ENetPeer* peer, VariantList* packet, user_data* data);
    GTPROXY_CB_HANDLE_DECLARE(OnReceiveVariantlist);

    GTPROXY_ADD_EVENT_DECLARE(OnReceiveVariantlist);
    GTPROXY_INVOKE_PACKET_EVENT_DECLARE(OnReceiveVariantlist,
                                        ENetPeer* peer, VariantList* packet);


    using OnSendPacket = void(ENetPeer* peer, ENetPacket* packet, user_data* data);
    GTPROXY_CB_HANDLE_DECLARE(OnSendPacket);

    GTPROXY_SET_EVENT_DECLARE(OnSendPacket);
    GTPROXY_INVOKE_PACKET_EVENT_DECLARE(OnSendPacket,
                                        ENetPeer* peer, ENetPacket* packet);

    using OnSendRawPacket = void(ENetPeer* peer, ENetPacket* packet, user_data* data);
    GTPROXY_CB_HANDLE_DECLARE(OnSendRawPacket);

    GTPROXY_ADD_EVENT_DECLARE(OnSendRawPacket);
    GTPROXY_INVOKE_PACKET_EVENT_DECLARE(OnSendRawPacket,
                                        ENetPeer* peer, ENetPacket* packet);

    using OnSendTankPacket = void(ENetPeer* peer, player::GameUpdatePacket* packet, user_data* data);
    GTPROXY_CB_HANDLE_DECLARE(OnSendTankPacket);

    GTPROXY_ADD_EVENT_DECLARE(OnSendTankPacket);
    GTPROXY_INVOKE_PACKET_EVENT_DECLARE(OnSendTankPacket,
                                        ENetPeer* peer, player::GameUpdatePacket* packet);

    using OnSendVariantlist = void(ENetPeer* peer, VariantList* packet, user_data* data);
    GTPROXY_CB_HANDLE_DECLARE(OnSendVariantlist);

    GTPROXY_ADD_EVENT_DECLARE(OnSendVariantlist);
    GTPROXY_INVOKE_PACKET_EVENT_DECLARE(OnSendVariantlist,
                                        ENetPeer* peer, VariantList* packet);


};
