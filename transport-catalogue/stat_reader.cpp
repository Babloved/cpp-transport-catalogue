#include <iomanip>
#include "stat_reader.h"
#include "iostream"
#include "string"


using namespace std;
using namespace tc;

namespace statreader {
    CommandDescription ParseCommandDescription(std::string_view line) {
        auto space_pos = line.find(' ');
        return {std::string(line.substr(0, space_pos)),
                std::string(line.substr(space_pos+1,line.size())),
                std::string()};
    }

    void PrintPathInformation(CommandDescription command, const TransportCatalogue& transport_catalogue,
                        std::ostream& output){
        if (transport_catalogue.GetCountAllStops(command.id) > 0){
            output << command.command << " " << command.id << ": " << transport_catalogue.GetCountAllStops(command.id)
                   << " stops on route, " << transport_catalogue.GetCountUniqueStops(command.id) << " unique stops, " << setprecision(6)
                   << transport_catalogue.CalculateFullPathLenght(command.id) << " route length" << std::endl;
        }else{
            output << command.command << " "<< command.id << ": not found" <<std::endl;
        }
    }
    void PrintPathsOnStop(CommandDescription command, const TransportCatalogue& transport_catalogue,
                        std::ostream& output){
            if (transport_catalogue.IsStopExist(command.id)){
                auto p_paths_on_stop = transport_catalogue.GetPathsOnStop(command.id);
                if (p_paths_on_stop){
                    output << command.command<< " "<< command.id << ": buses";
                    for (const auto &path: *p_paths_on_stop){
                        output << " " << path;
                    }
                    output <<std::endl;
                }else{
                    output << command.command << " "<< command.id << ": no buses" <<std::endl;
                }
            }else{
                output << command.command << " "<< command.id << ": not found" <<std::endl;
            }
    }

    void ParseAndPrintStat(const tc::TransportCatalogue& tansport_catalogue, std::string_view request,
                    std::ostream& output) {
        CommandDescription command = ParseCommandDescription(request);
        if (command.command == "Bus"){
            PrintPathInformation(command,tansport_catalogue,output);
        }else if (command.command == "Stop"){
            PrintPathsOnStop(command,tansport_catalogue,output);
        }else{
            output << "Unncorrect comand: " << command.command << " " << command.id << std::endl;
        }
    } 
}
  













