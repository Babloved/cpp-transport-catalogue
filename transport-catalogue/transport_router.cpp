#include "transport_router.h"
#include <algorithm>
#include <cmath>

namespace transport_router{
    TransportRouter::TransportRouter(const tc::TransportCatalogue& catalogue)
        : catalogue_(catalogue){}

    void TransportRouter::SetRoutingSettings(const RoutingSettings& settings){
        settings_ = settings;
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

    std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute(const std::string& from,
        const std::string& to) const{
        if (stop_ids_.count(from) == 0 || stop_ids_.count(to) == 0){
            return std::nullopt;
        }
        return router_->BuildRoute(stop_ids_.at(from), stop_ids_.at(to));
    }

    const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const{
        return graph_;
    }

    const std::unordered_map<graph::EdgeId, RouteEdgeInfo>& TransportRouter::GetEdgeInfo() const{
        return edge_id_to_info_;
    }

    const std::unordered_map<graph::VertexId, std::string>& TransportRouter::GetVertexToStopMap() const{
        return vertex_id_to_stop_name_;
    }

    const RoutingSettings& TransportRouter::GetRoutingSettings() const{
        return settings_;
    }
} // namespace transport_router