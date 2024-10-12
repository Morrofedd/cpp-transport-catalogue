#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

json::Node StopToJSON(const int id, const std::set<std::string_view>& buses)
{
	json::Builder result{};
	result.StartDict();
	result.Key("request_id").Value(id);

	if (buses.empty()) {
		result.Key("buses").StartArray().EndArray();
		return result.EndDict().Build();
	}

	result.Key("buses").StartArray();

	for (const auto& bus : buses) {
		result.Value(std::string(bus));
	}

	result.EndArray();
	return result.EndDict().Build();
}

json::Node BusToJSON(const int id, const TransportCatalogue::StatisticOfRoute& stat)
{
	json::Builder result{};
	result.StartDict()
		.Key("curvature").Value(static_cast<double>(stat.curvature))
		.Key("request_id").Value(static_cast<double>(id))
		.Key("route_length").Value(static_cast<double>(stat.distance_of_route))
		.Key("stop_count").Value(static_cast<double>(stat.stop_count))
		.Key("unique_stop_count").Value(static_cast<double>(stat.uniq_stops))
		.EndDict();

	return result.Build();
}

json::Node json_reader::StatRequestHundle(const TransportCatalogue::TransportCatalogue& catalogue) const {
	TransportRouter router(catalogue);
	json::Array result;//Array of Dicts
	json::Array temp = GetNode("stat_requests").AsArray();

	for (const auto& request : temp) {//request - map

		int request_id = request.AsDict().at("id").AsInt();
		RequestType request_t = GetRequestType(request.AsDict().at("type").AsString());
		std::string_view name;

		if (request_t == RequestType::stop_type) {
			name = request.AsDict().at("name").AsString();
			try {
				result.emplace_back(StopToJSON(request_id, catalogue.GetStopInformation(name)));
			}
			catch (const std::out_of_range& e) {
				result.emplace_back(ErrorNode(request_id, e.what()));
			}
		}
		if (request_t == RequestType::bus_type) {
			name = request.AsDict().at("name").AsString();
			try {
				const TransportCatalogue::StatisticOfRoute stats = catalogue.GetStatisticOfRoute(name);
				result.emplace_back(BusToJSON(request_id, stats));
			}
			catch (const std::out_of_range& e) {
				result.emplace_back(ErrorNode(request_id, e.what()));
			}
		}
		if (request_t == RequestType::map_type) {
			map_renderer render(catalogue, *this);
			std::ostringstream os;
			render.MapRander(os);
			result.emplace_back(json::Builder{}.StartDict()
				.Key("map").Value(os.str())
				.Key("request_id").Value(request_id)
				.EndDict().Build());
		}
		if (request_t == RequestType::route_type) {

			std::string_view from = request.AsDict().at("from").AsString();
			std::string_view to = request.AsDict().at("to").AsString();

			json::Builder builder;


			if (from == to) {
				builder.StartDict()
					.Key("total_time").Value(.0)
					.Key("request_id").Value(request_id)
					.Key("items").StartArray()
					.EndArray()
					.EndDict();
				result.emplace_back(builder.Build());
				continue;
			}

			TransportRouter::PathInformation information = router.BuildPath(request.AsDict().at("from").AsString(), request.AsDict().at("to").AsString());

			if (information.RInfo == std::nullopt) {
				result.emplace_back(ErrorNode(request_id, "not found"));
				continue;
			}

			builder.StartDict();
			builder.Key("total_time").Value(information.RInfo->weight)
				.Key("request_id").Value(request_id)
				.Key("items").StartArray();
			for (const auto& el : information.EInfo) {
				builder.StartDict();
				if (el.type == EdgeType::edge_wait_type) {
					builder
						.Key("stop_name").Value(std::string(el.bus_stop))
						.Key("time").Value(el.time)
						.Key("type").Value("Wait").EndDict();
					continue;
				}
				if (el.type == EdgeType::edge_bus_type) {
					builder
						.Key("bus").Value(std::string(el.bus_stop))
						.Key("span_count").Value(el.span)
						.Key("time").Value(el.time)
						.Key("type").Value("Bus").EndDict();
					continue;
				}

			}
			builder.EndArray();


			builder.EndDict();
			result.emplace_back(builder.Build());
		}
		if (request_t == RequestType::unknown_type) {
			continue;
		}
	}

	return result;
}

