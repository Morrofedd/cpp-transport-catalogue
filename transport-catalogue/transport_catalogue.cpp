#include "transport_catalogue.h"
#include <cassert>
namespace TransportCatalogue {
	Bus::Bus(const std::string name, const std::vector<std::string_view>& stops) :
		name_(name)
	{
		for (const auto& stop : stops) {
			stops_.push_back(static_cast<std::string>(stop));
		}
	}

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

	Stop* TransportCatalogue::GetStop(std::string_view name) const
	{
		if (stops_index_map_.find(name) == stops_index_map_.end())
		{
			return nullptr;
		}
		return stops_index_map_.at(name);
	}

	std::set<std::string_view> TransportCatalogue::GetStopInformation(std::string_view name) const
	{
		using namespace std::string_view_literals;
		if (stops_info_index_map_.find(name) == stops_info_index_map_.end())
		{
			return { "not found"sv };//когда нужно будет поменять будет сделалано а так пусть работает...
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
			AddStop(stop);
			if (stops_info_index_map_.find(stop) != stops_info_index_map_.end())
			{
				stops_info_index_map_.at(stop).insert(buses_.back().name_);
				continue;
			}
			stops_info_index_map_[stop] = { buses_.back().name_ };
		}
	}

	Bus* TransportCatalogue::GetBus(std::string_view name) const
	{
		if (buses_index_map_.find(name) == buses_index_map_.end()) {
			return nullptr;
		}
		return buses_index_map_.at(name);
	}

	StatisticOfRoute TransportCatalogue::GetStatisticOfRoute(std::string_view name)const
	{
		StatisticOfRoute temp;
		std::unordered_set<std::string_view> uniq_counter;
		Bus* result = GetBus(name);
		std::string_view prev_stop = result->stops_.at(0);

		for (const auto& stop : result->stops_) {
			temp.distance_of_route_ += ComputeDistance(GetStop(prev_stop)->coord_, GetStop(stop)->coord_);
			uniq_counter.insert(stop);
			prev_stop = stop;
		}
		temp.uniq_stops_ = uniq_counter.size();
		return temp;
	}
}
