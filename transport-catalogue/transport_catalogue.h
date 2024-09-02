#pragma once

#include "domain.h"

#include <deque>
#include <initializer_list>
#include <iostream>
#include <unordered_set>
#include <set>
#include <memory>

namespace TransportCatalogue {

	class TransportCatalogue {
	public:
		void AddStop(const Stop& stop);
		void AddRangesBetweenStops(std::string_view from, std::string_view to, int range);
		Stop* GetStop(std::string_view name) const;
		const std::string_view GetStopByID(std::size_t id)const;
		std::size_t GetStopId(std::string_view name) const;
		size_t GetCountOfStops()const;
		int GetRangesBetweenStops(std::string_view from, std::string_view to) const;
		std::set<std::string_view> GetStopInformation(std::string_view name) const;
		void AddBus(const Bus& bus);
		Bus* GetBus(std::string_view name)const;
		size_t GetCountOfBus()const;
		std::vector<std::string_view> GetNamesOfAllRouts() const;
		std::pair<std::string_view, int> FindBusAndSpan(std::string_view from, std::string_view to) const;
		EdgeInfo RouteInfo(std::size_t from, std::size_t to, double time) const;
		StatisticOfRoute GetStatisticOfRoute(std::string_view name) const;
		void SetRouteSettings(double, double);
		void SetRouteSettings(RouteSettings);
		RouteSettings GetRouteSettings()const;
		
		mutable MinMaxCoords min_max_coords;
	private:

		struct StopPtrHasher {
			std::size_t operator() (const std::pair< Stop*, Stop*> s) const noexcept {
				std::size_t h1 = std::hash<const void*>{}(s.first);
				std::size_t h2 = std::hash<const void*>{}(s.second);
				return h1 ^ (h2 << 1);
			}
		};
	private:
		RouteSettings setting_;
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, std::size_t> stops_ids_;//save stop's id
		std::unordered_map<std::string_view, Stop*> stops_index_map_;
		std::unordered_map<std::string_view, std::set<std::string_view>> stops_info_index_map_;
		std::unordered_map<std::pair< Stop*, Stop*>, int, StopPtrHasher> stops_ranges_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*>buses_index_map_;
	};
}