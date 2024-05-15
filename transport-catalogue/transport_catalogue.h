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
		Bus(const std::string name, const std::vector<std::string_view>& stops);
		std::string name_;
		std::deque<std::string> stops_;
		std::size_t uniq_stops_;
		double distance_of_route_;
	};

	class TransportCatalogue {
		// Реализуйте класс самостоятельно
	public:
		void AddStop(const Stop& stop);
		Stop* GetStop(const std::string_view& name) const;
		std::set<std::string_view> GetStopInformation(const std::string_view& name) const;
		void AddBus(const Bus& bus);
		Bus* GetBus(const std::string_view& name)const;

	private:
		std::deque<Stop>stops_;
		std::unordered_map<std::string_view, Stop*>stops_index_map_;
		std::unordered_map<std::string_view, std::set<std::string_view>>stops_info_index_map_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*>buses_index_map_;
	};
}