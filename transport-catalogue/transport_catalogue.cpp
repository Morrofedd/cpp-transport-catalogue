#include "transport_catalogue.h"
#include <cassert>

namespace TransportCatalogue {

	void TransportCatalogue::AddStop(const Stop& stop)
	{
		if (stops_index_map_.count(stop.name_) != 0) {
			if (stop.coord_.lat == 0 && stop.coord_.lng == 0) {
				return;
			}
			stops_index_map_.at(stop.name_)->coord_ = stop.coord_;
			return;
		}
		stops_.push_back(stop);
		stops_index_map_.insert({ stops_.back().name_,&stops_.back() });
		if (stops_info_index_map_.find(stops_.back().name_) == stops_info_index_map_.end())
		{
			stops_info_index_map_[stops_.back().name_];
		}

	}

	void TransportCatalogue::AddStopWithRanges(const Stop& stop, std::unordered_map<std::string, int> ranges)
	{
		AddStop(stop);
		for (const auto& [to, range] : ranges) {
			AddRangesBetweenStops(stop.name_, to, range);
		}
	}

	Stop* TransportCatalogue::GetStop(std::string_view name) const
	{
		if (stops_index_map_.find(name) == stops_index_map_.end())
		{
			return nullptr;
		}
		return stops_index_map_.at(name);
	}

	void TransportCatalogue::AddRangesBetweenStops(std::string_view from, std::string_view to, int range)
	{
		if (!GetStop(to)) {
			AddStop(static_cast<std::string>(to));
		}
		stops_ranges_.insert({
				std::make_pair<Stop*, Stop*>(GetStop(from),GetStop(to)),
				range
			});
	}

	double TransportCatalogue::GetRangesBetweenStops(std::string_view from, std::string_view to) const
	{
		std::pair< Stop*, Stop*> stop_pair = std::make_pair<Stop*, Stop*>(GetStop(from), GetStop(to));
		if (stops_ranges_.find(stop_pair) == stops_ranges_.end()) {
			stop_pair = std::make_pair<Stop*, Stop*>(GetStop(to), GetStop(from));
			if (stops_ranges_.find(stop_pair) == stops_ranges_.end()) {
				return 0;
			}
		}
		return stops_ranges_.at(stop_pair);
	}

	std::set<std::string_view> TransportCatalogue::GetStopInformation(std::string_view name) const
	{
		using namespace std::string_view_literals;
		if (stops_info_index_map_.find(name) == stops_info_index_map_.end())
		{
			return { "not found"sv };
		}
		return stops_info_index_map_.at(name);
	}

	void TransportCatalogue::AddBus(const Bus& bus)
	{
		if (buses_index_map_.count(bus.name_) != 0) {
			return;
		}
		buses_.push_back(bus);
		buses_index_map_.insert({ buses_.back().name_,&buses_.back() });
		for (const auto& stop : buses_.back().stops_) {
			AddStop(static_cast<std::string>(stop));
			if (stops_info_index_map_.find(stop) != stops_info_index_map_.end())
			{
				stops_info_index_map_.at(stop).insert(buses_.back().name_);
				continue;
			}
			stops_info_index_map_[stop] = { buses_.back().name_ };
		}
		for (auto i = buses_.back().stops_.begin(); i < buses_.back().stops_.end(); i++) {
			*i = GetStop(*i)->name_;
		}
	}

	Bus* TransportCatalogue::GetBus(std::string_view name) const
	{
		if (buses_index_map_.find(name) == buses_index_map_.end()) {
			return nullptr;
		}
		return buses_index_map_.at(name);
	}

	StatisticOfRoute TransportCatalogue::GetStatisticOfRoute(std::string_view name) const
	{
		StatisticOfRoute temp;
		std::unordered_set<std::string_view> uniq_counter;

		Bus* result = GetBus(name);
		std::string_view prev_stop = result->stops_.at(0);

		for (const auto& stop : result->stops_) {
			temp.distance_of_route_ += GetRangesBetweenStops(prev_stop,stop);
			temp.curvature_ += ComputeDistance(GetStop(prev_stop)->coord_, GetStop(stop)->coord_);
			uniq_counter.insert(stop);
			prev_stop = stop;
		}

		if (temp.curvature_ != 0) {
			temp.curvature_ = temp.distance_of_route_ / temp.curvature_;
		}

		temp.uniq_stops_ = uniq_counter.size();
		return temp;
	}
}
