#include <iostream>
#include <string>
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"


using namespace std;

int main(){
    tc::TransportCatalogue db;
    renderer::MapRenderer map_renderer;
    auto doc  = jsonReader::LoadStreamJSON(std::cin);
    jsonReader::LoadRenderSettingsFromDocument(doc, map_renderer);
    RequestHandler request_handler(db,map_renderer);
    jsonReader::LoadStatRequestsFromDocumentToDB(doc, db);
    auto doc_res = jsonReader::ProcessRequestsFromDocument(doc, request_handler);
//    Print(doc_res, cout);
    auto a = request_handler.RenderMap();
    a.Render(cout);

    return 0;
}
