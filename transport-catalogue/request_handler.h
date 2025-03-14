#pragma once

#include <optional>
#include <string>
#include <utility>
#include <string_view>
#include <set>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "domain.h"
#include "transport_router.h"

struct PathStat {
    double curvature;
    double route_length;
    size_t stop_count;
    size_t unique_stop_count;
};

class RequestHandler {
public:
    RequestHandler(const tc::TransportCatalogue& db,
                   const renderer::MapRenderer& renderer,
                   transport_router::TransportRouter& router)
        : db_(db), renderer_(renderer), router_(router) {
    }

    // Возвращает статистику для маршрута
    [[nodiscard]] std::optional<PathStat> GetPathStat(const std::string_view& path_name) const;

    // Возвращает список маршрутов, проходящих через указанную остановку
    [[nodiscard]] std::set<std::shared_ptr<domain::Path>, domain::PathComp>* GetBusesByStop(
        const std::string_view& stop_name) const;

    // Рендерит карту маршрутов и остановок
    [[nodiscard]] svg::Document RenderMap() const;

    // Строит маршрут между двумя остановками
    [[nodiscard]] std::optional<graph::Router<double>::RouteInfo> BuildRoute(
        const std::string& from, const std::string& to) const;

    [[nodiscard]] const transport_router::TransportRouter& GetRouter() const;

private:
    const tc::TransportCatalogue& db_; // Ссылка на транспортный каталог
    const renderer::MapRenderer& renderer_; // Ссылка на рендерер карты
    transport_router::TransportRouter& router_; // Ссылка на маршрутизатор
};