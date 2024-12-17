#include "transport_catalogue.h"

using namespace std;
using namespace tc;


std::pair<const std::string, TransportCatalogue::StopData> &
TransportCatalogue::AddStop(const string &stop_name, const geo::Coordinates &coordinates){
    if (bus_stops_.count(stop_name)){
        if (coordinates.ValidateData()){
            bus_stops_[stop_name].coordinates = coordinates;
        }
        return *bus_stops_.find(stop_name);
    }
    return *bus_stops_.insert({stop_name, {coordinates, {}}}).first;
}

pair<const string, Path> &TransportCatalogue::AddPath(const string &path_name){
    return *bus_paths_.insert({path_name, {}}).first;
}

const std::pair<const std::string, TransportCatalogue::StopData> *
TransportCatalogue::GetStopByName(const string &stop_name) const{
    auto it_bus_stop = bus_stops_.find(stop_name);
    if (it_bus_stop == bus_stops_.end()){
        return nullptr;
    }
    return &*it_bus_stop;
}

void Path::AddStopOnPath(const string &stop_name, const string &path_name,
                         TransportCatalogue &catalogue){
    static geo::Coordinates DummyCoordinate;
    auto &bus_stop = catalogue.AddStop(stop_name, DummyCoordinate);
    bus_stop.second.paths_names.insert(path_name);
    ordered_stops.push_back(bus_stop.first);
    stops_on_path.insert(bus_stop.first);
}

size_t Path::GetCountUniqueStops() const{
    return stops_on_path.size();
}

size_t Path::GetCountAllStops() const{
    return ordered_stops.size();
}

bool Path::IsPathLooped() const{
    return ordered_stops.front() == ordered_stops.back();
}

double Path::CalculateFullPathLenght(const TransportCatalogue &catalogue) const{
    if (ordered_stops.empty()){
        return 0;
    }
    double total_distance{0.};
    pair<const string, TransportCatalogue::StopData> const *prev = nullptr;
    pair<const string, TransportCatalogue::StopData> const *current = nullptr;
    for (const auto &stop_name: ordered_stops){
        current = &*catalogue.bus_stops_.find(string{stop_name});
        total_distance +=
                prev == nullptr ? 0 : geo::ComputeDistance(prev->second.coordinates, current->second.coordinates);
        prev = current;
    }
    total_distance = this->IsPathLooped() ? total_distance : total_distance * 2;
    return total_distance;
}
const std::pair<const std::string, Path> *TransportCatalogue::GetPathByName(const string &path_name) const{
    auto it_found_path = bus_paths_.find(path_name);
    if (it_found_path == bus_paths_.end()){
        return nullptr;
    }
    return &*it_found_path;
}