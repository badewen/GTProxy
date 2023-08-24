#include "event_manager.h"


eventpp::CallbackList<event_manager::OnWorldEnter> OnWorldEnterList {};
std::unordered_map<std::string, event_manager::OnWorldEnterCbHandle> OnWorldEnterHandleList {};

void event_manager::AddOnWorldEnter(event_manager::OnWorldEnter cb, const std::string& id) {
    auto h = OnWorldEnterList.append(cb);
    OnWorldEnterHandleList.insert_or_assign(id, h);
}

void event_manager::InvokeOnWorldEnter(const World& world) {
    OnWorldEnterList(world);
}
