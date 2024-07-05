#include "json_reader.h"

json::Node StopToJSON(const int id, const std::set<std::string_view>& buses)
{
	json::Dict result;
	result.insert({ "request_id",json::Node(id) });

	if (buses.empty()) {
		result.insert({ "buses", json::Array{} });
		return result;
	}

	json::Array bus_list;

	for (const auto& bus : buses) {
		bus_list.emplace_back(std::string(bus));
	}

	result.insert({ "buses",bus_list });
	return result;
}

json::Node BusToJSON(const int id, const TransportCatalogue::StatisticOfRoute& stat)
{
	json::Dict result;

	result.insert({ "curvature",json::Node(static_cast<double>(stat.curvature)) });
	result.insert({ "request_id",json::Node(id) });
	result.insert({ "route_length",json::Node(stat.distance_of_route) });
	result.insert({ "stop_count",json::Node(static_cast<int>(stat.stop_count)) });
	result.insert({ "unique_stop_count",json::Node(static_cast<int>(stat.uniq_stops)) });

	return result;
}

json::Node ErrorNode(const int id, const std::string& error);

TransportCatalogue::StopAndHisNaiboor JSONtoStop(const json::Node& root)
{
	TransportCatalogue::Stop result;
	std::unordered_map< std::string, int> to;
	json::Dict stop = root.AsMap();

	result.name = stop.at("name").AsString();
	result.coord = { stop.at("latitude").AsDouble(), stop.at("longitude").AsDouble() };

	to.reserve(stop.at("road_distances").AsMap().size());
	for (const auto& [stop_n, range] : stop.at("road_distances").AsMap()) {
		to.insert({ std::string(stop_n), range.AsInt() });
	}

	return { result,to };
}

TransportCatalogue::Bus JSONtoBus(const json::Node& root)
{
	TransportCatalogue::Bus result;
	json::Dict bus = root.AsMap();

	result.name = bus.at("name").AsString();
	result.is_roundtrip = bus.at("is_roundtrip").AsBool();

	for (auto s_stop_node : bus.at("stops").AsArray()) {
		result.stops.emplace_back(s_stop_node.AsString());
	}

	if (!bus.at("is_roundtrip").AsBool()) {
		for (auto begin = bus.at("stops").AsArray().rbegin() + 1; begin < bus.at("stops").AsArray().rend(); begin++) {
			result.stops.emplace_back(begin->AsString());
		}
	}

	return result;
}

std::pair<request_type, RequestValue> JSONtoRequestElement(const json::Node& root)
{
	request_type r_type = GetRequestType(root.AsMap().at("type").AsString());
	if (r_type == request_type::Stop_type) {
		return std::make_pair< request_type, RequestValue>(request_type::Stop_type, JSONtoStop(root));
	}
	if (r_type == request_type::Bus_type) {
		return std::make_pair< request_type, RequestValue>(request_type::Bus_type, JSONtoBus(root));
	}
	throw std::invalid_argument("unknown type");
}

json::Node ErrorNode(const int id, const std::string& error)
{
	return json::Dict{
				{ "request_id",json::Node(id) },
				{ "error_message" , json::Node(error) }
	};
}

json_reader::json_reader(std::istream& input)
{
	root_ = json::Load(input).GetRoot();
}

json::Node json_reader::GetNode(std::string_view request)
{
		return root_.AsMap().at(std::string(request));
}
