#include "request_handler.h"
#include "map_renderer.h"
//В сдачу следующего спринта все будет переработанно
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
    json::Print(json::Document(StatRequestHundle(reader_.GetNode("stat_requests"))), output);
}

//private class function
//будет перенесенно в json_reader!
//изначально думал, чтобы не выносить лишний раз каталог за придел класс, сделать здесь обработчик запросов
json::Node request_handler::StatRequestHundle(const json::Node& Root) const
{
    json::Array result;//Array of map

    for (const auto& request : Root.AsArray()) {//request - map

        int request_id = request.AsMap().at("id").AsInt();
        request_type request_t = GetRequestType(request.AsMap().at("type").AsString());
        std::string_view name;

        if (request_t == request_type::Stop_type) {
            name = request.AsMap().at("name").AsString();
            try {
                result.emplace_back(StopToJSON(request_id, catalogue_.GetStopInformation(name)));
            }catch (const std::out_of_range& e) {
                result.emplace_back(ErrorNode(request_id, e.what()));
            }
        }
        if (request_t == request_type::Bus_type) {
            name = request.AsMap().at("name").AsString();
            try {
                const TransportCatalogue::StatisticOfRoute stats = catalogue_.GetStatisticOfRoute(name);
                result.emplace_back(BusToJSON(request_id, stats));
            }
            catch (const std::out_of_range& e) {
                result.emplace_back(ErrorNode(request_id, e.what()));
            }
        }
        if (request_t == request_type::Map_type) {
            map_renderer render(catalogue_, reader_);
            std::ostringstream os;
            render.MapRander(os);
            result.emplace_back(json::Dict{ { "map",os.str() }, { "request_id",request_id } });
        }
        if (request_t == request_type::unknown_type) {
            continue;
        }
    }

    return result;
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
