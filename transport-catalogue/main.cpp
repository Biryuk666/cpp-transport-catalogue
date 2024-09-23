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
    reader.RequestProcess(catalogue, cin, cout, renderer, handler);
}