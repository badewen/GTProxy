#pragma once

#include "../utils/randutils.hpp"
#include "../utils/random.h"
#include "../utils/hash.h"
#include <spdlog/fmt/bin_to_hex.h>

namespace utils {
class LoginSpoofData {
public:
    static LoginSpoofData Generate() {
        LoginSpoofData ret{};
        ret.Regenerate();
        return ret;
    }

    LoginSpoofData() = default;

    void Regenerate() {
        randutils::pcg_rng gen{ utils::random::get_generator_local() };
        Spoofed_mac = utils::random::generate_mac(gen);
        Spoofed_mac_hash = utils::hash::proton(fmt::format("{}RT", Spoofed_mac).c_str());
        Spoofed_rid = utils::random::generate_hex(gen, 32, true);
        Spoofed_wk = utils::random::generate_hex(gen, 32, true);
        Spoofed_device_id = utils::random::generate_hex(gen, 16, true);
        Spoofed_device_id_hash = utils::hash::proton(fmt::format("{}RT", Spoofed_device_id).c_str());
    }

public:
    std::string Spoofed_mac;
    std::int32_t Spoofed_mac_hash;
    std::string Spoofed_rid;
    std::string Spoofed_wk;
    std::string Spoofed_device_id;
    std::int32_t Spoofed_device_id_hash;

};
}
