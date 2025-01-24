#pragma once

#include "domain.h"
#include "geo.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <deque>

namespace tc{
    template<typename T, typename U>
    class PairHash{
    public:
        size_t operator()(const std::pair<const T *, const U *> &key) const{
            return std::hash<const void *>()(key.first) + std::hash<const void *>()(key.second) * salt_;
        }

    private:
        static const size_t salt_ = 77;
    };

    template<typename T, typename U>
    class PairEqual{
    public:
        bool operator()(const std::pair<const T *, const U *> &lhs, const std::pair<const T *, const U *> &rhs) const{
            return lhs.first == rhs.first && lhs.second == rhs.second;
        }
    };


    class TransportCatalogue{
    public:
        const Path *GetPathByName(std::string_view path_name) const;
        const Stop *GetStopByName(std::string_view stop_name) const;
        //Возвращаемое значение используется для исопльзования ссылки в структуре пути
        using StopDistanceMap = std::unordered_map<std::string_view, double>;
        Stop &
        AddStop(std::string stop_name, const geo::Coordinates &coordinates, const StopDistanceMap &stops_distances);
        Path &AddPath(std::string path_name);
        [[nodiscard]] Path::Distance GetDistanceBetweenStops(const Stop *stop_src, const Stop *stop_dst) const noexcept;

        static size_t GetCountUniqueStopsOnPath(const Path &path) ;
        static size_t GetCountAllStopsOnPath(const Path &path) ;
        void AddStopOnPath(const std::string &stop_name, Path &path);
        static bool IsPathLooped(const Path &path) ;
        Path::Distance CalculateFullPathLength(const Path &path) const;

    private:
        void AddStopsDistances(const Stop *stop_src, const TransportCatalogue::StopDistanceMap &stops_distances);
        template<typename iterator>
        Path::Distance CalculatePathLength(const iterator &it_begin, const iterator &it_end) const;
        std::deque<Path> all_path_;
        std::deque<Stop> all_stops_;
        //Хэш словарь с наименованием остановок
        std::unordered_map<std::string_view, Stop *> stops_names_;
        //Хэш таблица остановок с путями
        std::unordered_map<std::string_view, Path *> paths_names_;
        //Хэш-таблица связанных остановок с растоянием между ними
        std::unordered_map<std::pair<const Stop *, const Stop *>, double,
                PairHash<Stop, Stop>, PairEqual<Stop, Stop>> stop_distances_;

    };

}

















 
