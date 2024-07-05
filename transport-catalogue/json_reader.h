#pragma once

#include "json.h"
#include "transport_catalogue.h"

class json_reader {
public:
	json_reader() = delete;
	json_reader(std::istream&);

	json::Node GetNode(std::string_view);
private:
	json::Node root_;
};

json::Node StopToJSON(const int id, const std::set<std::string_view>& buses);
json::Node BusToJSON(const int id, const TransportCatalogue::StatisticOfRoute& stat);
json::Node ErrorNode(const int id, const std::string& error);


TransportCatalogue::StopAndHisNaiboor JSONtoStop(const json::Node& root);
TransportCatalogue::Bus JSONtoBus(const json::Node& root);

std::pair<request_type, RequestValue> JSONtoRequestElement(const json::Node& root);
