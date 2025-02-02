#pragma once
#define _USE_MATH_DEFINES
#include "domain.h"
#include "geo.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <deque>
#include <memory>
#include <string_view>

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
        std::shared_ptr<Path> GetPathByName(std::string_view path_name) const;
        std::shared_ptr<Stop> GetStopByName(std::string_view stop_name) const;
        using StopDistanceMap = std::unordered_map<std::string_view, double>;
        std::shared_ptr<Stop> &
        AddStop(std::string stop_name, const geo::Coordinates &coordinates, const StopDistanceMap &stops_distances);
        std::shared_ptr<Path> & AddPath(std::string path_name);
        void SetPathLooped(Path &path, bool is_looped);
        [[nodiscard]] Path::Distance GetDistanceBetweenStops(const Stop *stop_src, const Stop *stop_dst) const noexcept;
        static size_t GetCountUniqueStopsOnPath(const Path &path);
        static size_t GetCountAllStopsOnPath(const Path &path);
        std::set <std::shared_ptr<Path>, PathComp> GetSortedAllPaths() const;
        std::set<std::shared_ptr<Stop>, StopComp> GetSortedAllStops() const;
        void AddStopOnPath(const std::string &stop_name, std::shared_ptr <Path> path);
        static bool IsPathLooped(const Path &path);
        Path::Distance CalculateFullPathLength(const Path &path) const;
    private:
        void AddStopsDistances(const Stop *stop_src, const TransportCatalogue::StopDistanceMap &stops_distances);
        template<typename iterator>
        Path::Distance CalculatePathLength(const iterator &it_begin, const iterator &it_end) const;
        std::deque<std::shared_ptr<Path>> all_path_;
        std::deque<std::shared_ptr<Stop>> all_stops_;
        //Хэш словарь с наименованием остановок
        std::unordered_map<std::string_view,std::shared_ptr<Stop>> stops_names_;
        //Хэш таблица остановок с путями
        std::unordered_map<std::string_view, std::shared_ptr<Path>> paths_names_;
        //Хэш-таблица связанных остановок с растоянием между ними
        std::unordered_map<std::pair<const Stop *, const Stop *>, double,
                PairHash<Stop, Stop>, PairEqual<Stop, Stop>> stop_distances_;
    };

}

















 
