#pragma once

#include <optional>
#include <string>
#include <utility>
#include <string_view>
#include <set>
#include "transport_catalogue.h"
#include "json.h"
#include "domain.h"
#include "map_renderer.h"

struct PathStat{
    double curvature;
    double route_length;
    size_t stop_count;
    size_t unique_stop_count;

};
class RequestHandler{
public:
    RequestHandler(tc::TransportCatalogue &db) : db_(db){};
    [[nodiscard]] std::optional<PathStat> GetPathStat(const std::string_view &path_name) const;
    [[nodiscard]] std::set<std::shared_ptr<Path>, PathComp> * GetBusesByStop(const std::string_view &stop_name) const;
    [[nodiscard]] svg::Document RenderMap() const;

private:
    const tc::TransportCatalogue &db_;
//    const renderer::MapRenderer& renderer_;
};
