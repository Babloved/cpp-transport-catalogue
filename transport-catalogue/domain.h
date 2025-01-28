#pragma once

#include <string>
#include "geo.h"
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <deque>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
struct Path;
struct Stop{
    const std::string stop_name_;
    //Координаты остановки
    geo::Coordinates coordinates_;
    //Словарь доступных маршрутов через останоку
    std::unordered_set<std::shared_ptr<Path>> paths_on_stop_;
};

struct Path{
    struct Distance{
        double geographic = 0.;
        double custom = 0.;
        Distance &operator+=(const Distance &oth);
    };

    explicit Path(std::string path_name) : path_name_(std::move(path_name)){}

    const std::string path_name_;
    //Дэка с последовательным расположением остановок
    std::deque<std::string_view> ordered_stops_;
    //Хэш словарь с доступными остановками на пути
    std::unordered_set<std::string_view> stops_on_path_;
};