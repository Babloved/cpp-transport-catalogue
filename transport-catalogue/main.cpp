#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include <iostream>

using namespace std;

int main() {
    tc::TransportCatalogue db;
    renderer::MapRenderer map_renderer;

    auto doc = JsonReader::LoadStreamJSON(std::cin);
    JsonReader::LoadRenderSettingsFromDocument(doc, map_renderer);
    JsonReader::LoadBaseRequestsFromDocumentToDB(doc, db);

    // Загружаем настройки маршрутизации через JsonReader
    auto routing_settings = JsonReader::LoadRoutingSettingsFromDocument(doc);

    // Передаем настройки в конструктор TransportRouter
    transport_router::TransportRouter transport_router(db, routing_settings);
    RequestHandler request_handler(db, map_renderer, transport_router);
    auto doc_res = JsonReader::ProcessRequestsFromDocument(doc, request_handler);
    Print(doc_res, std::cout);
    return 0;
}