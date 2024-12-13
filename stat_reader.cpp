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

    void PrintPathInformation(CommandDescription command, const TransportCatalogue& tansport_catalogue,
                        std::ostream& output){
        auto &path = tansport_catalogue.GetPathByName(command.id);
        if (path.GetCountAllStops()){
            output << command.command << " " <<  command.id << ": " << path.GetCountAllStops()
                << " stops on route, " << path.GetCountUniqueStops() << " unique stops, " << setprecision(6)
                << path.CalculateFullPathLenght(tansport_catalogue) << " route length" << std::endl;
        }else{
            output << command.command << " "<< command.id << ": not found" <<std::endl;
        }
    }
    void PrintPathsOnStop(CommandDescription command, const TransportCatalogue& tansport_catalogue,
                        std::ostream& output){
            auto &stop = tansport_catalogue.GetStopByName(command.id);
            if (!stop.first.empty()){
                if (!stop.second.paths_names.empty()){
                    output << command.command<< " "<< command.id << ": buses";
                    for (const auto &bus: stop.second.paths_names){
                        output << " " << bus;
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
  













