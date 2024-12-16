#pragma once
#include <iosfwd>
#include <string_view>
#include "transport_catalogue.h"
#include "transport_path.h"

namespace statreader{
    struct CommandDescription {
        // Определяет, задана ли команда (поле command непустое)
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;      // Название команды
        std::string id;           // id маршрута или остановки
        std::string description;  // Параметры команды
    };
    void ParseAndPrintStat(const tc::TransportCatalogue& tansport_catalogue, std::string_view request,
                    std::ostream& output);
}

