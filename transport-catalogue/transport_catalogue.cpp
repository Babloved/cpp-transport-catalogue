#include "transport_catalogue.h"
#include "transport_path.h"

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

