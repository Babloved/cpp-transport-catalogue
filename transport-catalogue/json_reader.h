#pragma once

#include "json.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include <string>

#include "json_builder.h"

class JsonReader {
public:
    [[nodiscard]] static json::Document LoadJSON(const std::string &&s);
    static std::string Print(const json::Node &node);
    static void LoadRenderSettingsFromDocument(const json::Document &doc, renderer::MapRenderer &renderer);
    static void LoadBaseRequestsFromDocumentToDB(const json::Document &doc, tc::TransportCatalogue &db);
    [[nodiscard]] static json::Document LoadStreamJSON(std::istream &input);
    static json::Document ProcessRequestsFromDocument(const json::Document &doc, RequestHandler &request_handler);
    static void LoadRoutingSettingsFromDocument(const json::Document& doc, transport_router::TransportRouter& router);
private:
    static void ProcessStopRequest(const json::Dict &data, RequestHandler &request_handler, json::Builder &builder);
    static void ProcessBusRequest(const json::Dict &data, RequestHandler &request_handler, json::Builder &json_builder);
    static void LoadStopRequestToDB(const json::Dict &data, tc::TransportCatalogue &db);
    static void LoadBusRequestToDB(const json::Dict &data, tc::TransportCatalogue &db);
    static void ProcessMapRequest(RequestHandler &request_handler, json::Builder &json_builder);
    static void ProcessRouteRequest(const json::Dict& data, RequestHandler& request_handler, json::Builder& json_builder);
};