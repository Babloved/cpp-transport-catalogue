#pragma once

#include "json.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include <string>

class JsonReader{
public:
    [[nodiscard]]static json::Document LoadJSON(const std::string &&s);
    static std::string Print(const json::Node &node);
    static void LoadRenderSettingsFromDocument(const json::Document &doc, renderer::MapRenderer &renderer);
    static void LoadBaseRequestsFromDocumentToDB(const json::Document &doc, tc::TransportCatalogue &db);
    [[nodiscard]]static json::Document LoadStreamJSON(std::istream &input);
    static json::Document ProcessRequestsFromDocument(const json::Document &doc, RequestHandler &request_handler);
private:
    static void ProcessStopRequest(const json::Dict &data, RequestHandler &request_handler, json::Dict &response);
    static void ProcessBusRequest(const json::Dict &data, RequestHandler &request_handler, json::Dict &response);
    static void LoadStopRequestToDB(const json::Dict &data, tc::TransportCatalogue &db);
    static void LoadBusRequestToDB(const json::Dict &data, tc::TransportCatalogue &db);
    static void ProcessMapRequest(RequestHandler &request_handler, json::Dict &response);
};