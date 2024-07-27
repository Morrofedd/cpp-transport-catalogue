#include "json_reader.h"
#include "map_renderer.h"

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

json::Node ErrorNode(const int id, const std::string& error);

json::Node json_reader::StatRequestHundle(const TransportCatalogue::TransportCatalogue& catalogue) const
{
	json::Array result;//Array of Dicts
	json::Array temp = GetNode("stat_requests").AsArray();

	for (const auto& request : temp) {//request - map

		int request_id = request.AsDict().at("id").AsInt();
		request_type request_t = GetRequestType(request.AsDict().at("type").AsString());
		std::string_view name;

		if (request_t == request_type::Stop_type) {
			name = request.AsDict().at("name").AsString();
			try {
				result.emplace_back(StopToJSON(request_id, catalogue.GetStopInformation(name)));
			}
			catch (const std::out_of_range& e) {
				result.emplace_back(ErrorNode(request_id, e.what()));
			}
		}
		if (request_t == request_type::Bus_type) {
			name = request.AsDict().at("name").AsString();
			try {
				const TransportCatalogue::StatisticOfRoute stats = catalogue.GetStatisticOfRoute(name);
				result.emplace_back(BusToJSON(request_id, stats));
			}
			catch (const std::out_of_range& e) {
				result.emplace_back(ErrorNode(request_id, e.what()));
			}
		}
		if (request_t == request_type::Map_type) {
			map_renderer render(catalogue, *this);
			std::ostringstream os;
			render.MapRander(os);
			result.emplace_back(json::Builder{}.StartDict()
				.Key("map").Value(os.str())
				.Key("request_id").Value(request_id)
				.EndDict().Build());
		}
		if (request_t == request_type::unknown_type) {
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
	request_type r_type = GetRequestType(root.AsDict().at("type").AsString());
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
		return svg::Rgb(
			color.AsArray()[0].AsInt(), 
			color.AsArray()[1].AsInt(), 
			color.AsArray()[2].AsInt()
			);
	}
	return svg::Rgba(
		color.AsArray()[0].AsInt(), 
		color.AsArray()[1].AsInt(), 
		color.AsArray()[2].AsInt(), 
		color.AsArray()[3].AsDouble());
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

void json_reader::Print(std::ostream& output,const TransportCatalogue::TransportCatalogue& catalogue)
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
	stop_label_font_size = temp.at("line_width").AsInt();
	color_palette = JSONtoColor(temp.at("color_palette").AsArray());
	underlayer_color = JSONtoColor(temp.at("underlayer_color"));
	bus_label_offset = JSONtoPoint(temp.at("bus_label_offset").AsArray());
	stop_label_offset = JSONtoPoint(temp.at("stop_label_offset").AsArray());
}