TransportCatalogue::StopAndHisNaiboor JSONtoStop(const json::Node& root)
{
	TransportCatalogue::Stop result;
	std::unordered_map< std::string, int> to;
	json::Dict stop = root.AsDict();

	result.name = stop.at("name").AsString();
	result.coord = { stop.at("latitude").AsDouble(), stop.at("longitude").AsDouble() };

	to.reserve(stop.at("road_distances").AsDict().size());
	for (const auto& [stop_n, range] : stop.at("road_distances").AsDict()) {
		to.insert({ std::string(stop_n), range.AsInt() });
	}

	return { result,to };
}

TransportCatalogue::Bus JSONtoBus(const json::Node& root)
{
	TransportCatalogue::Bus result;
	json::Dict bus = root.AsDict();

	result.name = bus.at("name").AsString();
	result.is_roundtrip = bus.at("is_roundtrip").AsBool();

	for (const auto& s_stop_node : bus.at("stops").AsArray()) {
		result.stops.emplace_back(s_stop_node.AsString());
	}

	if (!bus.at("is_roundtrip").AsBool()) {
		for (auto begin = bus.at("stops").AsArray().rbegin() + 1; begin < bus.at("stops").AsArray().rend(); begin++) {
			result.stops.emplace_back(begin->AsString());
		}
	}

	return result;
}

std::pair<RequestType, RequestValue> JSONtoRequestElement(const json::Node& root)
{
	RequestType r_type = GetRequestType(root.AsDict().at("type").AsString());
	if (r_type == RequestType::stop_type) {
		return std::make_pair< RequestType, RequestValue>(RequestType::stop_type, JSONtoStop(root));
	}
	if (r_type == RequestType::bus_type) {
		return std::make_pair< RequestType, RequestValue>(RequestType::bus_type, JSONtoBus(root));
	}
	throw std::invalid_argument("unknown type");
}

json::Node ErrorNode(const int id, const std::string& error)
{
	return json::Builder{}.StartDict()
		.Key("request_id").Value(id)
		.Key("error_message").Value(error)
		.EndDict().Build();
}

json_reader::json_reader(std::istream& input)
{
	root_ = json::Load(input).GetRoot();
}

svg::Color JSONtoColor(json::Node color) {
	if (color.IsString()) {
		return color.AsString();
	}
	if (color.AsArray().size() == 3) {
		return svg::Rgb(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt());
	}
	return svg::Rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble());
}

std::vector<svg::Color> JSONtoColor(json::Array array) {
	std::vector<svg::Color> result;
	for (int i = 0; i < array.size();i++) {
		result.push_back(JSONtoColor(array.at(i)));
	}
	return result;
}

svg::Point JSONtoPoint(json::Array array) {
	return { array.at(0).AsDouble(), array.at(1).AsDouble() };
}

void json_reader::Print(
	std::ostream& output,
	const TransportCatalogue::TransportCatalogue& catalogue)
{
	json::Print(json::Document(StatRequestHundle(catalogue)), output);
}

json::Node json_reader::GetNode(std::string_view request)
{
	return root_.AsDict().at(std::string(request));
}
json::Node json_reader::GetNode(std::string_view request)const
{
	return root_.AsDict().at(std::string(request));
}

Settings::Settings(const json_reader& reader)
{	
	json::Dict temp = reader.GetNode("render_settings").AsDict();

	width = temp.at("width").AsDouble();
	height = temp.at("height").AsDouble();
	padding = temp.at("padding").AsDouble();
	underlayer_width = temp.at("underlayer_width").AsDouble();
	line_width = temp.at("line_width").AsDouble();
	stop_radius = temp.at("stop_radius").AsDouble();
	bus_label_font_size = temp.at("bus_label_font_size").AsInt();
	stop_label_font_size = temp.at("stop_label_font_size").AsInt();
	color_palette = JSONtoColor(temp.at("color_palette").AsArray());
	underlayer_color = JSONtoColor(temp.at("underlayer_color"));
	bus_label_offset = JSONtoPoint(temp.at("bus_label_offset").AsArray());
	stop_label_offset = JSONtoPoint(temp.at("stop_label_offset").AsArray());
}

std::vector < std::pair< RequestType, RequestValue>> BaseRequestHundle(const json::Node& root) {

	//LOG_DURATION_STREAM("BaseRequestHundle", std::cerr);
	std::vector < std::pair< RequestType, RequestValue>> result;
	result.reserve(root.AsArray().size());

	for (const auto& request : root.AsArray()) {
		result.emplace_back(JSONtoRequestElement(request));
	}

	return result;
}

TransportCatalogue::RouteSettings ParseTimeAndVelocity(const json::Node& root) {
	return {
		root.AsDict().at("bus_wait_time").AsDouble(),
		root.AsDict().at("bus_velocity").AsDouble() 
	};
}