	#include "transport_catalogue.h"
	#include <cassert>
	#include <cmath>

	namespace TransportCatalogue {

		void TransportCatalogue::AddStop(const Stop& stop)
		{
			if (stops_index_map_.count(stop.name) != 0) {
				if (stop.coord.lat == 0 && stop.coord.lng == 0) {
					return;
				}
				stops_index_map_.at(stop.name)->coord = stop.coord;
				return;
			}
			stops_.push_back(stop);
			stops_index_map_.insert({ stops_.back().name,&stops_.back() });
			if (stops_info_index_map_.find(stops_.back().name) == stops_info_index_map_.end())
			{
				stops_info_index_map_[stops_.back().name];
			}

		}

		void TransportCatalogue::AddRangesBetweenStops(std::string_view from, std::string_view to, int range)
		{
			if (!GetStop(to)) {
				AddStop(std::string(to));
			}
			stops_ranges_.insert({
					std::make_pair<Stop*, Stop*>(GetStop(from),GetStop(to)),
					range
				});
		}

		Stop* TransportCatalogue::GetStop(std::string_view name) const
		{
			if (stops_index_map_.find(name) == stops_index_map_.end())
			{
				return nullptr;
			}
			return stops_index_map_.at(name);
		}

		int TransportCatalogue::GetRangesBetweenStops(std::string_view from, std::string_view to) const
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
				throw std::out_of_range("not found");
			}
			return stops_info_index_map_.at(name);
		}

		void TransportCatalogue::AddBus(const Bus& bus)
		{
			if (buses_index_map_.count(bus.name) != 0) {
				return;
			}
			buses_.push_back(bus);
			buses_index_map_.insert({ buses_.back().name,&buses_.back() });
			for (const auto& stop : buses_.back().stops) {
				AddStop(static_cast<std::string>(stop));
				if (stops_info_index_map_.find(stop) != stops_info_index_map_.end())
				{
					stops_info_index_map_.at(stop).insert(buses_.back().name);
					continue;
				}
				stops_info_index_map_[stop] = { buses_.back().name };
			}
			for (auto i = buses_.back().stops.begin(); i < buses_.back().stops.end(); i++) {
				*i = GetStop(*i)->name;
			}
		}

		Bus* TransportCatalogue::GetBus(std::string_view name) const
		{
			if (buses_index_map_.find(name) == buses_index_map_.end()) {
				return nullptr;
			}
			return buses_index_map_.at(name);
		}

		std::vector<std::string_view> TransportCatalogue::GetNamesOfAllRouts() const
		{
			std::vector<std::string_view> result;

			for (const auto& [name, bus] : buses_index_map_) {
				result.push_back(name);
				for (const auto& stop : bus->stops) {
					min_max_coords.Upload(GetStop(stop)->coord);
				}
			}

			std::sort(result.begin(), result.end());

			return result;
		}

		StatisticOfRoute TransportCatalogue::GetStatisticOfRoute(std::string_view name) const
		{
			StatisticOfRoute temp;
			std::unordered_set<std::string_view> uniq_counter;

			Bus* result = GetBus(name);
			if (!result) {
				throw std::out_of_range("not found");
			}
			std::string_view prev_stop = result->stops.at(0);

			for (const auto& stop : result->stops) {
				temp.distance_of_route += GetRangesBetweenStops(prev_stop, stop);
				temp.curvature += ComputeDistance(GetStop(prev_stop)->coord, GetStop(stop)->coord);
				uniq_counter.insert(stop);
				prev_stop = stop;
			}

			if (temp.curvature != 0) {
				temp.curvature = temp.distance_of_route / temp.curvature;
			}

			if (std::isnan(temp.curvature)) {
				assert(false);
			}

			temp.stop_count = result->stops.size();

			temp.uniq_stops = uniq_counter.size();
			return temp;
		}
	}
