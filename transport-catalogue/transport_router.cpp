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

    //загружаем список маршрутов
    //const std::unordered_map<std::string_view, transport_obj::Route*>& routes_list = tc.GetAllRoutes();
    const std::vector<std::string_view> routes_list = catalogue.GetNamesOfAllRouts();

    //пройдёмся по всем маршрутам
    for (const std::string_view cur_route : routes_list) {
        //в зависимости от типа маршрута свой расчёт
        int length = 0;
        const TransportCatalogue::Bus* cur_route_ptr = catalogue.GetBus(cur_route);
        const int cur_route_size = static_cast<int>(cur_route_ptr->stops.size());
        if (cur_route_ptr->is_roundtrip) {
            graph.AddEdge(
                graph::Edge<double>{
                    catalogue.GetStopId(cur_route_ptr->stops.at(0)),
                    catalogue.GetStopId(cur_route_ptr->stops.at(0)) + n,
                    bus_settings.wait_time
                }
            );
            for (int departure_idx = 0; departure_idx < cur_route_size - 1; departure_idx++) {
                length = 0;
                for (int destination_idx = departure_idx + 1;
                    destination_idx < cur_route_size;
                    destination_idx++) {

                    //итоговый путь равен сумме путей по предыдущим узлам, плюс новый
                    length += catalogue.GetRangesBetweenStops(
                        cur_route_ptr->stops.at(destination_idx - 1),
                        cur_route_ptr->stops.at(destination_idx)// sec/60 == minuts
                    );

                    // //создаём новую грань
                    graph.AddEdge(
                        graph::Edge<double>{
                            catalogue.GetStopId(cur_route_ptr->stops.at(departure_idx)),
                            catalogue.GetStopId(cur_route_ptr->stops.at(destination_idx)) + n,
                            ComputeTimeToTravel(length, bus_settings.velocity) / 60
                    }
                    );
                }
            }
            continue;
        }
        //в прямом направлении
        for (int departure_idx = 0; departure_idx < cur_route_size / 2; departure_idx++) {
            length = 0;
            for (int destination_idx = departure_idx + 1; destination_idx <= cur_route_size / 2; destination_idx++) {

                //итоговый путь равен сумме путей по предыдущим узлам, плюс новый
                length += catalogue.GetRangesBetweenStops(
                    cur_route_ptr->stops.at(destination_idx - 1),
                    cur_route_ptr->stops.at(destination_idx)
                );

                //создаём новую грань
                graph.AddEdge(
                    graph::Edge<double>{
                        catalogue.GetStopId(cur_route_ptr->stops.at(departure_idx)),
                        catalogue.GetStopId(cur_route_ptr->stops.at(destination_idx)) + n,
                        ComputeTimeToTravel(length, bus_settings.velocity) / 60// sec/60 == minuts
                }
                );
            }
        }
        //в обратном направлении
        for (int departure_idx = cur_route_size / 2; departure_idx > 0; departure_idx--) {
            length = 0;
            for (int destination_idx = departure_idx - 1; destination_idx >= 0; destination_idx--) {
                //итоговый путь равен сумме путей по предыдущим узлам, плюс новый
                length += catalogue.GetRangesBetweenStops(
                    cur_route_ptr->stops.at(destination_idx + 1),
                    cur_route_ptr->stops.at(destination_idx)
                );

                //создаём новую грань
                graph.AddEdge(
                    graph::Edge<double>{
                        catalogue.GetStopId(cur_route_ptr->stops.at(departure_idx)),
                        catalogue.GetStopId(cur_route_ptr->stops.at(destination_idx)) + n,
                        ComputeTimeToTravel(length, bus_settings.velocity) / 60// sec/60 == minuts
                }
                );
            }
        }

    }

    return graph;
}

std::optional<graph::Router<WeightValue>::RouteInfo> TransportRouter::BuildPath(std::string_view from, std::string_view to) const
{
    return router_.BuildRoute(catalogue_.GetStopId(from),catalogue_.GetStopId(to));
}

const graph::Router<WeightValue>& TransportRouter::GetRouter() const
{
    return router_;
}

const graph::DirectedWeightedGraph<WeightValue>& TransportRouter::GetGraph() const
{
    return graph_;
}
