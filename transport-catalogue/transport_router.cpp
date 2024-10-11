#include "transport_router.h"

graph::DirectedWeightedGraph<WeightValue> TransportRouter::BuildGraph(const TransportCatalogue::TransportCatalogue& catalogue) const
{
    const std::size_t n = catalogue.GetCountOfStops();
    const TransportCatalogue::RouteSettings bus_settings = catalogue.GetRouteSettings();

    //создаём дерево с двумя узлами на каждую остановку
    graph::DirectedWeightedGraph<WeightValue> graph(n * 2);

    for (size_t i = 0; i < n; i++) {
        //связываем между собой пары узлов (остановка1 -> остановка1) через грань, равную времени ожидания автобуса       
        graph.AddEdge(graph::Edge<double>{i + n, i, bus_settings.wait_time});
    }

    const std::vector<std::string_view> routes_list = catalogue.GetNamesOfAllRouts();
    for (const std::string_view cur_route : routes_list) {
        //в зависимости от типа маршрута свой расчёт
        int length = 0;
        const TransportCatalogue::Bus* cur_route_ptr = catalogue.GetBus(cur_route);
        const std::vector<std::string>& stops = cur_route_ptr->stops;
        const int cur_route_size = static_cast<int>(stops.size());

        auto fn = [&](int from, int to,bool flag = true) {
            length += catalogue.GetRangesBetweenStops(
                stops.at(flag ? to - 1 : to + 1),
                stops.at(to)
            );
            graph.AddEdge(
                graph::Edge<WeightValue>{
                    catalogue.GetStopId(stops.at(from)),
                    catalogue.GetStopId(stops.at(to)) + n,
                    ComputeTimeToTravel(length, bus_settings.velocity) / 60
            }
            );
            }; // в отдельную функцию не выношу тк нужно передовать слишком много пораметров

        if (cur_route_ptr->is_roundtrip) {
            graph.AddEdge(
                graph::Edge<WeightValue>{
                    catalogue.GetStopId(stops.at(0)),
                    catalogue.GetStopId(stops.at(0)) + n,
                    bus_settings.wait_time
                }
            );
            for (int departure_idx = 0; departure_idx < cur_route_size - 1; departure_idx++) {
                length = 0;
                for (int destination_idx = departure_idx + 1; destination_idx < cur_route_size; destination_idx++) {
                    fn(departure_idx, destination_idx);
                }
            }
            continue;
        }
        //в прямом направлении
        for (int departure_idx = 0; departure_idx < cur_route_size / 2; departure_idx++) {
            length = 0;
            for (int destination_idx = departure_idx + 1; destination_idx <= cur_route_size / 2; destination_idx++) {
                fn(departure_idx, destination_idx);
            }
        }
        //в обратном направлении
        for (int departure_idx = cur_route_size / 2; departure_idx > 0; departure_idx--) {
            length = 0;
            for (int destination_idx = departure_idx - 1; destination_idx >= 0; destination_idx--) {
                fn(departure_idx, destination_idx,false);
            }
        }

    }
    return graph;
}

void TransportRouter::BuildPath(PathInformation& result, std::string_view from, std::string_view to) const
{
    result.RInfo = router_.BuildRoute(catalogue_.GetStopId(from),catalogue_.GetStopId(to));
    if (result.RInfo == std::nullopt) {
        return;
    }

    graph::Edge temp = graph_.GetEdge(result.RInfo->edges.front());
    result.EInfo.push_back(catalogue_.RouteInfo(temp.from, temp.from, graph_.GetEdge(0).weight));
    result.RInfo->edges.pop_back();

    for (const auto& el : result.RInfo->edges) {
        temp = graph_.GetEdge(el);
        result.EInfo.push_back(catalogue_.RouteInfo(temp.from, temp.to, temp.weight));
    }
}
