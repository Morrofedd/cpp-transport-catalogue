#pragma once

#include "geo.h"

#include <deque>
#include <initializer_list>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <memory>

namespace TransportCatalogue {

	struct Stop {
		Stop(const std::string Name, const Coordinates Coord) :
			name(Name), coord(Coord) {};
		Stop(std::string name) :
			name(name) {
			coord.lat = 0;
			coord.lng = 0;
		}
		std::string name;
		Coordinates coord;
	};

	struct Bus {
		std::string name;
		std::vector<std::string_view> stops;
	}; 
	
	struct StatisticOfRoute {
		std::size_t uniq_stops = 0;
		double distance_of_route = 0;
		double curvature = 0;
	};


	class TransportCatalogue {
	public:
		void AddStop(const Stop& stop);
		void AddRangesBetweenStops(std::string_view from, std::unordered_map<std::string,int> stops_and_ranges);
		Stop* GetStop(std::string_view name) const;
		double GetRangesBetweenStops(std::string_view from, std::string_view to) const;
		std::set<std::string_view> GetStopInformation(std::string_view name) const;
		void AddBus(const Bus& bus);
		Bus* GetBus(std::string_view name)const;
		StatisticOfRoute GetStatisticOfRoute(std::string_view name) const;
	private:
		struct StopPtrHasher {
			std::size_t operator() (const std::pair< Stop*, Stop*> s) const noexcept{
				std::size_t h1 = std::hash<const void*>{}(s.first);
				std::size_t h2 = std::hash<const void*>{}(s.second);
				return h1 ^ (h2 << 1);
			}
		};
	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stops_index_map_;
		std::unordered_map<std::string_view, std::set<std::string_view>> stops_info_index_map_;
		std::unordered_map<std::pair< Stop*, Stop*>, int, StopPtrHasher> stops_ranges_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*>buses_index_map_;
	};
}