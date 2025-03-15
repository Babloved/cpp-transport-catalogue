#include "request_handler.h"
#include <list>

using namespace std;
using namespace svg;

optional<PathStat> RequestHandler::GetPathStat(const string_view &path_name) const {
    auto p_path = db_.GetPathByName(path_name);
    if (p_path) {
        auto distance = db_.CalculateFullPathLength(*p_path);
        return PathStat{
            distance.custom / distance.geographic,
            distance.custom,
            db_.GetCountAllStopsOnPath(*p_path),
            db_.GetCountUniqueStopsOnPath(*p_path)
        };
    }
    return {nullopt};
}

set<shared_ptr<domain::Path>, domain::PathComp>* RequestHandler::GetBusesByStop(const string_view &stop_name) const {
    auto stop = db_.GetStopByName(stop_name);
    if (stop) {
        return &stop->paths_on_stop_;
    }
    return nullptr;
}

Document RequestHandler::RenderMap() const {
    Document doc;
    vector<geo::Coordinates> all_paths_coordinates;\
    //Создаем SphereProjector
    auto paths = db_.GetSortedAllPaths();
    for (const auto &path: paths) {
        for (const auto &stop: path->stops_on_path_) {
            all_paths_coordinates.push_back(stop->coordinates_);
        }
    }
    auto &settings = renderer_.GetRenderSettings();
    renderer::SphereProjector proj(all_paths_coordinates.begin(), all_paths_coordinates.end(),
                                   settings.width_,
                                   settings.height_,
                                   settings.padding_);
    auto &color_palette = renderer_.GetRenderSettings().color_palette_;

    //Рендерим линии пути
    size_t index{0};
    for (const auto &path: paths) {
        doc.Add(renderer_.RenderPathLine(*path, proj, color_palette.at(index % color_palette.size())));
        index++;
    }

    // Рендерим наименования остановки
    index = 0;
    for (const auto &path: paths) {
        auto texts = renderer_.RenderPathName(*path, proj, color_palette.at(index % color_palette.size()));
        for (auto &text: texts) {
            doc.Add(text);
        }
        index++;
    }
    auto stops = db_.GetSortedAllStops();

    //Удаляем остановки без маршрутов
    auto it_curr = stops.begin();
    while (it_curr != stops.end()) {
        if ((**it_curr).paths_on_stop_.empty()) {
            it_curr = stops.erase(it_curr);
        } else {
            ++it_curr;
        }
    }

    //Рендерим круги на остановках
    for (const auto &stop: stops) {
        doc.Add(renderer_.RenderStopCircle(*stop, proj));
    }

    //Рендерим название остановок
    for (const auto &stop: stops) {
        auto texts = renderer_.RenderStopsName(*stop, proj);
        for (auto &text: texts) {
            doc.Add(text);
        }
    }
    return doc;
}

std::optional<transport_router::RouteResult> RequestHandler::BuildRoute(const std::string& from, const std::string& to) const {
    return router_.BuildRoute(from, to);
}

const transport_router::TransportRouter& RequestHandler::GetRouter() const { return router_; }