#pragma once

#include "geo.h"
#include <string>
#include <vector>
#include <string_view>
#include <variant>
#include <unordered_map>
#include <stdexcept>

using namespace geo;

enum RequestType {
	unknown_type,
	bus_type,
	map_type,
	stop_type,
	route_type
};
enum EdgeType {
	edge_bus_type,
	edge_wait_type
};

RequestType GetRequestType(const std::string_view s_type);

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
		EdgeType type = EdgeType::edge_wait_type;
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
	velocity = velocity * 1000 / 3600;// тут идет (double*int)/int = double | М/С
	return range / velocity;//удобнее было воспринимать в системе СИ
}

using WeightValue = double;
