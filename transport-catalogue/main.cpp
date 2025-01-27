#include <iostream>
#include <string>
#include "json_reader.h"
using namespace std;

int main(){
    auto raw_json_str = jsonReader::ReadRawJSON(cin);
    if (!raw_json_str.empty()){
        cout << raw_json_str;
        json::Document document = jsonReader::LoadJSON(raw_json_str);
        auto &root_node = document.GetRoot();
        if (document.GetRoot().IsMap()){
            for (const auto &item: root_node.AsMap()){
                cout << item.first;
            }
        }
    } else{
        cout << "Empty JSON";
    }
    string str;
    cin >> str;
    cout << endl << str;
    return 0;
}