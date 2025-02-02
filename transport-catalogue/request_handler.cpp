#include "request_handler.h"
#include <list>

using namespace std;

optional<PathStat> RequestHandler::GetPathStat(const string_view &path_name) const{
    auto p_path = db_.GetPathByName(path_name);
    if (p_path){
        auto distance = db_.CalculateFullPathLength(*p_path);
        return PathStat{distance.custom / distance.geographic,
                        distance.custom,
                        db_.GetCountAllStopsOnPath(*p_path),
                        db_.GetCountUniqueStopsOnPath(*p_path)};
    }
    return {nullopt};
}

std::set<std::shared_ptr<Path>, PathComp> *RequestHandler::GetBusesByStop(const std::string_view &stop_name) const{
    auto p_stop = db_.GetStopByName(stop_name);
    if (p_stop){
        return &p_stop->paths_on_stop_;
    }
    return nullptr;
}

svg::Document RequestHandler::RenderMap() const{
    svg::Document doc;
    vector<geo::Coordinates> all_paths_coordinates;
    for (const auto &path: db_.GetSortedAllPaths()){
        for (const auto &stop: path->stops_on_path_){
            all_paths_coordinates.push_back(stop->coordinates_);
        }
    }
    auto &settings = renderer_.GetRenderSettings();
    renderer::SphereProjector proj(all_paths_coordinates.begin(), all_paths_coordinates.end(),
                                       settings.width_,
                                       settings.height_,
                                       settings.padding_);
    auto &color_palette = renderer_.GetRenderSettings().color_palette_;
    size_t index{0};
    for (const auto &path: db_.GetSortedAllPaths()){
        doc.Add(renderer_.RenderPath(*path, proj, color_palette.at(index%color_palette.size())));
        index++;
    }
    return doc;
}

