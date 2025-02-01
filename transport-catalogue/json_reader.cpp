#include <cassert>
#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include <array>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace json;

json::Document jsonReader::LoadJSON(const std::string &&s){
    std::istringstream strm(s);
    return json::Load(strm);
}

std::string Print(const json::Node &node){
    std::ostringstream out;
    json::Print(json::Document{node}, out);
    return out.str();
}

json::Document jsonReader::LoadStreamJSON(std::istream &input){
    long long bracket_counter{0};
    std::string json_raw_string;
    // Проверяем первый символ
    char ch;
    input.get(ch);
    if (ch == '{'){
        ++bracket_counter;
        json_raw_string.push_back(ch);
    } else{
        throw std::runtime_error("Incorrect JSON format, first char not a '{'");
    }
    // Чтение данных из потока
    while (bracket_counter > 0){
        input.get(ch);
        switch (ch){
            case '{':
                ++bracket_counter;
                break;
            case '}':
                --bracket_counter;
                break;
            default:
                break;
        }
        json_raw_string.push_back(ch);
    }
    return jsonReader::LoadJSON(std::move(json_raw_string));
}

void jsonReader::LoadStatRequestsFromDocumentToDB(const json::Document &doc, tc::TransportCatalogue &db){
    auto base_requests = doc.GetRoot().AsMap().at("base_requests").AsArray();
    for (const auto &base_request: base_requests){
        const auto data = base_request.AsMap();
        const auto &type_data = data.at("type").AsString();
        if (type_data == "Stop"){
            LoadStopRequestToDB(data, db);
        } else if (type_data == "Bus"){
            LoadBusRequestToDB(data, db);
        }
    }
}


void jsonReader::LoadStopRequestToDB(const json::Dict &data, tc::TransportCatalogue &db){
    tc::TransportCatalogue::StopDistanceMap stop_distance_map;
    for (const auto &[stop_name, distance]: data.at("road_distances").AsMap()){
        stop_distance_map.insert({stop_name, distance.AsDouble()});
    }
    db.AddStop(data.at("name").AsString(),
               {data.at("latitude").AsDouble(), data.at("longitude").AsDouble()},
               stop_distance_map
    );
}

void jsonReader::LoadBusRequestToDB(const json::Dict &data, tc::TransportCatalogue &db){
    auto &path = db.AddPath(data.at("name").AsString());
    for (const auto &stop_name: data.at("stops").AsArray()){
        db.AddStopOnPath(stop_name.AsString(), path);
    }
    db.SetPathLooped(*path, data.at("is_roundtrip").AsBool());
}

void jsonReader::ProcessStopRequest(const json::Dict &data, RequestHandler &request_handler, json::Dict &response){
    auto paths = request_handler.GetBusesByStop(data.at("name").AsString());
    if (paths){
        json::Array path_vec{};
        for (const auto &path: *paths){
            path_vec.push_back(path->path_name_);
        }
        response["buses"] = path_vec;
    } else{
        response["error_message"] = string("not found");
    }
}

void jsonReader::ProcessBusRequest(const json::Dict &data, RequestHandler &request_handler, json::Dict &response){
    auto path_stat = request_handler.GetPathStat(data.at("name").AsString());
    if (path_stat){
        response["curvature"] = path_stat->curvature;
        response["route_length"] = static_cast<int>(path_stat->route_length);
        response["stop_count"] = static_cast<int>(path_stat->stop_count);
        response["unique_stop_count"] = static_cast<int>(path_stat->unique_stop_count);
    } else{
        response["error_message"] = string("not found");
    }
}

json::Document jsonReader::ProcessRequestsFromDocument(const Document &doc, tc::TransportCatalogue &catalogue){
    auto stat_requests = doc.GetRoot().AsMap().at("stat_requests").AsArray();
    RequestHandler request_handler(catalogue);
    json::Array output;
    for (const auto &base_request: stat_requests){
        const auto data = base_request.AsMap();
        json::Dict response;
        response["request_id"] = data.at("id").AsInt();
        const auto &type_data = data.at("type").AsString();
        if (type_data == "Stop"){
            ProcessStopRequest(data, request_handler, response);
        } else if (type_data == "Bus"){
            ProcessBusRequest(data, request_handler, response);
        }
        output.push_back(response);
    }
    return Document(output);
}
void jsonReader::LoadRenderSettingsFromDocument(const json::Document &doc, tc::TransportCatalogue &db){
    //TODO Добавить загрузчк настроек
}



