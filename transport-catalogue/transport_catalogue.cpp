#include "transport_catalogue.h"

#include <cassert>

using namespace std;
using namespace tc;

Stop &TransportCatalogue::
AddStop(string stop_name, const geo::Coordinates &coordinates, const StopDistanceMap &stops_distances){
    auto it_stop = stops_names_.find(stop_name);
    if (it_stop != stops_names_.end()){
        if (coordinates.ValidateData()){
            it_stop->second->coordinates_ = coordinates;
            AddStopsDistances(it_stop->second, stops_distances);
        }
        return *it_stop->second;
    }
    all_stops_.push_back({std::move(stop_name), coordinates, {}});
    it_stop = stops_names_.insert({all_stops_.back().stop_name_, {&all_stops_.back()}}).first;
    AddStopsDistances(&all_stops_.back(), stops_distances);
    return *it_stop->second;
}

void TransportCatalogue::AddStopsDistances(const Stop *stop_src, const TransportCatalogue::StopDistanceMap &stops_distances){
    for (const auto &[name, distance]: stops_distances){
        auto stop_dst = GetStopByName(name);
        if (!stop_dst){
            stop_dst = &AddStop(string{name}, {}, {});
        }
        stop_distances_.insert({{stop_src, stop_dst}, distance});
    }
}

Path::Distance TransportCatalogue::GetDistanceBetweenStops(const Stop *stop_src, const Stop *stop_dst) const noexcept{
    assert(stop_src && stop_dst);
    Path::Distance distance{geo::ComputeDistance(stop_src->coordinates_, stop_dst->coordinates_), 0};
    auto it_custom_distance = stop_distances_.find({stop_src, stop_dst});
    if (it_custom_distance != stop_distances_.end()){
        distance.custom = it_custom_distance->second;
    } else{
        it_custom_distance = stop_distances_.find({stop_dst, stop_src});
        if (it_custom_distance != stop_distances_.end()){
            distance.custom = it_custom_distance->second;
        }
    }
    return distance;
}

Path &TransportCatalogue::AddPath(string path_name){
    auto &path = all_path_.emplace_back(std::move(path_name));
    return *paths_names_.insert({path.path_name_, {&path}}).first->second;
}

const Stop *
TransportCatalogue::GetStopByName(string_view stop_name) const{
    auto it_stop = stops_names_.find(stop_name);
    if (it_stop == stops_names_.end()){
        return nullptr;
    }
    return it_stop->second;
}

const Path *TransportCatalogue::GetPathByName(string_view path_name) const{
    auto it_found_path = paths_names_.find(path_name);
    if (it_found_path == paths_names_.end()){
        return nullptr;
    }
    return it_found_path->second;
}

void TransportCatalogue::AddStopOnPath(const std::string &stop_name, Path &path){
    static geo::Coordinates DummyCoordinate;
    auto &stop = this->AddStop(stop_name, DummyCoordinate, {});
    stop.paths_on_stop_.insert(path.path_name_);
    path.ordered_stops_.push_back(stop.stop_name_);
    path.stops_on_path_.insert(stop.stop_name_);
}

size_t TransportCatalogue::GetCountUniqueStopsOnPath(const Path &path) {
    return path.stops_on_path_.size();
}

size_t TransportCatalogue::GetCountAllStopsOnPath(const Path &path) {
    return path.ordered_stops_.size();
}

bool TransportCatalogue::IsPathLooped(const Path &path) {
    return path.ordered_stops_.front() == path.ordered_stops_.back();
}

Path::Distance &Path::Distance::operator+=(const Path::Distance &oth){
    geographic += oth.geographic;
    custom += oth.custom;
    return *this;
}

template<typename iterator>
Path::Distance TransportCatalogue::CalculatePathLength(const iterator &it_begin, const iterator &it_end) const{
    Path::Distance total_distance{0., 0.};
    Stop const *prev = nullptr;
    Stop const *current;
    for (auto currentIterator = it_begin; currentIterator != it_end; ++currentIterator){
        current = this->stops_names_.find(*currentIterator)->second;
        if (prev){
            total_distance += this->GetDistanceBetweenStops(prev, current);
        }
        prev = current;
    }
    return total_distance;
}

Path::Distance TransportCatalogue::CalculateFullPathLength(const Path &path) const{
    if (path.ordered_stops_.empty()){
        return {};
    }
    auto total_distance = CalculatePathLength(path.ordered_stops_.begin(), path.ordered_stops_.end());
    if (!this->IsPathLooped(path)){
        total_distance += CalculatePathLength(path.ordered_stops_.rbegin(), path.ordered_stops_.rend());
    }
    return total_distance;
}


