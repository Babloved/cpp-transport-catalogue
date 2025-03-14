#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <cctype> // Добавляем заголовочный файл
using namespace std;
using namespace json;

Document JsonReader::LoadJSON(const string &&s) {
    istringstream strm(s);
    return Load(strm);
}

string Print(const Node &node) {
    ostringstream out;
    Print(Document{node}, out);
    return out.str();
}

Document JsonReader::LoadStreamJSON(istream &input) {
    long long bracket_counter{0};
    string json_raw_string;
    // Пропускаем пробелы перед первым символом '{'
    char ch;
    while (input.get(ch) && isspace(ch)) {}
    if (!input) {
        throw runtime_error("Empty input");
    }
    if (ch != '{') {
        throw runtime_error("Incorrect JSON format, first char not a '{'");
    }
    bracket_counter = 1;
    json_raw_string.push_back(ch);
    // Чтение данных из потока
    while (bracket_counter > 0 && input.get(ch)) {
        switch (ch) {
            case '{': ++bracket_counter; break;
            case '}': --bracket_counter; break;
        }
        json_raw_string.push_back(ch);
    }
    if (bracket_counter != 0) {
        throw runtime_error("Unbalanced brackets in JSON");
    }
    return JsonReader::LoadJSON(move(json_raw_string));
}

void JsonReader::LoadBaseRequestsFromDocumentToDB(const Document &doc, tc::TransportCatalogue &db) {
    auto base_requests = doc.GetRoot().AsDict().at("base_requests").AsArray();
    for (const auto &base_request: base_requests) {
        const auto data = base_request.AsDict();
        const auto &type_data = data.at("type").AsString();
        if (type_data == "Stop") {
            LoadStopRequestToDB(data, db);
        } else if (type_data == "Bus") {
            LoadBusRequestToDB(data, db);
        }
    }
}

void JsonReader::LoadStopRequestToDB(const Dict &data, tc::TransportCatalogue &db) {
    tc::TransportCatalogue::StopDistanceMap stop_distance_map;
    for (const auto &[stop_name, distance]: data.at("road_distances").AsDict()) {
        stop_distance_map.insert({stop_name, distance.AsDouble()});
    }
    db.AddStop(data.at("name").AsString(),
               {data.at("latitude").AsDouble(), data.at("longitude").AsDouble()},
               stop_distance_map
    );
}

void JsonReader::LoadBusRequestToDB(const Dict &data, tc::TransportCatalogue &db) {
    auto &path = db.AddPath(data.at("name").AsString());
    for (const auto &stop_name: data.at("stops").AsArray()) {
        db.AddStopOnPath(stop_name.AsString(), path);
    }
    db.SetPathLooped(*path, data.at("is_roundtrip").AsBool());
}

void JsonReader::ProcessStopRequest(const Dict &data, RequestHandler &request_handler, Builder &builder) {
    auto paths = request_handler.GetBusesByStop(data.at("name").AsString());
    if (paths) {
        builder.Key("buses");
        builder.StartArray();
        for (const auto &path: *paths) {
            builder.Value(path->path_name_);
        }
        builder.EndArray();
    } else {
        builder.Key("error_message").Value("not found");
    }
}

void JsonReader::ProcessBusRequest(const Dict &data, RequestHandler &request_handler,
                                   Builder &json_builder) {
    auto path_stat = request_handler.GetPathStat(data.at("name").AsString());

    if (path_stat) {
        json_builder.Key("curvature").Value(std::round(path_stat->curvature * 100000) / 100000.0);
        json_builder.Key("route_length").Value(path_stat->route_length);
        json_builder.Key("stop_count").Value(static_cast<int>(path_stat->stop_count));
        json_builder.Key("unique_stop_count").Value(static_cast<int>(path_stat->unique_stop_count));
    } else {
        json_builder.Key("error_message").Value("not found");
    }
}


void JsonReader::ProcessRouteRequest(const Dict& data, RequestHandler& request_handler, Builder& json_builder) {
    auto route_result = request_handler.BuildRoute(data.at("from").AsString(), data.at("to").AsString());
    if (route_result) {
        json_builder.Key("total_time").Value(route_result->total_time);
        json_builder.Key("items");
        json_builder.StartArray();
        for (const auto& item : route_result->items) {
            json_builder.StartDict();
            if (item.type == transport_router::RouteItem::Type::WAIT) {
                json_builder.Key("type").Value("Wait");
                json_builder.Key("stop_name").Value(item.name);
                json_builder.Key("time").Value(item.time);
            } else { // BUS
                json_builder.Key("type").Value("Bus");
                json_builder.Key("bus").Value(item.name);
                json_builder.Key("span_count").Value(static_cast<int>(item.span_count));
                json_builder.Key("time").Value(item.time);
            }
            json_builder.EndDict();
        }
        json_builder.EndArray();
    } else {
        json_builder.Key("error_message").Value("not found");
    }
}

transport_router::RoutingSettings JsonReader::LoadRoutingSettingsFromDocument(const json::Document& doc) {
    const auto& routing_settings = doc.GetRoot().AsDict().at("routing_settings").AsDict();
    transport_router::RoutingSettings settings;
    settings.bus_wait_time = routing_settings.at("bus_wait_time").AsInt();
    settings.bus_velocity = routing_settings.at("bus_velocity").AsDouble();
    return settings;
}
Document JsonReader::ProcessRequestsFromDocument(const Document &doc, RequestHandler &request_handler) {
    Builder json_builder;
    json_builder.StartArray();
    auto stat_requests = doc.GetRoot().AsDict().at("stat_requests").AsArray();
    for (const auto &base_request: stat_requests) {
        json_builder.StartDict();
        const auto data = base_request.AsDict();
        json_builder.Key("request_id").Value(data.at("id").AsInt());
        const auto &type_data = data.at("type").AsString();
        if (type_data == "Stop") {
            ProcessStopRequest(data, request_handler, json_builder);
        } else if (type_data == "Bus") {
            ProcessBusRequest(data, request_handler, json_builder);
        } else if (type_data == "Map") {
            ProcessMapRequest(request_handler, json_builder);
        } else if (type_data == "Route") {
            ProcessRouteRequest(data, request_handler, json_builder);
        }
        json_builder.EndDict();
    }
    json_builder.EndArray();
    return Document(json_builder.Build());
}

void JsonReader::ProcessMapRequest(RequestHandler &request_handler, Builder &json_builder) {
    ostringstream map;
    request_handler.RenderMap().Render(map);
    json_builder.Key("map").Value(map.str());
}

svg::Color GetColorFromNode(const Node &color_node) {
    svg::Color color;
    if (color_node.IsArray()) {
        auto &color_array = color_node.AsArray();
        ostringstream oss_color;
        switch (color_array.size()) {
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
    } else if (color_node.IsString()) {
        color = color_node.AsString();
    } else {
        throw logic_error("Node not a type color");
    }
    return color;
}

void JsonReader::LoadRenderSettingsFromDocument(const Document &doc, renderer::MapRenderer &renderer) {
    auto &doc_render_settings = doc.GetRoot().AsDict().at("render_settings").AsDict();
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
    for (const auto &color: color_palette) {
        render_settings.color_palette_.push_back(GetColorFromNode(color));
    }
    renderer = renderer::MapRenderer(render_settings);
}