#include <cassert>
#include <iostream>
#include "tests.h"
#include "transport_catalogue.h"

using namespace std;
using namespace transport_catalogue;

void TertGetRouteLength() {
        TransportCatalogue::Stop stop1{"from", 55.85623777240991, 38.965963307835146};
        TransportCatalogue::Stop stop2{"to", 55.856631639749466, 38.96792060733355};
        TransportCatalogue::Bus bus;
        bus.stops.push_back(&stop1);
        bus.stops.push_back(&stop2);
        auto route_info = GetRouteInfo(bus);
        assert(route_info.route_length < 130 && route_info.route_length > 129.5);        
}

void Tests() {
    TertGetRouteLength();
}