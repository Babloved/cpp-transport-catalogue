#include "cassert"
#include "json.h"
#include "json_reader.h"
#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <request_handler.h>

using namespace std;
using namespace json;
namespace jsonReader{
    json::Document LoadJSON(const std::string &&s){
        std::istringstream strm(s);
        return json::Load(strm);
    }

    std::string Print(const json::Node &node){
        std::ostringstream out;
        json::Print(json::Document{node}, out);
        return out.str();
    }

    json::Document LoadStreamJSON(std::istream &input){
        const size_t buffer_size = 1024; // Размер буфера
        std::array<char, buffer_size> buffer{};// Буфер для чтения данных
        bool end_json{false};
        long long bracket_counter{0};
        std::string json_raw_string;
        // Проверяем первый символ
        if (input.peek() != '{'){
            throw std::runtime_error("Incorrect JSON format, first char not a '{'");
        }
        // Чтение данных из потока
        while (input && !end_json){
            input.read(buffer.data(), buffer_size);
            std::streamsize bytes_read = input.gcount();
            for (int i = 0; i < bytes_read; ++i){
                switch (buffer[i]){
                    case '{':
                        ++bracket_counter;
                        break;
                    case '}':
                        --bracket_counter;
                        break;
                    default:
                        break;
                }
                json_raw_string.push_back(buffer[i]);

                // Если скобки сбалансированы, выходим из цикла
                if (bracket_counter == 0){
                    end_json = true;
                    break;
                }
            }
        }

        // Проверка на ошибки или несбалансированные скобки
        if ((input.fail() && !input.eof()) || bracket_counter != 0){
            json_raw_string.clear();
            throw std::runtime_error("Incorrect JSON format");
        }
        return LoadJSON(std::move(json_raw_string));
    }

    void LoadDataFromDocumentToDB(const json::Document &doc, tc::TransportCatalogue &db){
        auto base_requests = doc.GetRoot().AsMap().at("base_requests").AsArray();
        for (const auto &base_request: base_requests){
            const auto data = base_request.AsMap();
            const auto & type_data = data.at("type").AsString();
            if (type_data == "Stop"){
                tc::TransportCatalogue::StopDistanceMap stop_distance_map;
                for (const auto &[stop_name, distance]: data.at("road_distances").AsMap()){
                    stop_distance_map.insert({stop_name, distance.AsDouble()});
                }
                db.AddStop(data.at("name").AsString(),
                           {data.at("latitude").AsDouble(), data.at("longitude").AsDouble()},
                           stop_distance_map
                );
            }else if (type_data == "Bus"){
                auto &path =   db.AddPath(data.at("name").AsString());
                for (const auto &stop_name: data.at("stops").AsArray()){
                    db.AddStopOnPath(stop_name.AsString(),path);
                }
                if (!data.at("is_roundtrip").AsBool()){
                    db.AddStopOnPath(data.at("stops").AsArray().front().AsString(), path);
                }
            }
        }
    }

    json::Document ProcessRequestsFromDocument(const Document &doc, tc::TransportCatalogue &catalogue){
        auto stat_requests = doc.GetRoot().AsMap().at("stat_requests").AsArray();
        RequestHandler request_handler(catalogue);
        json::Array output;
        json::Dict response;
        for (const auto &base_request: stat_requests){
            size_t index = &stat_requests.back() - &base_request + 1;
            response["request_id"] = static_cast<int>(index);
            const auto data = base_request.AsMap();
            const auto & type_data = data.at("type").AsString();
            if (type_data == "Stop"){
                auto &paths = *request_handler.GetBusesByStop(data.at("name").AsString());
                json::Array path_vec{};
                for (const auto &path: paths){
                    path_vec.push_back(path->path_name_);
                }
                response["buses"] = path_vec;
            }else if (type_data == "Bus"){
                auto path_stat = request_handler.GetPathStat(data.at("name").AsString());
                if(path_stat){
                    response["curvature"] = path_stat->curvature;
                    response["route_length"] = path_stat->route_length;
                    response["stop_count"] = static_cast<int>(path_stat->stop_count);
                    response["unique_stop_count"] = static_cast<int>(path_stat->unique_stop_count);
                }
            }
            output.push_back(response);
            response.clear();
        }
        return Document(output);
    }
}


