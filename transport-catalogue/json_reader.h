#pragma once

#include "json.h"
#include <string>
#include "transport_catalogue.h"
#include "request_handler.h"

namespace jsonReader{
    [[nodiscard]] json::Document LoadJSON(const std::string &&s);
    [[nodiscard]] json::Document LoadStreamJSON(std::istream &input);
    void LoadStatRequestsFromDocumentToDB(const json::Document &doc, tc::TransportCatalogue &db);
    json::Document ProcessRequestsFromDocument(const json::Document &doc, tc::TransportCatalogue &catalogue);
    std::string Print(const json::Node &node);
    void ProcessStopRequest(const json::Dict &data, RequestHandler &request_handler, json::Dict &response);
    void ProcessBusRequest(const json::Dict &data, RequestHandler &request_handler, json::Dict &response);
    void LoadStopRequestToDB(const json::Dict &data, tc::TransportCatalogue &db);
    void LoadBusRequestToDB(const json::Dict &data, tc::TransportCatalogue &db);
    void LoadRenderSettingsFromDocument(const json::Document &doc, tc::TransportCatalogue &db);
}