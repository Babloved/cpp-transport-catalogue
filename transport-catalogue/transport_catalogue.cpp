#include "transport_catalogue.h"

using namespace std;
using namespace tc;

TransportCatalogue::Stop &TransportCatalogue::
AddStop(string stop_name, const geo::Coordinates &coordinates, const StopDistanceMap &stops_distances){
    auto bus_stop = bus_stops_names_.find(stop_name);
    if (bus_stop != bus_stops_names_.end()){
        if (coordinates.ValidateData()){
            bus_stop->second->coordinates = coordinates;
            bus_stop->second->linked_stops_distance = CopyWithRelinkDistanceMap(stops_distances);
        }
        return *bus_stop->second;
    }
    all_stops_.push_back({std::move(stop_name), coordinates, {}, CopyWithRelinkDistanceMap(stops_distances)});
    return *bus_stops_names_.insert({all_stops_.back().stop_name, {&all_stops_.back()}}).first->second;
}

[[nodiscard]]TransportCatalogue::StopDistanceMap
TransportCatalogue::CopyWithRelinkDistanceMap(const TransportCatalogue::StopDistanceMap &stops_distances){
    StopDistanceMap copy_stops_distances;
    for (const auto &[name, distance]: stops_distances){
        auto stop = GetStopByName(name);
        if (!stop){
            stop = &AddStop(string{name}, {}, {});
        }
        copy_stops_distances.insert({stop->stop_name, distance});
    }
    return copy_stops_distances;
}

Path &TransportCatalogue::AddPath(string path_name){
    auto &path = all_path_.emplace_back(std::move(path_name));
    return *bus_paths_names_.insert({path.path_name_, {&path}}).first->second;
}

const TransportCatalogue::Stop *
TransportCatalogue::GetStopByName(string_view stop_name) const{
    auto it_bus_stop = bus_stops_names_.find(stop_name);
    if (it_bus_stop == bus_stops_names_.end()){
        return nullptr;
    }
    return it_bus_stop->second;
}

const Path *TransportCatalogue::GetPathByName(string_view path_name) const{
    auto it_found_path = bus_paths_names_.find(path_name);
    if (it_found_path == bus_paths_names_.end()){
        return nullptr;
    }
    return it_found_path->second;
}

void Path::AddStopOnPath(const string &stop_name, TransportCatalogue &catalogue){
    static geo::Coordinates DummyCoordinate;
    auto &bus_stop = catalogue.AddStop(stop_name, DummyCoordinate, {});
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

Path::Distance &Path::Distance::operator+=(const Path::Distance &oth){
    geographic += oth.geographic;
    custom += oth.custom;
    return *this;
}

template<typename iterator>
Path::Distance Path::CalculatePathLength(const TransportCatalogue &catalogue,
                                         const iterator &it_begin,
                                         const iterator &it_end) const{
    Distance total_distance{0., 0.};
    TransportCatalogue::Stop const *prev = nullptr;
    TransportCatalogue::Stop const *current;
    for (auto currentIterator = it_begin; currentIterator != it_end; ++currentIterator){
        current = catalogue.bus_stops_names_.find(*currentIterator)->second;
        double current_distance{0.};
        double geographic_distance{0.};
        if (prev){
            geographic_distance = geo::ComputeDistance(prev->coordinates, current->coordinates);
            auto it_linked_stop = prev->linked_stops_distance.find(current->stop_name);
            if (it_linked_stop != prev->linked_stops_distance.end()){
                current_distance = it_linked_stop->second;
            } else{
                it_linked_stop = current->linked_stops_distance.find(prev->stop_name);
                if (it_linked_stop != current->linked_stops_distance.end()){
                    current_distance = it_linked_stop->second;
                } else{
                    current_distance = geographic_distance;
                }
            }
        }
        total_distance += {geographic_distance, current_distance};
        prev = current;
    }
    return total_distance;
}

Path::Distance Path::CalculateFullPathLength(const TransportCatalogue &catalogue) const{
    if (ordered_stops_.empty()){
        return {};
    }
    auto total_distance = CalculatePathLength(catalogue, ordered_stops_.begin(), ordered_stops_.end());
    if (!this->IsPathLooped()){
        total_distance += CalculatePathLength(catalogue, ordered_stops_.rbegin(), ordered_stops_.rend());
    }
    return total_distance;
}


