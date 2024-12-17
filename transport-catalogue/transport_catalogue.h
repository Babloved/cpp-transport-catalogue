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

    class Path{
    public:
        explicit Path(std::string path_name) : path_name_(std::move(path_name)){}

        size_t GetCountUniqueStops() const;
        size_t GetCountAllStops() const;
        void AddStopOnPath(const std::string &stop_name, TransportCatalogue &catalogue);
        bool IsPathLooped() const;
        double CalculateFullPathLenght(const TransportCatalogue &catalogue) const;

        friend class TransportCatalogue;

        const std::string path_name_;
    private:
        //Дэка с последовательным расположением остановок
        std::deque<std::string_view> ordered_stops_;
        //Хэш словарь с доступными остановками на пути
        std::unordered_set<std::string_view> stops_on_path_;
    };

    class TransportCatalogue{
    public:
        struct Stop{
            const std::string stop_name;
            //Координаты остановки
            geo::Coordinates coordinates;
            //Словарь доступных маршрутов через останоку
            std::set<std::string_view> paths_on_stop_;
        };
        const Path *GetPathByName(const std::string &path_name) const;
        const Stop *GetStopByName(const std::string &stop_name) const;
        //Возвращаемое значение используется для исопльзования ссылки в структуре пути
        Stop &AddStop(std::string stop_name, const geo::Coordinates &coordinates);
        Path &AddPath(std::string path_name);

        friend class Path;

    private:
        std::deque<Path> all_path_;
        std::deque<Stop> all_stops_;
        //Хэш словарь с наименованием остановок
        std::unordered_map<std::string_view, Stop *> bus_stops_names_;
        //Хэш таблица остановок с путями
        std::unordered_map<std::string_view, Path *> bus_paths_names_;

    };
}
 
