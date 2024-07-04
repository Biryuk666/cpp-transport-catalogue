#include <deque>
#include "geo.h"
#include "input_reader.h"
#include <iomanip>
#include <iostream>
#include "stat_reader.h"
#include <string>

using namespace std;

namespace transport_catalogue {
    namespace stat_reader {
        using namespace transport_catalogue::input_reader;

        vector<string_view> ParseRequest(string_view request) {
            vector<string_view> result;
            for (size_t index = request.find(" ", 0); index != request.npos; index = request.find(" ", 0)) {
                string_view text = request.substr(0, index);
                result.push_back(text);
                request = request.substr(index + 1);
            }
            return result;

        }

        void PrintRoute(const TransportCatalogue& tansport_catalogue, string_view bus_name,
                            ostream& output) {
            const auto& bus = tansport_catalogue.GetBus(bus_name);
            if (bus == nullptr) {
                output << "Bus "s << bus_name << ": "s << "not found"s << endl;
                return;
            }
            int stops_number = GetStopsNumber(*bus);
            int unique_stops_number = GetUniqueStopsNumber(*bus);    
            double route_length = GetRouteLength(bus->stops);
            output << "Bus "s << bus->name << ": "s << stops_number << " stops on route, "s << unique_stops_number << " unique stops, "s 
            << std::fixed << std::setprecision(2) << route_length << " route length"s << endl;
        }

        void PrintBusesForStop(const TransportCatalogue& tansport_catalogue, string_view stop_name,
                            ostream& output) {
            if (tansport_catalogue.GetStop(stop_name) == nullptr) {
                output << "Stop "s << stop_name << ": not found"s << endl;
                return;
            }
            auto buses = tansport_catalogue.GetBusesForStop(stop_name);
            if (buses == nullopt) {
                output << "Stop "s << stop_name << ": no buses"s << endl;
                return;
            }
            output << "Stop "s << stop_name << ": buses"s;
            for (const auto& bus : *buses) {
                output << " "s << bus->name; 
            }
            output << endl;
        }

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, string_view request,
                            ostream& output) {
            QueryType query_type = GetQueryType(request);
            if(query_type == QueryType::BUS) {
                PrintRoute(tansport_catalogue, request.substr(4, request.npos), output);
            } else if (query_type == QueryType::STOP) {
                PrintBusesForStop(tansport_catalogue, request.substr(5, request.npos), output);
            } else {
                output << "Unknown command"s << endl;
            }
        }

    } // end of namespace stat_reader
} // end of namespace transport_catalogue