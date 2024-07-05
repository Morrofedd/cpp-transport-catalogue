#include "domain.h"

request_type GetRequestType(const std::string_view s_type) {
    if (s_type == "Bus") {
        return request_type::Bus_type;
    }
    if (s_type == "Stop") {
        return request_type::Stop_type;
    }
    if (s_type == "Map") {
        return request_type::Map_type;
    }
    return request_type::unknown_type;
}
