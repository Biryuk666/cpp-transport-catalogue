#include <fstream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "json_reader.h"
#include "serialization.h"
#include "transport_catalogue.h"

using namespace std::literals;
using namespace transport_catalogue;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }    

    TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;
    transport_router::RouterSettings router_settings;
    json_reader::JsonReader reader;

    const std::string_view mode(argv[1]);
    //const std::string mode("make_base");
    //const std::string mode("process_requests");

    if (mode == "make_base"sv) {    
        try {
            reader.MakeBase(std::cin, catalogue, renderer, router_settings);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 2;
        }

    } else if (mode == "process_requests"sv) {
        try {
            transport_router::TransportRouter router(catalogue);
            reader.ProcessRequests(std::cin, std::cout, catalogue, renderer, router);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 3;
        }
        
    } else {
        PrintUsage();
        return 1;
    }
}