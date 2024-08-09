//#include "input_reader.h"
#include <iostream>
#include "json.h"
#include "json_reader.h"
//#include "stat_reader.h"
#include <string>
//#include "tests.h"

using namespace std;
using namespace transport_catalogue;

int main() {
    TransportCatalogue catalogue;
    json_reader::JsonReader reader;
    reader.RequestProcess(catalogue, cin, cout);


    //Tests();

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