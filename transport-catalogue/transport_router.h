#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

class TransportRouter {
private:
	graph::DirectedWeightedGraph<WeightValue> BuildGraph(const TransportCatalogue::TransportCatalogue& catalogue)const;

public:
	struct PathInformation {
		std::optional<graph::Router<WeightValue>::RouteInfo> RInfo;
		std::vector<TransportCatalogue::EdgeInfo> EInfo;
	};
	TransportRouter() = delete;
	TransportRouter(const TransportCatalogue::TransportCatalogue& catalogue) :
		catalogue_(catalogue), 
		graph_(BuildGraph(catalogue)), 
		router_(graph_) //Cейчас по сути так и работает
	{};

	void BuildPath(PathInformation& result, std::string_view from, std::string_view to) const;

private://они приватные а не публичные
	const TransportCatalogue::TransportCatalogue& catalogue_;
	graph::DirectedWeightedGraph<WeightValue> graph_;
	graph::Router<WeightValue> router_;
};