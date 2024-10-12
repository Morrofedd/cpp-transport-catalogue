#include "domain.h"
#include <iostream>

RequestType GetRequestType(const std::string_view s_type) {
    if (s_type == "Bus") {
        return RequestType::bus_type;
    }
    if (s_type == "Stop") {
        return RequestType::stop_type;
    }
    if (s_type == "Map") {
        return RequestType::map_type;
    }
    if (s_type == "Route") {
        return RequestType::route_type;
    }
    return RequestType::unknown_type;
}