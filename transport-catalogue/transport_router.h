#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include <optional>
#include <string>
#include <unordered_map>

namespace transport_router {

    struct RoutingSettings {
        int bus_wait_time = 0;
        double bus_velocity = 0.0;
    };

    struct RouteEdgeInfo {
        std::string bus_name;
        size_t start_stop_idx;
        size_t end_stop_idx;
    };

    class TransportRouter {
    public:
        explicit TransportRouter(const tc::TransportCatalogue& catalogue);

        void SetRoutingSettings(const RoutingSettings& settings);
        std::optional<graph::Router<double>::RouteInfo> BuildRoute(const std::string& from, const std::string& to) const;

        const graph::DirectedWeightedGraph<double>& GetGraph() const;
        const std::unordered_map<graph::EdgeId, RouteEdgeInfo>& GetEdgeInfo() const;
        const std::unordered_map<graph::VertexId, std::string>& GetVertexToStopMap() const;
        const RoutingSettings& GetRoutingSettings() const;
        [[nodiscard]] const graph::Edge<double>& GetEdge(graph::EdgeId edge_id) const;
        [[nodiscard]] const RouteEdgeInfo& GetEdgeInfo(graph::EdgeId edge_id) const;
        [[nodiscard]] std::string GetStopNameByVertexId(graph::VertexId vertex_id) const;
        [[nodiscard]] double GetBusWaitTime() const;

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