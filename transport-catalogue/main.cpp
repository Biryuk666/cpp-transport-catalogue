#include <iostream>
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include <string>

#include "tests.h"

using namespace std;
using namespace transport_catalogue;

int main() {
    // тест SVG библиотеки
    /*TransportCatalogue catalogue;
    json_reader::JsonReader reader;
    map_renderer::MapRenderer renderer;
    reader.RequestProcess(catalogue, cin, cerr, renderer);
    
    svg::Document doc = renderer.RenderMap(catalogue.GetBusesList());
    doc.Render(cout);*/

    // тест Json библиотеки
    /*TransportCatalogue catalogue;
    json_reader::JsonReader reader;
    reader.RequestProcess(catalogue, cin, cout);*/


    Tests();

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