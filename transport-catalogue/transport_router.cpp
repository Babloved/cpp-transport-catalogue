#include "transport_router.h"
#include <algorithm>
#include <cmath>

namespace transport_router{
    TransportRouter::TransportRouter(const tc::TransportCatalogue& catalogue, const RoutingSettings& settings)
        : catalogue_(catalogue), settings_(settings) {
        BuildGraph();
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    void TransportRouter::BuildGraph(){
        const auto& stops = catalogue_.GetSortedAllStops();
        graph_ = graph::DirectedWeightedGraph<double>(stops.size() * 2);
        stop_ids_.clear();
        edge_id_to_info_.clear();
        vertex_id_to_stop_name_.clear();

        size_t vertex_id = 0;
        for (const auto& stop : stops){
            stop_ids_[stop->stop_name_] = vertex_id;
            vertex_id_to_stop_name_[vertex_id] = stop->stop_name_;
            graph_.AddEdge({vertex_id, vertex_id + 1, static_cast<double>(settings_.bus_wait_time)});
            vertex_id += 2;
        }

        for (const auto& path : catalogue_.GetSortedAllPaths()){
            const auto& stops_on_path = path->ordered_stops_;
            bool is_looped = path->path_looped_;

            if (stops_on_path.size() < 2){
                continue;
            }

            if (is_looped){
                for (size_t i = 0; i < stops_on_path.size(); ++i){
                    for (size_t j = i + 1; j < stops_on_path.size(); ++j){
                        const auto& from_stop = stops_on_path[i];
                        const auto& to_stop = stops_on_path[j];
                        double distance = 0.0;
                        for (size_t k = i; k < j; ++k){
                            distance += catalogue_.GetDistanceBetweenStops(
                                stops_on_path[k].get(), stops_on_path[k + 1].get()).custom;
                        }
                        double travel_time = distance / (settings_.bus_velocity * 1000.0 / 60.0);
                        auto from_vertex = stop_ids_[from_stop->stop_name_] + 1;
                        auto to_vertex = stop_ids_[to_stop->stop_name_];
                        auto edge_id = graph_.AddEdge({from_vertex, to_vertex, travel_time});
                        edge_id_to_info_[edge_id] = {path->path_name_, i, j};
                    }
                }
            } else {
                // Обработка прямого направления
                for (size_t i = 0; i < stops_on_path.size() - 1; ++i) {
                    double accumulated_distance = 0.0;
                    for (size_t j = i + 1; j < stops_on_path.size(); ++j) {
                        accumulated_distance += catalogue_.GetDistanceBetweenStops(stops_on_path[j-1].get(), stops_on_path[j].get()).custom;
                        double travel_time = accumulated_distance / (settings_.bus_velocity * 1000.0 / 60.0);
                        auto from_vertex = stop_ids_[stops_on_path[i]->stop_name_] + 1;
                        auto to_vertex = stop_ids_[stops_on_path[j]->stop_name_];
                        auto edge_id = graph_.AddEdge({from_vertex, to_vertex, travel_time});
                        edge_id_to_info_[edge_id] = {path->path_name_, i, j};
                    }
                }
                // Обработка обратного направления
                for (int i = static_cast<int>(stops_on_path.size()) - 1; i > 0; --i) {
                    double accumulated_distance = 0.0;
                    for (int j = i - 1; j >= 0; --j) {
                        size_t j_idx = static_cast<size_t>(j);
                        accumulated_distance += catalogue_.GetDistanceBetweenStops(stops_on_path[j+1].get(), stops_on_path[j].get()).custom;
                        double travel_time = accumulated_distance / (settings_.bus_velocity * 1000.0 / 60.0);
                        auto from_vertex = stop_ids_[stops_on_path[i]->stop_name_] + 1;
                        auto to_vertex = stop_ids_[stops_on_path[j_idx]->stop_name_];
                        auto edge_id = graph_.AddEdge({from_vertex, to_vertex, travel_time});
                        edge_id_to_info_[edge_id] = {path->path_name_, j_idx, static_cast<size_t>(i)};
                    }
                }
            }
        }
    }

    std::optional<RouteResult> TransportRouter::BuildRoute(const std::string& from, const std::string& to) const {
        if (stop_ids_.count(from) == 0 || stop_ids_.count(to) == 0) {
            return std::nullopt;
        }
        auto route_info = router_->BuildRoute(stop_ids_.at(from), stop_ids_.at(to));
        if (!route_info) {
            return std::nullopt;
        }

        RouteResult result;
        result.total_time = route_info->weight;
        for (const auto edge_id : route_info->edges) {
            const auto& edge = graph_.GetEdge(edge_id);
            if (edge.to == edge.from + 1 && std::abs(edge.weight - settings_.bus_wait_time) < 1e-6) {
                // Ожидание
                std::string stop_name = vertex_id_to_stop_name_.at(edge.from);
                result.items.push_back({RouteItem::Type::WAIT, stop_name,static_cast<double>(settings_.bus_wait_time), 0});
            } else {
                // Поездка на автобусе
                const auto& edge_info = edge_id_to_info_.at(edge_id);
                size_t span_count = edge_info.end_stop_idx - edge_info.start_stop_idx;
                result.items.push_back({RouteItem::Type::BUS, edge_info.bus_name, edge.weight, span_count});
            }
        }
        return result;
    }
} // namespace transport_router