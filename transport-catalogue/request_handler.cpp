#include "request_handler.h"

//ctor
request_handler::request_handler(TransportCatalogue::TransportCatalogue& catalogue, json_reader& reader):catalogue_(catalogue),reader_(reader)
{
}

//out of classfunction

std::vector < std::pair< request_type, RequestValue>> BaseRequestHundle(const json::Node& Root){

    //LOG_DURATION_STREAM("BaseRequestHundle", std::cerr);
    std::vector < std::pair< request_type, RequestValue>> result;
    result.reserve(Root.AsArray().size());

    for (const auto& request : Root.AsArray()) {
        result.emplace_back(JSONtoRequestElement(request));
    }

    return result;
}

//public class function

void request_handler::MakeRequest(std::ostream& output)
{
    AddInformationInCatalogue(BaseRequestHundle(reader_.GetNode("base_requests")));
    RequestAnser(output);
}

void request_handler::RequestAnser(std::ostream& output)
{
    reader_.Print(output,catalogue_);
}

void request_handler::AddInformationInCatalogue(const std::vector<std::pair<request_type, RequestValue>>& request_queue)
{
    //LOG_DURATION_STREAM("AddInformationInCatalogue", std::cerr);
    for (const auto& el : request_queue) {
        if (el.first == request_type::Stop_type) {
            TransportCatalogue::StopAndHisNaiboor stop = std::get< TransportCatalogue::StopAndHisNaiboor>(el.second);
            catalogue_.AddStop(stop.from);
            for (const auto& [to, range] : stop.to)
                catalogue_.AddRangesBetweenStops(stop.from.name, to, range);
        }
        if (el.first == request_type::Bus_type) {
            catalogue_.AddBus(std::get< TransportCatalogue::Bus>(el.second));
        }
    }
}
