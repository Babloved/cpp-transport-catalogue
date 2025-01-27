#include "cassert"
#include "json.h"
#include "json_reader.h"
#include <array>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;



namespace jsonReader{
    json::Document LoadJSON(const std::string &s){
        std::istringstream strm(s);
        return json::Load(strm);
    }

    std::string Print(const json::Node &node){
        std::ostringstream out;
        Print(json::Document{node}, out);
        return out.str();
    }

    std::string ReadRawJSON(std::istream &input){
        const size_t buffer_size = 1024; // Размер буфера
        std::array<char, buffer_size> buffer{};// Буфер для чтения данных
        bool end_json{false};
        long long bracket_counter{0};
        std::string json_raw_string;
        // Проверяем первый символ
        if (input.peek() != '{'){
            throw std::runtime_error("Incorrect JSON format, first char not a '{'");
        }
        // Чтение данных из потока
        while (input && !end_json){
            input.read(buffer.data(), buffer_size);
            std::streamsize bytes_read = input.gcount();
            for (int i = 0; i < bytes_read; ++i){
                switch (buffer[i]){
                    case '{':
                        ++bracket_counter;
                        break;
                    case '}':
                        --bracket_counter;
                        break;
                    default:
                        break;
                }
                json_raw_string.push_back(buffer[i]);

                // Если скобки сбалансированы, выходим из цикла
                if (bracket_counter == 0){
                    end_json = true;
                    break;
                }
            }
        }

        // Проверка на ошибки или несбалансированные скобки
        if ((input.fail() && !input.eof()) || bracket_counter != 0){
            json_raw_string.clear();
            throw std::runtime_error("Incorrect JSON format");
        }
        return json_raw_string;
    }

}