#include "input_reader.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <optional>
#include <unordered_map>
#include <charconv>

using namespace inputreader;
using namespace std;
using namespace geo;
using namespace tc;

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str){
    static const double nan = std::nan("");
    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');
    if (comma == str.npos){
        return {nan, nan};
    }
    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));
    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string){
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos){
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim){
    std::vector<std::string_view> result;
    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()){
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos){
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()){
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }
    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route){
    if (route.find('>') != route.npos){
        return Split(route, '>');
    }
    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());
    return results;
}

inline std::optional<size_t> FindPosCharOnNum(char ch, size_t num, string_view str) noexcept{
    size_t result{0};
    size_t last_found_pos{0};
    for (size_t i = num; i > 0; --i){
        last_found_pos = str.find(ch);
        if (last_found_pos == str.npos){
            return std::nullopt;
        } else{
            result += last_found_pos + 1;
        }
        str = str.substr(last_found_pos + 1, str.size());
    }
    return result;
}

TransportCatalogue::StopDistanceMap ParseStopDistance(std::string_view str){
    TransportCatalogue::StopDistanceMap result;
    auto start_pos = FindPosCharOnNum(',', 2, str);
    if (!start_pos){
        return result;
    }
    str = str.substr(*start_pos + 1, str.size());
    auto split_str = Split(str, ',');
    for (const auto &sub_str: split_str){
        auto str_distance = sub_str.substr(sub_str.find_first_not_of(' '), sub_str.find('m'));
        double distance{0};
        from_chars(str_distance.begin(), str_distance.end(), distance);
        auto str_stop_name = sub_str.substr(sub_str.find("to "s) + 3, sub_str.size());
        result.insert({str_stop_name, distance});
    }
    return result;
};

CommandDescription ParseCommandDescription(std::string_view line){
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos){
        return {};
    }
    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos){
        return {};
    }
    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos){
        return {};
    }
    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line){
    auto command_description = ParseCommandDescription(line);
    if (command_description){
        commands_.push_back(std::move(command_description));
    }
}

void ReadAddStop(const CommandDescription &command, TransportCatalogue &catalogue){
    Coordinates coordinates = ParseCoordinates(command.description);
    auto stop_distance_map = ParseStopDistance(command.description);
    catalogue.AddStop(std::string(command.id), coordinates, stop_distance_map);
}

void ReadAddPath(const CommandDescription &command, TransportCatalogue &catalogue){
    std::vector<std::string_view> vec_stops = ParseRoute(command.description);
    auto &path = catalogue.AddPath(command.id);
    for (const auto &stop_name: vec_stops){
        catalogue.AddStopOnPath(std::string(stop_name), path);
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue &catalogue) const{
    for (auto &command: commands_){
        if (command.command == "Stop"){
            ReadAddStop(command, catalogue);
        } else if (command.command == "Bus"){
            ReadAddPath(command, catalogue);
        } else{
            assert("Uncorrected command");
        }
    }

}