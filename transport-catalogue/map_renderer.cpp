#include "map_renderer.h"
//non-class functions

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
		color.AsArray()[3].AsDouble()
		);
}

svg::Color JSONtoColor(json::Array array,int index) {
	json::Node temp = array[index % array.size()];
	if (temp.IsString()) {
		return temp.AsString();
	}
	if (temp.AsArray().size() == 3) {
		return svg::Rgb(
			temp.AsArray()[0].AsInt(), 
			temp.AsArray()[1].AsInt(), 
			temp.AsArray()[2].AsInt()
			);
	}
	return svg::Rgba(
		temp.AsArray()[0].AsInt(), 
		temp.AsArray()[1].AsInt(), 
		temp.AsArray()[2].AsInt(), 
		temp.AsArray()[3].AsDouble()
		);
}

svg::Point JSONtoPoint(json::Array array) {
	return { array.at(0).AsDouble(), array.at(1).AsDouble() };
}

//class functions

map_renderer::map_renderer(TransportCatalogue::TransportCatalogue& catalogue, json_reader& reader) :catalogue_(catalogue)
{
	settings_ = reader.GetNode("render_settings").AsMap();
}

void map_renderer::MapRander(std::ostream& output)
{
	MakeRenderDocument().Render(output);
}

svg::Document map_renderer::MakeRenderDocument()
{
	auto arr = catalogue_.GetNamesOfAllRouts();

	sp_ = SphereProjector(catalogue_.min_max_coords, settings_.at("width").AsDouble(), settings_.at("height").AsDouble(), settings_.at("padding").AsDouble());

	svg::Document result;
	std::set<std::string_view> UsedStops;

	int index = 0;

	for (const auto& name : arr) {
		try {
			std::set<std::string_view> UsedStopsNow;

			UsedStopsNow = MakeRoute(result, name, index);
			UsedStops.insert(UsedStopsNow.begin(), UsedStopsNow.end());

			index++;
		}
		catch (const std::length_error& ) {
			continue;
		}
	}

	index = 0;
	for (const auto& name : arr) {
		try {
			MakeNameOfRoute(result, name, index);
			index++;
		}
		catch (const std::length_error&) {
			continue;
		}
	}

	for (const auto& name : UsedStops) {
		MakeCircle(name, result);
	}

	for (const auto& name : UsedStops) {
		MakeNameOfStop(result, name);
	}

	return result;
}

std::set<std::string_view> map_renderer::MakeRoute(svg::Document& doc, std::string_view name, int index)
{
	if (catalogue_.GetBus(name)->stops.size()==0) {
		throw std::length_error("");
	}

	std::set<std::string_view> UsedStops;
	svg::Polyline result;
	result.SetFillColor("none").SetStrokeColor(JSONtoColor(settings_.at("color_palette").AsArray(),index)).SetStrokeWidth(settings_.at("line_width").AsDouble());
	result.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeLineCap(svg::StrokeLineCap::ROUND);

	std::vector<geo::Coordinates> coords;
	for (const auto& stop : catalogue_.GetBus(name)->stops) {
		UsedStops.insert(stop);
		coords.push_back(catalogue_.GetStop(stop)->coord);
	}

	for (const auto& geo_coord : coords) {
		result.AddPoint(sp_(geo_coord));
	}

	doc.Add(result);

	return UsedStops;
}

svg::Text map_renderer::MakeText(std::string content, int index, LayerType l_type,request_type r_type)
{
	svg::Text result;

	result
		.SetData(content)
		.SetFontFamily("Verdana");

	if (r_type == request_type::Stop_type) {
		result
			.SetFillColor("black")
			.SetFontSize(settings_.at("stop_label_font_size").AsInt())
			.SetOffset(JSONtoPoint(settings_.at("stop_label_offset").AsArray()));
	}
	if (r_type == request_type::Bus_type) {
		result
			.SetFillColor(JSONtoColor(settings_.at("color_palette").AsArray(), index))
			.SetFontWeight("bold")
			.SetFontSize(settings_.at("bus_label_font_size").AsInt())
			.SetOffset(JSONtoPoint(settings_.at("bus_label_offset").AsArray()));
	}
	if (l_type == LayerType::LOWER) {
		result
			.SetFillColor(JSONtoColor(settings_.at("underlayer_color")))
			.SetStrokeColor(JSONtoColor(settings_.at("underlayer_color")))
			.SetStrokeWidth(settings_.at("underlayer_width").AsDouble())
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	}

	return result;
}

void map_renderer::MakeNameOfRoute(svg::Document& doc,std::string_view name, int index) {
	TransportCatalogue::Bus* bus= catalogue_.GetBus(name);

	if (bus->stops.size() == 0) {
		throw std::length_error("");
	}

	svg::Text lower_text = MakeText(std::string(name), index, LayerType::LOWER, request_type::Bus_type);
	svg::Text upper_text = MakeText(std::string(name), index, LayerType::UPPER, request_type::Bus_type);

	if (bus->is_roundtrip) {
		upper_text.SetPosition(sp_(catalogue_.GetStop(bus->stops.at(0))->coord));
		lower_text.SetPosition(sp_(catalogue_.GetStop(bus->stops.at(0))->coord));

		doc.Add(lower_text);
		doc.Add(upper_text);

		return;
	}

	upper_text.SetPosition(sp_(catalogue_.GetStop(bus->stops.at(0))->coord));
	lower_text.SetPosition(sp_(catalogue_.GetStop(bus->stops.at(0))->coord));

	doc.Add(lower_text);
	doc.Add(upper_text);

	if (bus->stops.at(0)!= bus->stops.at(bus->stops.size() / 2)) {
		upper_text.SetPosition(sp_(catalogue_.GetStop(bus->stops.at(bus->stops.size() / 2))->coord));
		lower_text.SetPosition(sp_(catalogue_.GetStop(bus->stops.at(bus->stops.size() / 2))->coord));

		doc.Add(lower_text);
		doc.Add(upper_text);
	}
}

void map_renderer::MakeNameOfStop(svg::Document& doc, std::string_view name)
{
	TransportCatalogue::Stop* stop_ptr = catalogue_.GetStop(name);

	svg::Text lower_text = MakeText(stop_ptr->name, 1, LayerType::LOWER, request_type::Stop_type);
	svg::Text upper_text = MakeText(stop_ptr->name, 1, LayerType::UPPER, request_type::Stop_type);

	upper_text.SetPosition(sp_(stop_ptr->coord));
	lower_text.SetPosition(sp_(stop_ptr->coord));

	doc.Add(lower_text);
	doc.Add(upper_text);
}

void map_renderer::MakeCircle(std::string_view name, svg::Document& doc) {
	svg::Circle circle;
	circle.SetCenter(sp_(catalogue_.GetStop(name)->coord)).SetRadius(settings_.at("stop_radius").AsDouble()).SetFillColor("white");
	doc.Add(circle);
}