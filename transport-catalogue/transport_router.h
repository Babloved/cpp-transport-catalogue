#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include <optional>
#include <string>
#include <unordered_map>

namespace transport_router {

    struct RoutingSettings {
        int bus_wait_time = 0;    // Время ожидания автобуса (минуты)
        double bus_velocity = 0.0; // Скорость автобуса (км/ч)
    };

    struct RouteItem {
        enum class Type {
            WAIT,
            BUS
        };
        Type type;
        std::string name;
        double time;
        size_t span_count;
    };

    struct RouteResult {
        double total_time;
        std::vector<RouteItem> items;
    };

    struct RouteEdgeInfo {
        std::string bus_name;
        size_t start_stop_idx;
        size_t end_stop_idx;
    };

    class TransportRouter {
    public:
        // Конструктор с настройками
        explicit TransportRouter(const tc::TransportCatalogue& catalogue, const RoutingSettings& settings);

        // Единственный публичный метод для построения маршрута
        std::optional<RouteResult> BuildRoute(const std::string& from, const std::string& to) const;

    private:

        void BuildGraph();

        const tc::TransportCatalogue& catalogue_;
        RoutingSettings settings_;
        graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<graph::Router<double>> router_;
        std::unordered_map<std::string_view, graph::VertexId> stop_ids_;
        std::unordered_map<graph::EdgeId, RouteEdgeInfo> edge_id_to_info_;
        std::unordered_map<graph::VertexId, std::string> vertex_id_to_stop_name_;
    };

} // namespace transport_router