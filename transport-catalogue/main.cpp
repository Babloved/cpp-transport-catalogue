#include <iostream>
#include <string>
#include "json_reader.h"
#include "transport_catalogue.h"


using namespace std;

int main(){
    auto doc  = jsonReader::LoadStreamJSON(std::cin);
    tc::TransportCatalogue db;
    jsonReader::LoadDataFromDocumentToDB(doc, db);
    auto doc_res = jsonReader::ProcessRequestsFromDocument(doc, db);
    Print(doc_res, cout);

    return 0;
}