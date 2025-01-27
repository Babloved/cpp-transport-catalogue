#pragma once
#include "json.h"

#include <string>

namespace jsonReader{
    json::Document LoadJSON(const std::string &s);
    std::string Print(const json::Node &node);
    std::string ReadRawJSON(std::istream &input);
}