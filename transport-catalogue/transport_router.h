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
		std::optional<graph::Router<WeightValue>::RouteInfo> RInfo;// структура на полное время марширута. не класс рутера
		std::vector<TransportCatalogue::EdgeInfo> EInfo;// в предыдущих ревью вы говорили сделать так, как я понял. Сейчас результат заполняется по списку готовых решений (откуда, куда, время)
	};

	TransportRouter() = delete;
	TransportRouter(const TransportCatalogue::TransportCatalogue& catalogue) :
		catalogue_(catalogue), 
		graph_(BuildGraph(catalogue)), 
		router_(graph_)
	{};

	PathInformation BuildPath(std::string_view from, std::string_view to) const;// решил сделать так из-за того что

private://они приватные а не публичные
	const TransportCatalogue::TransportCatalogue& catalogue_;
	graph::DirectedWeightedGraph<WeightValue> graph_;
	graph::Router<WeightValue> router_;
};