#include "transport_catalogue.h"

using namespace std;
using namespace tc;

TransportCatalogue::Stop &TransportCatalogue::AddStop(string stop_name, const geo::Coordinates &coordinates){
    auto bus_stop = bus_stops_names_.find(stop_name);
    if (bus_stop != bus_stops_names_.end()){
        if (coordinates.ValidateData()){
            bus_stop->second->coordinates = coordinates;
        }
        return *bus_stop->second;
    }
    all_stops_.push_back({std::move(stop_name), coordinates, {}});
    return *bus_stops_names_.insert({all_stops_.back().stop_name, {&all_stops_.back()}}).first->second;
}

Path &TransportCatalogue::AddPath(string path_name){
    auto &path = all_path_.emplace_back(std::move(path_name));
    return *bus_paths_names_.insert({path.path_name_, {&path}}).first->second;
}

const TransportCatalogue::Stop *
TransportCatalogue::GetStopByName(const string &stop_name) const{
    auto it_bus_stop = bus_stops_names_.find(stop_name);
    if (it_bus_stop == bus_stops_names_.end()){
        return nullptr;
    }
    return it_bus_stop->second;
}

const Path *TransportCatalogue::GetPathByName(const string &path_name) const{
    auto it_found_path = bus_paths_names_.find(path_name);
    if (it_found_path == bus_paths_names_.end()){
        return nullptr;
    }
    return it_found_path->second;
}

void Path::AddStopOnPath(const string &stop_name, TransportCatalogue &catalogue){
    static geo::Coordinates DummyCoordinate;
    auto &bus_stop = catalogue.AddStop(stop_name, DummyCoordinate);
    bus_stop.paths_on_stop_.insert(path_name_);
    ordered_stops_.push_back(bus_stop.stop_name);
    stops_on_path_.insert(bus_stop.stop_name);
}

size_t Path::GetCountUniqueStops() const{
    return stops_on_path_.size();
}

size_t Path::GetCountAllStops() const{
    return ordered_stops_.size();
}

bool Path::IsPathLooped() const{
    return ordered_stops_.front() == ordered_stops_.back();
}

double Path::CalculateFullPathLenght(const TransportCatalogue &catalogue) const{
    if (ordered_stops_.empty()){
        return 0;
    }
    double total_distance{0.};
    TransportCatalogue::Stop const *prev = nullptr;
    TransportCatalogue::Stop const *current = nullptr;
    for (const auto &stop_name: ordered_stops_){
        current = catalogue.bus_stops_names_.find(stop_name)->second;
        total_distance +=
                prev == nullptr ? 0 : geo::ComputeDistance(prev->coordinates, current->coordinates);
        prev = current;
    }
    total_distance = this->IsPathLooped() ? total_distance : total_distance * 2;
    return total_distance;
}
