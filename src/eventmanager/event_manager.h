#pragma once

#include "eventpp/callbacklist.h"

#include "../world/world.h"

namespace event_manager
{
    using OnWorldEnter = void(const World& world);
    using OnWorldEnterCbHandle = eventpp::internal_::CallbackListBase<event_manager::OnWorldEnter, eventpp::DefaultPolicies>::Handle;

    void AddOnWorldEnter(OnWorldEnter cb, const std::string& id);
    void InvokeOnWorldEnter(const World& world);
};
