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

    struct Path{
    public:
        size_t GetCountUniqueStops() const;
        size_t GetCountAllStops() const;
        void AddStopOnPath(const std::string &stop_name, const std::string &path_name, TransportCatalogue &catalogue);
        bool IsPathLooped() const;
        double CalculateFullPathLenght(const TransportCatalogue &catalogue) const;
        
    private:
        //Дэка с последовательным расположением остановок
        std::deque<std::string_view> ordered_stops;
        //Хэш словарь с доступными остановками на пути 
        std::unordered_set<std::string_view> stops_on_path;
    };

    class TransportCatalogue{
    public:
        struct StopData{
            //Координаты остановки
            geo::Coordinates coordinates;
            //Словарь доступных маршрутов через останоку
            std::set<std::string_view> paths_names;
            std::string_view stop_name;
        };
        const Path * GetPathByName(const std::string &path_name)const;
        const StopData *GetStopByName(const std::string &stop_name) const;
        //Возвращаемое значение используется для исопльзования ссылки в структуре пути
        StopData & AddStop(const std::string &stop_name, const geo::Coordinates &coordinates);
        std::pair<const std::string, Path> & AddPath(const std::string &path_name);
        friend class Path;
    private:
        //Хэш словарь с наименованием остановок
        std::unordered_map<std::string,StopData> bus_stops_;
        //Хэш таблица остановок с путями
        std::unordered_map<std::string, Path> bus_paths_;
    };
}
 
