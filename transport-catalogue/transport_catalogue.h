#pragma once

#include "geo.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <deque>

namespace tc{
    class TransportCatalogue;

    class TransportCatalogue{
    public:
        size_t GetCountUniqueStops(const std::string &path_name) const;
        size_t GetCountAllStops(const std::string &path_name) const;
        void AddStopOnPath(const std::string &stop_name, const std::string &path_name);
        bool IsPathLooped(const std::string &path_name) const;
        double CalculateFullPathLenght(const std::string &path_name) const;
        bool IsPathExist(const std::string &path_name) const;
        bool IsStopExist(const std::string &stop_name) const;
        const std::deque<std::string_view> *GetPathStopsOrdered(const std::string &path_name) const;
        const std::unordered_set<std::string_view> *GetStopsOnPath(const std::string &path_name) const;
        const std::set<std::string_view> *GetPathsOnStop(const std::string &path_name) const;
        const geo::Coordinates *GetCoordinatesByStopName(const std::string &stop_name) const;
        //Возвращаемое значение используется для исопльзования ссылки в структуре пути
        const std::string &AddStop(const std::string &stop_name, const geo::Coordinates &coordinates);
        const std::string &AddPath(const std::string &path_name);
    private:
        struct Path{
            //Дэка с последовательным расположением остановок
            std::deque<std::string_view> ordered_stops;
            //Хэш словарь с доступными остановками на пути
            std::unordered_set<std::string_view> stops_on_path;
        };
        const Path *GetPathByName(const std::string &path_name) const;
        //Хэш словарь с путями через остановку
        std::unordered_map<std::string_view, std::set<std::string_view>> paths_on_stops_;
        //Хэш словарь с наименованием остановок
        std::unordered_map<std::string, geo::Coordinates> bus_stops_;
        //Хэш таблица остановок с путями
        std::unordered_map<std::string, Path> bus_paths_;
    };
}
 
