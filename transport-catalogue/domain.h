#pragma once

#include "geo.h"
#include <algorithm>
#include <deque>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace domain {
    struct Stop;

    struct Path {
        const std::string path_name_;

        struct Distance {
            double geographic = 0.;
            double custom = 0.;
            Distance &operator+=(const Distance &oth);
        };

        explicit Path(std::string path_name) : path_name_(std::move(path_name)) {
        }

        //Дэка с последовательным расположением остановок
        std::deque<std::shared_ptr<Stop> > ordered_stops_;
        //Хэш словарь с доступными остановками на пути
        std::unordered_set<std::shared_ptr<Stop> > stops_on_path_;
        bool path_looped_{false};
    };

    struct PathComp {
        bool operator()(const std::shared_ptr<Path> &lhs, const std::shared_ptr<Path> &rhs) const {
            return lhs->path_name_ < rhs->path_name_;
        }
    };

    struct Stop {
        const std::string stop_name_;
        //Координаты остановки
        geo::Coordinates coordinates_;
        //Словарь доступных маршрутов через останоку
        std::set<std::shared_ptr<Path>, PathComp> paths_on_stop_;
    };

    struct StopComp {
        bool operator()(const std::shared_ptr<Stop> &lhs, const std::shared_ptr<Stop> &rhs) const {
            return lhs->stop_name_ < rhs->stop_name_;
        }
    };
}
