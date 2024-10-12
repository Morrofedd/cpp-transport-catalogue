#include "request_handler.h"
#include "domain.h"

//ctor
request_handler::request_handler(TransportCatalogue::TransportCatalogue& catalogue, json_reader& reader):catalogue_(catalogue),reader_(reader)
{
}

//public class function

void request_handler::MakeRequest(std::ostream& output)
{
    AddInformationInCatalogue(BaseRequestHundle(reader_.GetNode("base_requests")));
    catalogue_.SetRouteSettings(ParseTimeAndVelocity(reader_.GetNode("routing_settings")));
    RequestAnser(output);
}

void request_handler::RequestAnser(std::ostream& output)
{
    reader_.Print(output,catalogue_);
}

void request_handler::AddInformationInCatalogue(const std::vector<std::pair<RequestType, RequestValue>>& request_queue)
{
    //LOG_DURATION_STREAM("AddInformationInCatalogue", std::cerr);
    for (const auto& el : request_queue) {
        if (el.first == RequestType::stop_type) {
            TransportCatalogue::StopAndHisNaiboor stop = std::get<TransportCatalogue::StopAndHisNaiboor>(el.second);
            catalogue_.AddStop(stop.from);
            for (const auto& [to, range] : stop.to)
                catalogue_.AddRangesBetweenStops(stop.from.name, to, range);
        }
        if (el.first == RequestType::bus_type) {
            catalogue_.AddBus(std::get< TransportCatalogue::Bus>(el.second));
        }
    }
}