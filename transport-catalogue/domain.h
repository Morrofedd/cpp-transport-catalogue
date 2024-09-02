#pragma once

#include "geo.h"
#include <string>
#include <vector>
#include <string_view>
#include <variant>
#include <unordered_map>
#include <stdexcept>

using namespace geo;

enum request_type {
	unknown_type,
	Bus_type,
	Map_type,
	Stop_type,
	Route_type
};
enum edge_type {
	B_type,
	W_type
};

request_type GetRequestType(const std::string_view s_type);

namespace TransportCatalogue {
	struct Stop {
		Stop() = default;
		Stop(const std::string Name, const Coordinates Coord) :
			name(Name), coord(Coord) {};
		Stop(std::string Name) :
			name(Name) {
			coord.lat = 0;
			coord.lng = 0;
		}
		std::string name;
		Coordinates coord;
	};

	struct Bus {
		std::string name;
		std::vector<std::string> stops;
		bool is_roundtrip;
	};

	struct StatisticOfRoute {
		std::size_t stop_count = 0;
		std::size_t uniq_stops = 0;
		int distance_of_route = 0;
		double curvature = 0;
	};

	struct EdgeInfo
	{
		double time = 0;
		int span = 0;
		std::string_view bus_stop{};
		edge_type type = edge_type::W_type;
	};

	struct StopAndHisNaiboor
	{
		Stop from;
		std::unordered_map< std::string, int> to;
	};

	struct RouteSettings {
		double wait_time = 0;
		double velocity = 0;
	};
}

using RequestValue = std::variant<TransportCatalogue::Bus, TransportCatalogue::StopAndHisNaiboor>;

//time == seconds
static inline double ComputeTimeToTravel(int range, double velocity) {
	velocity = velocity * 1000 / 3600;
	return range / velocity;
}

using WeightValue = double;
