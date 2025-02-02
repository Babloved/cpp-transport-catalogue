#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include <iostream>

using namespace std;

int main(){
    tc::TransportCatalogue db;
    renderer::MapRenderer map_renderer;
    auto doc = jsonReader::LoadStreamJSON(std::cin);
    jsonReader::LoadRenderSettingsFromDocument(doc, map_renderer);
    RequestHandler request_handler(db, map_renderer);
    jsonReader::LoadBaseRequestsFromDocumentToDB(doc, db);
    auto doc_res = jsonReader::ProcessRequestsFromDocument(doc, request_handler);
    Print(doc_res, cout);
    return 0;
}