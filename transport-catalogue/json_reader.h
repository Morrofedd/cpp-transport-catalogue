#pragma once

#include "svg.h"
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"

class json_reader {
public:
	json_reader() = delete;
	json_reader(std::istream&);

	void Print(std::ostream& output, const TransportCatalogue::TransportCatalogue& catalogue);
	json::Node GetNode(std::string_view);
	json::Node GetNode(std::string_view request)const;
	json::Node StatRequestHundle(const TransportCatalogue::TransportCatalogue& catalogue) const;
private:
	json::Node root_;
};

json::Node StopToJSON(const int id, const std::set<std::string_view>& buses);
json::Node BusToJSON(const int id, const TransportCatalogue::StatisticOfRoute& stat);
json::Node ErrorNode(const int id, const std::string& error);

TransportCatalogue::StopAndHisNaiboor JSONtoStop(const json::Node& root);
TransportCatalogue::Bus JSONtoBus(const json::Node& root);

std::pair<request_type, RequestValue> JSONtoRequestElement(const json::Node& root);

struct Settings {
	Settings() = delete;
	Settings(const json_reader&);

	double width;
	double height;
	double padding;
	double underlayer_width;
	double line_width;
	double stop_radius;
	int bus_label_font_size;
	int stop_label_font_size;
	std::vector<svg::Color> color_palette;
	svg::Color underlayer_color;
	svg::Point bus_label_offset;
	svg::Point stop_label_offset;
};
