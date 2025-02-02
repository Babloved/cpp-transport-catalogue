#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include <array>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace json;

json::Document jsonReader::LoadJSON(const string &&s){
    istringstream strm(s);
    return json::Load(strm);
}

string Print(const json::Node &node){
    ostringstream out;
    json::Print(json::Document{node}, out);
    return out.str();
}

json::Document jsonReader::LoadStreamJSON(istream &input){
    long long bracket_counter{0};
    string json_raw_string;
    // Проверяем первый символ
    char ch;
    input.get(ch);
    if (ch == '{'){
        ++bracket_counter;
        json_raw_string.push_back(ch);
    } else{
        throw runtime_error("Incorrect JSON format, first char not a '{'");
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

void jsonReader::LoadBaseRequestsFromDocumentToDB(const json::Document &doc, tc::TransportCatalogue &db){
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

json::Document jsonReader::ProcessRequestsFromDocument(const Document &doc, RequestHandler &request_handler){
    auto stat_requests = doc.GetRoot().AsMap().at("stat_requests").AsArray();
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
        } else if (type_data == "Map"){
            ProcessMapRequest(request_handler, response);
        }
        output.push_back(response);
    }
    return Document(output);
}

void jsonReader::ProcessMapRequest(RequestHandler &request_handler, json::Dict &response){
    ostringstream map;
    request_handler.RenderMap().Render(map);
    response["map"] = map.str();
}

svg::Color GetColorFromNode(const json::Node &color_node){
    svg::Color color;
    if (color_node.IsArray()){
        auto &color_array = color_node.AsArray();
        ostringstream oss_color;
        switch (color_array.size()){
            case 3:
                oss_color << "rgb(" + to_string(color_array.at(0).AsInt()) << ","
                          << color_array.at(1).AsInt() << ","
                          << color_array.at(2).AsInt() << ")";
                break;
            case 4:
                oss_color << "rgba(" << to_string(color_array.at(0).AsInt()) << ","
                          << color_array.at(1).AsInt() << ","
                          << color_array.at(2).AsInt() << ","
                          << color_array.at(3).AsDouble() << ")";
                break;
            default:
                throw logic_error("The number of color channels is not correct: " + to_string(color_array.size()));
        }
        color = oss_color.str();
    } else if (color_node.IsString()){
        color = color_node.AsString();
    } else{
        throw logic_error("Node not a type color");
    }
    return color;
}

void jsonReader::LoadRenderSettingsFromDocument(const Document &doc, renderer::MapRenderer &renderer){
    auto &doc_render_settings = doc.GetRoot().AsMap().at("render_settings").AsMap();
    renderer::MapRenderer::RenderSettings render_settings;
    render_settings.width_ = doc_render_settings.at("width").AsDouble();
    render_settings.height_ = doc_render_settings.at("height").AsDouble();
    render_settings.padding_ = doc_render_settings.at("padding").AsDouble();
    render_settings.line_width_ = doc_render_settings.at("line_width").AsDouble();
    render_settings.stop_radius_ = doc_render_settings.at("stop_radius").AsDouble();
    render_settings.bus_label_font_size_ = doc_render_settings.at("bus_label_font_size").AsInt();
    auto &bus_label_offset = doc_render_settings.at("bus_label_offset").AsArray();
    render_settings.bus_label_offset_ = {bus_label_offset.front().AsDouble(), bus_label_offset.back().AsDouble()};
    render_settings.stop_label_font_size_ = doc_render_settings.at("stop_label_font_size").AsInt();
    auto &stop_label_offset = doc_render_settings.at("stop_label_offset").AsArray();
    render_settings.stop_label_offset_ = {stop_label_offset.front().AsDouble(), stop_label_offset.back().AsDouble()};
    render_settings.underlayer_color_ = GetColorFromNode(doc_render_settings.at("underlayer_color"));
    render_settings.underlayer_width_ = doc_render_settings.at("underlayer_width").AsDouble();
    auto &color_palette = doc_render_settings.at("color_palette").AsArray();
    for (const auto &color: color_palette){
        render_settings.color_palette_.push_back(GetColorFromNode(color));
    }
    renderer = renderer::MapRenderer(render_settings);
}
