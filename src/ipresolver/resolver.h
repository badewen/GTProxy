#pragma once

#include <string>

namespace Resolver {

enum Status {
    NoError = 0,
    FormErr,
    ServerFail,
    NXDomain,
    NotImp,
    Refused,
    YXDomain,
    XReset,
    NotAuth,
    NotZone
};

struct Result {
    std::string Ip;
    Status Statuz;
};

Result ResolveHostname(std::string hostname);

}