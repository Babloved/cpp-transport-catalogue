#include "request_handler.h"


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

std::set<std::shared_ptr<Path>, PathComp> * RequestHandler::GetBusesByStop(const std::string_view &stop_name) const{
    auto p_stop = db_.GetStopByName(stop_name);
    if (p_stop){
        return &p_stop->paths_on_stop_;
    }
    return nullptr;
}

svg::Document RequestHandler::RenderMap() const{
//    svg::Document doc;
//    for (const auto &item: db_.){
//
//    }
//    GetBusesByStop()
//    return renderer_.RenderDocument();
    return {};
}

