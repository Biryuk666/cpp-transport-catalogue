#include <iostream>
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include <string>

//#include "tests.h"

using namespace std;
using namespace transport_catalogue;

int main() {
    //Tests();

    TransportCatalogue catalogue;
    json_reader::JsonReader reader;
    map_renderer::MapRenderer renderer;
    request_handler::RequestHandler handler (catalogue, renderer);
    transport_router::TransportRouter router(catalogue);
    reader.RequestProcess(catalogue, cin, cout, renderer, handler, router);

    
    // iput_reader & stat_reader
    /*TransportCatalogue catalogue;

    int base_request_count;
    cin >> base_request_count >> ws;

    {
        input_reader::InputReader reader;
        for (int i = 0; i < base_request_count; ++i) {
            string line;
            getline(cin, line);
            reader.ParseLine(line);
        }
        reader.ApplyCommands(catalogue);
    }

    int stat_request_count;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
        stat_reader::ParseAndPrintStat(catalogue, line, cout);
    }
    cin.get();
    cin.get();*/
}