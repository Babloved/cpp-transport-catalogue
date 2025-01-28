#pragma once

#include "json.h"
#include <string>
#include "transport_catalogue.h"

namespace jsonReader{
    [[nodiscard]] json::Document LoadJSON(const std::string &&s);
    [[nodiscard]] json::Document LoadStreamJSON(std::istream &input);
    void LoadDataFromDocumentToDB(const json::Document &doc, tc::TransportCatalogue &catalogue);
    json::Document ProcessRequestsFromDocument(const json::Document &doc, tc::TransportCatalogue &catalogue);
    std::string Print(const json::Node &node);
}