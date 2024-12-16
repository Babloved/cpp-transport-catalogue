#include "transport_catalogue.h"
#include "transport_path.h"

using namespace std;
using namespace tc;


TransportCatalogue::StopData &
TransportCatalogue::AddStop(const string &stop_name, const geo::Coordinates &coordinates){
    if (bus_stops_.count(stop_name)){
        if (coordinates.ValidateData()){
            bus_stops_[stop_name].coordinates = coordinates;
        }
        return bus_stops_.find(stop_name)->second;
    }
    auto &new_bus_stop = *bus_stops_.insert({stop_name, {coordinates, {}, {}}}).first;
    new_bus_stop.second.stop_name = new_bus_stop.first;
    return new_bus_stop.second;
}

pair<const string, Path> &TransportCatalogue::AddPath(const string &path_name){
    return *bus_paths_.insert({path_name, {}}).first;
}

const TransportCatalogue::StopData *
TransportCatalogue::GetStopByName(const string &stop_name) const{
    static pair<const string, TransportCatalogue::StopData> DummyStopData;
    if (bus_stops_.count(stop_name)){
        return &bus_stops_.find(stop_name)->second;
    }
    return nullptr;
}

