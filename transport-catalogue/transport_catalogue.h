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
		Stop(const std::string name, const Coordinates coord) :
			name_(name), coord_(coord) {};
		Stop(std::string name) :
			name_(name) {
			coord_.lat = 0;
			coord_.lng = 0;
		}
		std::string name_;
		Coordinates coord_;
	};

	struct Bus {
		std::string name_;
		std::vector<std::string_view> stops_;//здесь хронятся только названия string_view инвалидируется
	}; 
	
	struct StatisticOfRoute {
		std::size_t uniq_stops_ = 0;
		double distance_of_route_ = 0;
		double curvature_ = 0;
	};


	class TransportCatalogue {
	public:
		void AddStop(const Stop& stop);
		void AddStop(const Stop& stop, std::unordered_map<std::string, int> ranges);
		void AddRanges(std::string_view from, std::string_view to, int range);
		Stop* GetStop(std::string_view name) const;
		double GetRanges(std::string_view from, std::string_view to) const;
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