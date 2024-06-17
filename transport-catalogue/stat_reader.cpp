#include "stat_reader.h"
#include <string>

enum request_type {
    unknown_type,
    Bus_type,
    Stop_type
};

request_type command_parse(std::string_view command) {
    using namespace std::string_view_literals;
    if (command == "Bus"sv) {
        return request_type::Bus_type;
    }
    if (command == "Stop"sv) {
        return request_type::Stop_type;
    }
    return unknown_type;
}

struct req {
    request_type type;
    std::string name;
};

req ParseRequest(std::string_view str) {
    req temp;
    auto start = str.find_first_of(' ');
    if (start == str.npos) {
        return {};
    }
    temp.type = command_parse(str.substr(0, start));

    start += 1;
    temp.name = str.substr(start, str.find_last_not_of(' ') + 1 - start);

    return temp;
}

void PrintBus(const TransportCatalogue::TransportCatalogue& transport_catalogue, req& request_, std::ostream& output)
{
    TransportCatalogue::Bus* requested = transport_catalogue.GetBus(request_.name);
    if (!requested) {
        output << "Bus " << request_.name << ": not found" << std::endl;
        return;
    }
    TransportCatalogue::StatisticOfRoute statistic = transport_catalogue.GetStatisticOfRoute(request_.name);
    output << "Bus " << requested->name_
        << ": " << requested->stops_.size()<< " stops on route, " 
        << statistic.uniq_stops_<< " unique stops, " 
        << statistic.distance_of_route_<< " route length, " 
        << statistic.curvature_<< " curvature" << std::endl;
}

void PrintStop(const TransportCatalogue::TransportCatalogue& tansport_catalogue, req& request_, std::ostream& output)
{
    std::set<std::string_view> requested = tansport_catalogue.GetStopInformation(request_.name);
    output << "Stop " << request_.name << ": ";
    if (requested.empty()) {
        output << "no buses" << std::endl;
        return;
    }
    bool is_first = true;
    for (const auto& bus : requested) {
        if (is_first) {
            if (bus != "not found") {
                output << "buses ";
            }
            output << bus;
            is_first = false;
            continue;
        }
        output << " " << bus;
    }
    output << std::endl;
}

void ParseAndPrintStat(const TransportCatalogue::TransportCatalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    req request_ = ParseRequest(request);
    if (request_.type == request_type::Bus_type) {
        PrintBus(transport_catalogue, request_, output);
        return;
    }
    if (request_.type == request_type::Stop_type) {
        PrintStop(transport_catalogue, request_, output);
        return;
    }
    output << "unknown command" << std::endl;
}

