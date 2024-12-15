#include "transport_catalogue.h"

using namespace std;
using namespace tc;

void TransportCatalogue::AddStopOnPath(const std::string &stop_name, const std::string &path_name){
    static geo::Coordinates DummyCoordinate;
    auto &bus_stop_name = this->AddStop(stop_name, DummyCoordinate);
    auto it_path = bus_paths_.find(path_name);
    paths_on_stops_[bus_stop_name].insert(it_path->first);
    it_path->second.ordered_stops.push_back(bus_stop_name);
    it_path->second.stops_on_path.insert(bus_stop_name);
}

size_t TransportCatalogue::GetCountUniqueStops(const std::string &stop_name) const{
    auto it_bus_path = bus_paths_.find(stop_name);
    if (it_bus_path != bus_paths_.end()){
        return bus_paths_.at(stop_name).stops_on_path.size();
    }
    return 0;

}

size_t TransportCatalogue::GetCountAllStops(const std::string &stop_name) const{
    auto it_bus_path = bus_paths_.find(stop_name);
    if (it_bus_path != bus_paths_.end()){
        return bus_paths_.at(stop_name).ordered_stops.size();
    }
    return 0;
}

bool TransportCatalogue::IsPathLooped(const std::string &stop_name) const{
    auto path = bus_paths_.at(stop_name);
    return path.ordered_stops.front() == path.ordered_stops.back();
}

double TransportCatalogue::CalculateFullPathLenght(const std::string& path_name) const{
    auto path = bus_paths_.at(path_name);
    if (path.ordered_stops.empty()){
        return 0;
    }
    double total_distance{0.};
    geo::Coordinates const *prev = nullptr;
    geo::Coordinates const *current = nullptr;
    for (const auto &stop_name: path.ordered_stops){
        current = &bus_stops_.find(string{stop_name})->second;
        total_distance +=
                prev == nullptr ? 0 : geo::ComputeDistance(*prev, *current);
        prev = current;
    }
    total_distance = IsPathLooped(path_name) ? total_distance : total_distance * 2;
    return total_distance;
}

const TransportCatalogue::Path * TransportCatalogue::GetPathByName(const string &path_name) const{
    auto it_found_path = bus_paths_.find(path_name);
    if (it_found_path == bus_paths_.end()){
        return nullptr;
    }
    return &it_found_path->second;
}

const std::deque<std::string_view>  *TransportCatalogue::GetPathStopsOrdered(const string &path_name) const{
    return &GetPathByName(path_name)->ordered_stops;
}

const std::unordered_set<std::string_view> *TransportCatalogue::GetStopsOnPath(const std::string &path_name) const{
    return &GetPathByName(path_name)->stops_on_path;
}

const std::string & TransportCatalogue::AddStop(const string &stop_name, const geo::Coordinates &coordinates){
    auto it_bus_stop = bus_stops_.find(stop_name);
    if (it_bus_stop != bus_stops_.end()){
        if (coordinates.ValidateData()){
            it_bus_stop->second = coordinates;
        }
        return it_bus_stop->first;
    }
    return bus_stops_.insert({stop_name,coordinates}).first->first;
}

const string & TransportCatalogue::AddPath(const string &path_name){
    return bus_paths_.insert({path_name, {}}).first->first;
}

const geo::Coordinates *
TransportCatalogue::GetCoordinatesByStopName(const std::string &stop_name) const{
    static pair<const string, geo::Coordinates> DummyStopData;
    auto it_bus_stop = bus_stops_.find(stop_name);
    if (it_bus_stop != bus_stops_.end()){
        return &it_bus_stop->second;
    }
    return nullptr;
}

const std::set<std::string_view> *TransportCatalogue::GetPathsOnStop(const std::string &path_name) const{
    auto it_path_on_stops = paths_on_stops_.find(path_name);
    if (it_path_on_stops != paths_on_stops_.end() ){
        return &it_path_on_stops->second;
    }
    return nullptr;
}

bool TransportCatalogue::IsPathExist(const string &path_name) const{
    return bus_paths_.count(path_name);
}

bool TransportCatalogue::IsStopExist(const string &stop_name) const{
    return bus_stops_.count(stop_name);
}


