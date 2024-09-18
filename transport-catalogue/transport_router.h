#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

class TransportRouter {
private:
	graph::DirectedWeightedGraph<WeightValue> BuildGraph(const TransportCatalogue::TransportCatalogue& catalogue)const;

public:
	TransportRouter() = delete;
	TransportRouter(const TransportCatalogue::TransportCatalogue& catalogue) :
		catalogue_(catalogue), 
		graph_(BuildGraph(catalogue)), 
		router_(graph_) //Cейчас по сути так и работает
	{};

	std::optional<graph::Router<WeightValue>::RouteInfo> BuildPath(std::string_view from,std::string_view to)const;
	const graph::Edge<WeightValue>& GetEdge(graph::EdgeId id)const;//текста в "надо исправить" нет

private:
	const TransportCatalogue::TransportCatalogue& catalogue_;
	graph::DirectedWeightedGraph<WeightValue> graph_;
	graph::Router<WeightValue> router_;
};