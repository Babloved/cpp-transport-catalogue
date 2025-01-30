#pragma once

#include <optional>
#include <string>
#include <utility>
#include <string_view>
#include <set>
#include "transport_catalogue.h"
#include "json.h"
#include "domain.h"

struct PathStat{
    double curvature;
    double route_length;
    size_t stop_count;
    size_t unique_stop_count;

};
// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См.  : https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
class RequestHandler{
public:
    RequestHandler(tc::TransportCatalogue &db):db_(db){};
//    // MapRenderer понадобится в следующей части итогового проекта
//    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<PathStat> GetPathStat(const std::string_view &path_name) const;
    // Возвращает маршруты, проходящие через
    std::set<std::shared_ptr<Path>, PathComp> * GetBusesByStop(const std::string_view &stop_name) const;

//    // Этот метод будет нужен в следующей части итогового проекта
//    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const tc::TransportCatalogue &db_;
//    const renderer::MapRenderer& renderer_;
};
