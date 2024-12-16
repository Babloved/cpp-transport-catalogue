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
        size_t GetCountUniqueStops() const;
        size_t GetCountAllStops() const;
        void AddStopOnPath(const std::string &stop_name, const std::string &path_name, TransportCatalogue &catalogue);
        bool IsPathLooped() const;
        double CalculateFullPathLenght(const TransportCatalogue &catalogue) const;
        friend class TransportCatalogue;
    private:
        //Дэка с последовательным расположением остановок
        std::deque<std::string_view> ordered_stops;
        //Хэш словарь с доступными остановками на пути
        std::unordered_set<std::string_view> stops_on_path;
    };
}

